#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "pthread_impl.h"

struct lio_state {
	struct sigevent *sev;
	int cnt;
	struct aiocb *cbs[];
};

static int lio_wait(struct lio_state *st)
{
	int i, err, got_err = 0;
	int cnt = st->cnt;
	struct aiocb **cbs = st->cbs;

	for (;;) {
		for (i=0; i<cnt; i++) {
			if (!cbs[i]) continue;
			err = aio_error(cbs[i]);
			if (err==EINPROGRESS)
				break;
			if (err) got_err=1;
			cbs[i] = 0;
		}
		if (i==cnt) {
			if (got_err) {
				errno = EIO;
				return -1;
			}
			return 0;
		}
		if (aio_suspend((void *)cbs, cnt, 0))
			return -1;
	}
}

static void notify_signal(struct sigevent *sev)
{
	siginfo_t si = {
		.si_signo = sev->sigev_signo,
		.si_value = sev->sigev_value,
		.si_code = SI_ASYNCIO,
		.si_pid = getpid(),
		.si_uid = getuid()
	};
	__syscall(SYS_rt_sigqueueinfo, si.si_pid, si.si_signo, &si);
}

static void *wait_thread(void *p)
{
	struct lio_state *st = p;
	struct sigevent *sev = st->sev;
	lio_wait(st);
	free(st);
	switch (sev->sigev_notify) {
	case SIGEV_SIGNAL:
		notify_signal(sev);
		break;
	case SIGEV_THREAD:
		sev->sigev_notify_function(sev->sigev_value);
		break;
	}
	return 0;
}

int lio_listio(int mode, struct aiocb *restrict const *restrict cbs, int cnt, struct sigevent *restrict sev)
{
	int i, ret, bad = 0;
	struct lio_state *st=0;

	/* Real POSIX: "[EINVAL] The mode argument is not LIO_NOWAIT or
	 * LIO_WAIT". Never checked at all before -- any other value (e.g.
	 * -1) silently fell through every `mode == LIO_WAIT` check as if it
	 * meant LIO_NOWAIT. Found via the Open POSIX Test Suite's
	 * lio_listio/18-1.c.
	 */
	if (mode != LIO_WAIT && mode != LIO_NOWAIT) {
		errno = EINVAL;
		return -1;
	}

	if (cnt < 0) {
		errno = EINVAL;
		return -1;
	}

	if (mode == LIO_WAIT || (sev && sev->sigev_notify != SIGEV_NONE)) {
		if (!(st = malloc(sizeof *st + cnt*sizeof *cbs))) {
			errno = EAGAIN;
			return -1;
		}
		st->cnt = cnt;
		st->sev = sev;
		memcpy(st->cbs, (void*) cbs, cnt*sizeof *cbs);
	}

	for (i=0; i<cnt; i++) {
		if (!cbs[i]) continue;
		switch (cbs[i]->aio_lio_opcode) {
		case LIO_READ:
			aio_read(cbs[i]);
			break;
		case LIO_WRITE:
			aio_write(cbs[i]);
			break;
		case LIO_NOP:
			/* Real POSIX: a LIO_NOP entry is legitimately ignored,
			 * never actually submitted -- but lio_wait() below
			 * still calls aio_error() on every non-null entry
			 * during its own WAIT-mode aggregation scan, and
			 * aio_error() now rejects a genuinely never-submitted
			 * aiocb with EINVAL (see struct aiocb's own __dummy4
			 * doc comment) -- which lio_wait() would misread as a
			 * real per-request failure, corrupting the aggregate
			 * result for every *other*, real entry in the same
			 * batch. Mark it "used" (leaving __err/__ret alone,
			 * whatever the caller already set them to -- typically
			 * 0 from a fresh memset) so aio_error() reports that
			 * real, harmless value instead.
			 */
			cbs[i]->__dummy4[0] = 1;
			break;
		default:
			/* Real POSIX defines no aio_lio_opcode value besides
			 * LIO_READ/LIO_WRITE/LIO_NOP -- unlike LIO_NOP (which
			 * is legitimately skipped), a genuinely invalid value
			 * is this request's own error, not a silent no-op.
			 * Never actually submitted, so fill in the same
			 * error/return state a real failed aio_read()/
			 * aio_write() would have -- a later aio_error()/
			 * aio_return() call, or this function's own lio_wait()
			 * aggregation below, needs to see a real failure here,
			 * not silently report success. `bad` additionally
			 * makes LIO_NOWAIT mode itself report the failure
			 * synchronously (below) -- LIO_WAIT mode already gets
			 * this for free from lio_wait()'s own aio_error() scan.
			 */
			cbs[i]->__dummy4[0] = 1;
			cbs[i]->__ret = -1;
			cbs[i]->__err = EINVAL;
			bad = 1;
			break;
		}
		/* A per-request submission failure (aio_read()/aio_write()
		 * itself returning -1, e.g. real EBADF on a bad fd) is
		 * already recorded on *that* aiocb's own __err/__ret by
		 * submit() -- real POSIX: "[a] failure of an individual
		 * request does not prevent completion of any other
		 * individual request" -- so, unlike an invalid opcode
		 * (discovered by this function itself, before ever
		 * attempting submission), it must not abort the rest of
		 * this loop or affect lio_listio()'s own return value.
		 * Found via the Open POSIX Test Suite's lio_listio/14-1.c
		 * (previously: any one bad-fd entry silently discarded every
		 * later entry in the list, never even attempting them).
		 */
	}

	if (mode == LIO_WAIT) {
		ret = lio_wait(st);
		free(st);
		return ret;
	}

	if (st) {
		pthread_attr_t a;
		sigset_t set, set_old;
		pthread_t td;

		if (sev->sigev_notify == SIGEV_THREAD) {
			if (sev->sigev_notify_attributes)
				a = *sev->sigev_notify_attributes;
			else
				pthread_attr_init(&a);
		} else {
			pthread_attr_init(&a);
			pthread_attr_setstacksize(&a, PAGE_SIZE);
			pthread_attr_setguardsize(&a, 0);
		}
		pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
		sigfillset(&set);
		pthread_sigmask(SIG_BLOCK, &set, &set_old);
		if (pthread_create(&td, &a, wait_thread, st)) {
			free(st);
			errno = EAGAIN;
			return -1;
		}
		pthread_sigmask(SIG_SETMASK, &set_old, 0);
	}

	/* Real POSIX: "if mode is LIO_NOWAIT, lio_listio() shall return the
	 * value -1 and set errno to indicate error if the operation is not
	 * successfully queued" -- an invalid opcode was never queued at all.
	 * Checked last, after any real wait_thread above is already spawned
	 * (see this function's own doc comment above `bad`'s assignment) so
	 * the rest of a real, valid batch still runs to completion and still
	 * fires the caller's own list-completion sigevent, exactly as if
	 * this function had returned 0. Found via the Open POSIX Test
	 * Suite's lio_listio/11-1.c.
	 */
	if (bad) {
		errno = EIO;
		return -1;
	}

	return 0;
}
