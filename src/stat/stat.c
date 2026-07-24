#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real stat() delegates to fstatat(AT_FDCWD, path, buf, 0), which itself calls
 * through musl's generic kstat-then-copy machinery (src/stat/fstatat.c). OxideBSD's own SYS_STAT
 * (modules/oxfs/oxfs_stat in the OxideBSD tree) instead expects (path_ptr, path_len, buf_ptr) --
 * the same length-prefixed argument-convention mismatch open()/chdir()/mkdir() needed patching
 * for (see src/fcntl/open.c and src/unistd/chdir.c on this same branch) -- and writes a
 * byte-exact struct stat straight into buf_ptr itself, no kstat translation needed (OxideBSD's
 * own MuslStat already matches struct stat's real layout field-for-field). So this bypasses
 * fstatat() entirely rather than patching that generic multiplexer, matching this port's existing
 * per-entry-point pattern.
 */
int stat(const char *restrict path, struct stat *restrict buf)
{
	return syscall(SYS_stat, path, strlen(path), buf);
}
