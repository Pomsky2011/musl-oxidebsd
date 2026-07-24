#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: same (path_ptr, path_len, buf_ptr) argument-convention fix src/stat/stat.c
 * needed (see that file's own comment). OxideBSD's oxfs has no symlinks at all, so unlike real
 * lstat(), there's no "don't follow the final component" distinction to preserve -- SYS_LSTAT
 * (modules/oxfs/oxfs_lstat) is registered as a plain alias of oxfs_stat's own resolver.
 */
int lstat(const char *restrict path, struct stat *restrict buf)
{
	return syscall(SYS_lstat, path, strlen(path), buf);
}
