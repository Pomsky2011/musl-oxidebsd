#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real utimensat()'s wire format is (fd, path, times, flags) -- path is a plain
 * null-terminated C string, and fd is a real dirfd a path can be resolved relative to. OxideBSD
 * has no real dirfd-relative resolution (same limitation every other *at()-shaped call already
 * patched on this branch accepts -- open()/chdir()/mkdir() only ever resolve against the caller's
 * own cwd or an absolute path), so this only supports the AT_FDCWD case -- the only one any real
 * caller in this port's roster (BusyBox's touch.c) ever actually uses. OxideBSD's own
 * SYS_UTIMENSAT (modules/oxfs/src/lib.rs's oxfs_utimensat) instead expects
 * (path_ptr, path_len, times_ptr, flags), the same "drop the always-AT_FDCWD fd, add a computed
 * length" shape chown()/rename() already established on this branch -- computing path's length
 * explicitly rather than remapping alone, since the real argument count/shape differs.
 */
int utimensat(int fd, const char *path, const struct timespec times[2], int flags)
{
#ifdef SYS_utimensat
	if (fd != AT_FDCWD)
		return __syscall_ret(-ENOSYS);
	long ret = __syscall4(SYS_utimensat, (long)path, (long)strlen(path), (long)times, (long)flags);
	return __syscall_ret(ret);
#else
	return __syscall_ret(-ENOSYS);
#endif
}
