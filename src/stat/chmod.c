#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real chmod()'s wire format is (path, mode) in RDI/RSI -- path is a plain
 * null-terminated C string. OxideBSD's own SYS_CHMOD (modules/oxfs/src/lib.rs's oxfs_chmod in the
 * OxideBSD tree) instead expects (path_ptr, path_len, mode) -- the same argument-convention
 * mismatch open()/chdir()/mkdir() needed patching for (see src/fcntl/open.c and src/unistd/
 * chdir.c on this same branch), computing path_len via strlen() since the caller only ever gives
 * us a null-terminated C string.
 */
int chmod(const char *path, mode_t mode)
{
#ifdef SYS_chmod
	return syscall(SYS_chmod, path, strlen(path), mode);
#else
	return syscall(SYS_fchmodat, AT_FDCWD, path, mode);
#endif
}
