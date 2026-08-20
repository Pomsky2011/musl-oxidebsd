#include <sched.h>
#include <errno.h>
#include "syscall.h"

/* Un-stubbed for the same reason src/sched/sched_getparam.c was -- see that file's own comment.
 * The real (pid, policy, param_ptr) wire format needed no repacking. */
int sched_setscheduler(pid_t pid, int sched, const struct sched_param *param)
{
	return syscall(SYS_sched_setscheduler, pid, sched, param);
}
