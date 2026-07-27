#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real chown()'s wire format is (path, uid, gid) in RDI/RSI/RDX -- path is a
 * plain null-terminated C string. OxideBSD's own SYS_CHOWN (modules/oxfs/src/lib.rs's oxfs_chown
 * in the OxideBSD tree) instead expects (path_ptr, path_len, uid, gid), using all four of this
 * ABI's argument registers -- the same "R10 becomes a real argument" shape SYS_rename already
 * established (see src/stdio/rename.c on this same branch).
 */
int chown(const char *path, uid_t uid, gid_t gid)
{
#ifdef SYS_chown
	long ret = __syscall4(SYS_chown, (long)path, (long)strlen(path), (long)uid, (long)gid);
	return __syscall_ret(ret);
#else
	return syscall(SYS_fchownat, AT_FDCWD, path, uid, gid, 0);
#endif
}
