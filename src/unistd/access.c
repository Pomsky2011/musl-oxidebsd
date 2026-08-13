#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real access()'s wire format is (path, amode) in RDI/RSI -- path is a plain
 * null-terminated C string. OxideBSD's own SYS_ACCESS (modules/oxfs/src/lib.rs's oxfs_access in
 * the OxideBSD tree) instead expects (path_ptr, path_len, amode), the same argument-convention
 * mismatch open()/chmod()/... needed patching for (see src/fcntl/open.c and src/stat/chmod.c on
 * this same branch), computing path_len via strlen() since the caller only ever gives us a
 * null-terminated C string. SYS_access itself is left at its real, inert Linux number (21) --
 * only the argument shape needed patching, not the number.
 */
int access(const char *filename, int amode)
{
#ifdef SYS_access
	return syscall(SYS_access, filename, strlen(filename), amode);
#else
	return syscall(SYS_faccessat, AT_FDCWD, filename, amode, 0);
#endif
}
