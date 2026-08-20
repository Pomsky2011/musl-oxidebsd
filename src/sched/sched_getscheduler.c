#include <sched.h>
#include <errno.h>
#include "syscall.h"

/* Un-stubbed for the same reason src/sched/sched_getparam.c was -- see that file's own comment.
 * The real (pid) wire format needed no repacking. */
int sched_getscheduler(pid_t pid)
{
	return syscall(SYS_sched_getscheduler, pid);
}
