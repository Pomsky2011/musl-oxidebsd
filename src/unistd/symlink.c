#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real symlink()'s wire format is (existing, new) in RDI/RSI -- two plain
 * null-terminated C strings. OxideBSD's own SYS_SYMLINK (modules/oxfs/src/lib.rs's oxfs_symlink
 * in the OxideBSD tree) instead expects (target_ptr, target_len, linkpath_ptr, linkpath_len),
 * using all four of this ABI's argument registers -- the same shape SYS_rename already
 * established (see src/stdio/rename.c on this same branch).
 */
int symlink(const char *existing, const char *new)
{
#ifdef SYS_symlink
	long ret = __syscall4(SYS_symlink, (long)existing, (long)strlen(existing),
		(long)new, (long)strlen(new));
	return __syscall_ret(ret);
#else
	return syscall(SYS_symlinkat, existing, AT_FDCWD, new);
#endif
}
