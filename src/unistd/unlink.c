#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real unlink()'s wire format is a single (path) argument in RDI -- a plain
 * null-terminated C string. OxideBSD's own SYS_UNLINK (modules/oxfs/src/lib.rs in the OxideBSD
 * tree) instead expects (path_ptr, path_len) -- the same argument-convention mismatch
 * open()/chdir()/mkdir() needed patching for (see src/fcntl/open.c and src/unistd/chdir.c on this
 * same branch), computing path_len via strlen() since the caller only ever gives us a
 * null-terminated C string.
 */
int unlink(const char *path)
{
	return syscall(SYS_unlink, path, strlen(path));
}
