#include "stdio_impl.h"
#include "pthread_impl.h"
#include <limits.h>

void __do_orphaned_stdio_locks()
{
	FILE *f;
	/* Real release, not a poison marker: the previous `a_store(&f->lock, 0x40000000)`
	 * left the lock word non-zero forever (MAYBE_WAITERS set, owner bits zeroed) with
	 * no live thread left to ever call funlockfile()/__unlockfile() on it -- any future
	 * ftrylockfile() sees a truthy `owner` and fails permanently, and any future blocking
	 * flockfile() enters __lockfile()'s retry loop, sees MAYBE_WAITERS already set, and
	 * __futexwait()s on a value nothing will ever change or wake. Confirmed live: an
	 * OxideBSD POSIX-pilot test (conformance/interfaces/fork/11-1.c) has a thread acquire
	 * stdout's lock via ftrylockfile() and exit without ever calling funlockfile() (legal
	 * -- POSIX doesn't require balancing flockfile/funlockfile before thread exit); the
	 * process's own later exit()-time stdio flush then deadlocks re-locking stdout for
	 * good. Match __unlockfile()'s own real release-and-wake pattern instead: swap to
	 * fully unlocked (0), and wake a real waiter if MAYBE_WAITERS says one might be
	 * blocked -- exactly what would happen if the exiting thread had properly called
	 * funlockfile() itself before exiting. */
	for (f=__pthread_self()->stdio_locks; f; f=f->next_locked)
		if (a_swap(&f->lock, 0) & MAYBE_WAITERS)
			__wake(&f->lock, 1, 1);
}

void __unlist_locked_file(FILE *f)
{
	if (f->lockcount) {
		if (f->next_locked) f->next_locked->prev_locked = f->prev_locked;
		if (f->prev_locked) f->prev_locked->next_locked = f->next_locked;
		else __pthread_self()->stdio_locks = f->next_locked;
	}
}

void __register_locked_file(FILE *f, pthread_t self)
{
	f->lockcount = 1;
	f->prev_locked = 0;
	f->next_locked = self->stdio_locks;
	if (f->next_locked) f->next_locked->prev_locked = f;
	self->stdio_locks = f;
}

int ftrylockfile(FILE *f)
{
	pthread_t self = __pthread_self();
	int tid = self->tid;
	int owner = f->lock;
	if ((owner & ~MAYBE_WAITERS) == tid) {
		if (f->lockcount == LONG_MAX)
			return -1;
		f->lockcount++;
		return 0;
	}
	if (owner < 0) f->lock = owner = 0;
	if (owner || a_cas(&f->lock, 0, tid))
		return -1;
	__register_locked_file(f, self);
	return 0;
}
