#include <unistd.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real chdir()'s wire format is a single (path) argument in RDI -- a plain
 * null-terminated C string, nothing else. OxideBSD's own SYS_CHDIR (modules/fat32/sys_chdir in the
 * OxideBSD tree) instead expects (path_ptr, path_len) -- a length-prefixed pointer, no
 * null-terminator requirement -- the same argument-convention mismatch open() needed patching for
 * (see src/fcntl/open.c on this same branch). Left unpatched, path_len (RSI) would carry whatever
 * garbage happened to already be in that register, since real chdir() never sets it.
 */
int chdir(const char *path)
{
	return syscall(SYS_chdir, path, strlen(path));
}
