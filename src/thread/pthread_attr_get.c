#include "pthread_impl.h"

int pthread_attr_getdetachstate(const pthread_attr_t *a, int *state)
{
	*state = a->_a_detach;
	return 0;
}
int pthread_attr_getguardsize(const pthread_attr_t *restrict a, size_t *restrict size)
{
	*size = a->_a_guardsize;
	return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *restrict a, int *restrict inherit)
{
	*inherit = a->_a_sched;
	return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *restrict a, struct sched_param *restrict param)
{
	param->sched_priority = a->_a_prio;
	return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *restrict a, int *restrict policy)
{
	*policy = a->_a_policy;
	return 0;
}

int pthread_attr_getscope(const pthread_attr_t *restrict a, int *restrict scope)
{
	*scope = PTHREAD_SCOPE_SYSTEM;
	return 0;
}

int pthread_attr_getstack(const pthread_attr_t *restrict a, void **restrict addr, size_t *restrict size)
{
	/* Real bug, not intentional: a fresh, never-pthread_attr_setstack()'d attr has
	 * _a_stackaddr==0 (its own real sentinel for "let pthread_create() choose the stack
	 * itself"), which used to unconditionally EINVAL here -- but POSIX documents no error
	 * conditions at all for this function, and real glibc succeeds unconditionally, reporting
	 * a NULL address (confirmed live: freshly pthread_attr_init()'d attr, glibc's own
	 * pthread_attr_getstack() returns 0 with addr=NULL). Found via the Open POSIX Test Suite's
	 * pthread_attr_setstack/{1,2,4,6,7}-1.c, every one of which calls this immediately after
	 * pthread_attr_init() (before ever calling pthread_attr_setstack()) purely to read back
	 * the attr's own current/default values -- the EINVAL here made all five fail before
	 * reaching any of their own real test logic, kernel-independent (no thread has been
	 * created yet at this point in any of them). */
	*size = a->_a_stacksize;
	*addr = a->_a_stackaddr ? (void *)(a->_a_stackaddr - *size) : 0;
	return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *restrict a, size_t *restrict size)
{
	*size = a->_a_stacksize;
	return 0;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *restrict a, int *restrict pshared)
{
	*pshared = !!a->__attr;
	return 0;
}

int pthread_condattr_getclock(const pthread_condattr_t *restrict a, clockid_t *restrict clk)
{
	*clk = a->__attr & 0x7fffffff;
	return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *restrict a, int *restrict pshared)
{
	*pshared = a->__attr>>31;
	return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict a, int *restrict protocol)
{
	*protocol = a->__attr / 8U % 2;
	return 0;
}
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict a, int *restrict pshared)
{
	*pshared = a->__attr / 128U % 2;
	return 0;
}

int pthread_mutexattr_getrobust(const pthread_mutexattr_t *restrict a, int *restrict robust)
{
	*robust = a->__attr / 4U % 2;
	return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *restrict a, int *restrict type)
{
	*type = a->__attr & 3;
	return 0;
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *restrict a, int *restrict pshared)
{
	*pshared = a->__attr[0];
	return 0;
}
