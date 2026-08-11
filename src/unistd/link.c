#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real link()'s wire format is (existing, new) in RDI/RSI -- two plain
 * null-terminated C strings. OxideBSD's own SYS_LINK (modules/oxfs/src/lib.rs's oxfs_link in the
 * OxideBSD tree) instead expects (existing_ptr, existing_len, new_ptr, new_len), the same
 * "two path strings, no other args" shape SYS_RENAME/SYS_SYMLINK already established -- see
 * src/stdio/rename.c on this same branch.
 */
int link(const char *existing, const char *new)
{
#ifdef SYS_link
	return syscall(SYS_link, existing, strlen(existing), new, strlen(new));
#else
	return syscall(SYS_linkat, AT_FDCWD, existing, AT_FDCWD, new, 0);
#endif
}
