#include <unistd.h>
#include <signal.h>
#include "syscall.h"
#include "libc.h"
#include "lock.h"
#include "pthread_impl.h"
#include "stdio_impl.h"
#include "aio_impl.h"
#include "fork_impl.h"

static void dummy(int x) { }
weak_alias(dummy, __aio_atfork);

/* A forked child inherits a byte-for-byte copy of every FILE's own .lock word, including
 * whatever real tid last held it -- a ghost identity, since the surviving thread's own tid is
 * reassigned just below and no other thread survives the fork. Nothing else in this tree ever
 * fixes this up: unlike glibc's stdio, which resets every stream's lock in the child via its own
 * pthread_atfork-registered handler (_IO_list_resetlock) for exactly this reason, stock musl
 * tracks no such generic "every FILE that might be locked" registry and never clears one. A lock
 * held across fork() (e.g. flockfile(stdout) before a fork()) becomes permanently unacquirable in
 * the child: any later flockfile()/ftrylockfile() call, by any thread, deadlocks or fails forever,
 * since the real owner tid no longer exists in this address space and will never call
 * funlockfile() again -- confirmed live via a permanent OxideBSD POSIX-pilot hang
 * (conformance/interfaces/fork/11-1.c) whose own error-reporting path deadlocked trying to
 * re-acquire stdout's lock to report the failure. Fixed by clearing every known FILE's lock in
 * the child, matching glibc's own real behavior.
 *
 * A second, real, independent bug found later chasing a *different* permanent hang
 * (OxideBSD's own pthread_cond_broadcast/1-2.c pilot investigation, a real fork() from a
 * genuinely multi-threaded process -- a process with at least one extra real pthread already
 * running, calling real fork() to create more children): this function's own `__ofl_lock()` call
 * just below is itself a real, non-reentrant lock acquisition -- but by the time it runs, the
 * real, upstream fork() wrapper (src/process/fork.c) has *already* taken that exact same lock
 * (`__stdio_ofl_lockptr`, one of its own `atfork_locks[]`) in the parent, whenever
 * `libc.need_locks > 0` (true for any genuinely multi-threaded process), before ever calling
 * _Fork() at all. The child inherits that lock in a real, genuinely *locked* state -- so this
 * function's own `__ofl_lock()` call deadlocks immediately, waiting forever on a lock nothing
 * will ever release (the only surviving thread in the child is the one stuck here; fork()'s own
 * later child-side cleanup, which *would* reset this exact lock, never gets the chance to run
 * first). Fixed the same way fork()'s own child-side atfork cleanup already treats every other
 * lock in this exact situation (a raw store, not a real UNLOCK): force the lock word itself back
 * to unlocked immediately before ever trying to acquire it, since a freshly forked child is
 * always, unconditionally, the sole surviving thread -- any inherited "locked" state is never
 * real contention, only ever exactly this kind of stale, inherited garbage. */
static void reset_stdio_locks_in_child(void)
{
	FILE *f;
	if ((f = __stdin_used)) { f->lock = 0; f->lockcount = 0; }
	if ((f = __stdout_used)) { f->lock = 0; f->lockcount = 0; }
	if ((f = __stderr_used)) { f->lock = 0; f->lockcount = 0; }
	if (__stdio_ofl_lockptr) *__stdio_ofl_lockptr = 0;
	for (f = *__ofl_lock(); f; f = f->next) { f->lock = 0; f->lockcount = 0; }
	__ofl_unlock();
}

void __post_Fork(int ret)
{
	if (!ret) {
		pthread_t self = __pthread_self();
		self->tid = __syscall(SYS_set_tid_address, &__thread_list_lock);
		self->robust_list.off = 0;
		self->robust_list.pending = 0;
		self->next = self->prev = self;
		__thread_list_lock = 0;
		libc.threads_minus_1 = 0;
		if (libc.need_locks) libc.need_locks = -1;
		reset_stdio_locks_in_child();
		/* This thread's own "files I hold the lock on" bookkeeping no longer matches
		 * reality once every lock above is force-cleared. */
		self->stdio_locks = 0;
	}
	UNLOCK(__abort_lock);
	if (!ret) __aio_atfork(1);
}

pid_t _Fork(void)
{
	pid_t ret;
	sigset_t set;
	__block_all_sigs(&set);
	LOCK(__abort_lock);
#ifdef SYS_fork
	ret = __syscall(SYS_fork);
#else
	ret = __syscall(SYS_clone, SIGCHLD, 0);
#endif
	__post_Fork(ret);
	__restore_sigs(&set);
	return __syscall_ret(ret);
}
