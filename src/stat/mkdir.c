#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real mkdir()'s wire format is (path, mode) in RDI/RSI. OxideBSD's own SYS_MKDIR
 * (modules/fat32/sys_mkdir in the OxideBSD tree) instead expects (path_ptr, path_len) -- a
 * length-prefixed pointer, no null-terminator requirement, and no mode argument at all (this
 * filesystem doesn't model permissions) -- the same argument-convention mismatch open()/chdir()
 * needed patching for (see src/fcntl/open.c and src/unistd/chdir.c on this same branch).
 */
int mkdir(const char *path, mode_t mode)
{
	(void)mode;
	return syscall(SYS_mkdir, path, strlen(path));
}
