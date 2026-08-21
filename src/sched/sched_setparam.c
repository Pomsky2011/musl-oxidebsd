#include <sched.h>
#include <errno.h>
#include "syscall.h"

/* Un-stubbed for the same reason src/sched/sched_getparam.c and src/sched/sched_setscheduler.c
 * were -- see those files' own comments. The real (pid, param_ptr) wire format needed no
 * repacking. */
int sched_setparam(pid_t pid, const struct sched_param *param)
{
	return syscall(SYS_sched_setparam, pid, param);
}
