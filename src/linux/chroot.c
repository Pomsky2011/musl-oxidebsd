#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real chroot()'s wire format is a single (path) argument in RDI -- a plain
 * null-terminated C string. OxideBSD's own SYS_CHROOT (modules/oxfs/src/lib.rs's oxfs_chroot in
 * the OxideBSD tree) instead expects (path_ptr, path_len), the same argument-convention mismatch
 * unlink()/rmdir() needed patching for -- see src/unistd/unlink.c on this same branch.
 */
int chroot(const char *path)
{
	return syscall(SYS_chroot, path, strlen(path));
}
