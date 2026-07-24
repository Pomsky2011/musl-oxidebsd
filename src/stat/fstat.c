#define _BSD_SOURCE
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

/* OxideBSD patch: real fstat(fd, st) delegates to __fstatat(fd, "", st, AT_EMPTY_PATH), which
 * itself calls through musl's generic kstat-then-copy machinery. OxideBSD's own SYS_FSTAT
 * (modules/oxfs/oxfs_fstat in the OxideBSD tree) takes real fstat's own (fd, buf) wire format
 * unchanged -- fd is a plain int, no length-prefix/string mismatch the way stat()/open() have --
 * and writes a byte-exact struct stat straight into buf, no kstat translation needed (see
 * src/stat/stat.c's own comment on this same branch). So this bypasses fstatat() entirely rather
 * than patching that generic multiplexer, matching this port's existing per-entry-point pattern.
 */
int __fstat(int fd, struct stat *st)
{
	if (fd<0) return __syscall_ret(-EBADF);
	return syscall(SYS_fstat, fd, st);
}

weak_alias(__fstat, fstat);
