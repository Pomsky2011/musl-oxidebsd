#include <sched.h>
#include <errno.h>
#include "syscall.h"

/* Upstream musl always stubs this to ENOSYS on the (correct) assumption that real Linux's own
 * CFS scheduler has no meaningful "current sched_param" to report for a SCHED_OTHER process.
 * OxideBSD's own single-core cooperative scheduler has a real, if unenforced, per-process
 * sched_policy/sched_priority pair (see the OxideBSD tree's process::do_sched_getparam) it's
 * always safe to report -- the real (pid, param_ptr) wire format needed no repacking, so
 * un-stubbing this to a real syscall was all that was needed. Found live: the Open POSIX Test
 * Suite pilot's sched_getparam/1-1..4-1.c all call the real libc function directly, never a raw
 * syscall(), so they never reached the kernel at all while this stayed stubbed. */
int sched_getparam(pid_t pid, struct sched_param *param)
{
	return syscall(SYS_sched_getparam, pid, param);
}
