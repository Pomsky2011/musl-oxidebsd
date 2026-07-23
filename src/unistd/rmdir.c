#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real rmdir()'s wire format is a single (path) argument in RDI -- a plain
 * null-terminated C string. OxideBSD's own SYS_RMDIR (modules/oxfs/src/lib.rs in the OxideBSD
 * tree) instead expects (path_ptr, path_len) -- same story as unlink()/chdir()/mkdir(), see
 * src/unistd/unlink.c on this same branch.
 */
int rmdir(const char *path)
{
	return syscall(SYS_rmdir, path, strlen(path));
}
