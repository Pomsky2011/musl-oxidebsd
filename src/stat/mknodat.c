#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real mknodat()'s wire format is (fd, path, mode, dev) -- path is a plain
 * null-terminated C string, and fd is a real dirfd a path can be resolved relative to. OxideBSD
 * has no real dirfd-relative resolution (same limitation every other *at()-shaped call already
 * patched on this branch accepts -- see src/stat/utimensat.c), so this only supports the
 * AT_FDCWD case. Routes through the same SYS_MKNOD number plain mknod() uses (see src/stat/
 * mknod.c on this same branch) rather than a separate SYS_mknodat remap -- both are the same real
 * kernel-side operation once fd is dropped, and OxideBSD's own oxfs_mknod (modules/oxfs/src/
 * lib.rs) only ever registers the one handler.
 */
int mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
#ifdef SYS_mknod
	if (fd != AT_FDCWD)
		return __syscall_ret(-ENOSYS);
	return syscall(SYS_mknod, path, strlen(path), mode, dev);
#else
	return syscall(SYS_mknodat, fd, path, mode, dev);
#endif
}
