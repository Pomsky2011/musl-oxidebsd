#include "pthread_impl.h"
#include <threads.h>

static int __pthread_detach(pthread_t t)
{
	/* If the cas fails, detach state is either already-detached
	 * or exiting/exited, and pthread_join will trap or cleanup. */
	int state = a_cas(&t->detach_state, DT_JOINABLE, DT_DETACHED);
	if (state != DT_JOINABLE) {
		/* Detaching a thread that already knows it's not joinable is
		 * exactly the case POSIX's own EINVAL contract for
		 * pthread_detach() covers -- but only safe to report directly,
		 * without ever reaching __pthread_join()'s own a_crash() safety
		 * check, when the target is the calling thread itself: t's TCB
		 * is then guaranteed still resident, since a thread can't have
		 * unmapped its own stack while still running on it. Detaching
		 * some *other* thread that might be mid-exit stays on the
		 * existing join-fallback path below -- that race (the target's
		 * memory being freed out from under us before we can read it)
		 * is genuinely undetectable, matching real musl's own
		 * pthread_join() behavior for that same case. */
		if (t == __pthread_self() && state == DT_DETACHED)
			return EINVAL;
		int cs;
		__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
		__pthread_join(t, 0);
		__pthread_setcancelstate(cs, 0);
	}
	return 0;
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
