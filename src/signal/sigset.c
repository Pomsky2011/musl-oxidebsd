#include <signal.h>

void (*sigset(int sig, void (*handler)(int)))(int)
{
	struct sigaction sa, sa_old;
	sigset_t mask, mask_old;

	sigemptyset(&mask);
	if (sigaddset(&mask, sig) < 0)
		return SIG_ERR;
	
	if (handler == SIG_HOLD) {
		/* disp==SIG_HOLD only adds sig to the mask -- disposition is left alone, so
		 * there's no "previous disposition" to report the way the else-branch below
		 * reports sa_old.sa_handler. Real XSI semantics: on success this always
		 * returns SIG_HOLD itself, confirming the hold was applied -- not the prior
		 * blocked-state-dependent value this used to return (sa_old.sa_handler
		 * whenever sig wasn't already blocked), which made a first-ever
		 * sigset(sig, SIG_HOLD) call indistinguishable from a real error to any
		 * caller checking the return against SIG_HOLD. */
		if (sigprocmask(SIG_BLOCK, &mask, 0) < 0)
			return SIG_ERR;
		return SIG_HOLD;
	}
	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(sig, &sa, &sa_old) < 0)
		return SIG_ERR;
	if (sigprocmask(SIG_UNBLOCK, &mask, &mask_old) < 0)
		return SIG_ERR;
	/* Setting a real disposition: report the previous one, unless it was itself a
	 * SIG_HOLD (i.e. sig was blocked and its stored sa_handler doesn't reflect a
	 * real disposition change since the last actual sigaction() call) -- SIG_HOLD
	 * is the only way to express that via this function's plain handler-shaped
	 * return type. */
	return sigismember(&mask_old, sig) ? SIG_HOLD : sa_old.sa_handler;
}
