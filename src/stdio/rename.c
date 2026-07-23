#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real rename()'s wire format is (old, new) in RDI/RSI -- two plain
 * null-terminated C strings. OxideBSD's own SYS_RENAME (modules/oxfs/src/lib.rs in the OxideBSD
 * tree) instead expects (old_ptr, old_len, new_ptr, new_len), using all four of this ABI's
 * argument registers -- the same "R10 becomes a real argument" precedent execve's envp_ptr set
 * (see src/process/execve.c on this same branch).
 */
int rename(const char *old, const char *new)
{
	long ret = __syscall4(SYS_rename, (long)old, (long)strlen(old),
		(long)new, (long)strlen(new));
	return __syscall_ret(ret);
}
