#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real mknod()'s wire format is (path, mode, dev) in RDI/RSI/RDX -- path is a
 * plain null-terminated C string. OxideBSD's own SYS_MKNOD (modules/oxfs/src/lib.rs's oxfs_mknod
 * in the OxideBSD tree) instead expects (path_ptr, path_len, mode, dev), the same "one path plus
 * trailing scalar args" shape SYS_CHMOD already established -- see src/stat/chmod.c on this same
 * branch.
 */
int mknod(const char *path, mode_t mode, dev_t dev)
{
#ifdef SYS_mknod
	return syscall(SYS_mknod, path, strlen(path), mode, dev);
#else
	return syscall(SYS_mknodat, AT_FDCWD, path, mode, dev);
#endif
}
