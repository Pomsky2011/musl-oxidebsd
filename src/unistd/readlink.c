#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real readlink()'s wire format is (path, buf, bufsize) in RDI/RSI/RDX -- path is
 * a plain null-terminated C string. OxideBSD's own SYS_READLINK (modules/oxfs/src/lib.rs's
 * oxfs_readlink in the OxideBSD tree) instead expects (path_ptr, path_len, buf_ptr, bufsize),
 * using all four of this ABI's argument registers -- same "R10 becomes a real argument" shape
 * SYS_rename already established (see src/stdio/rename.c on this same branch). buf_ptr/bufsize
 * are unchanged, real readlink(2)'s own remaining args.
 */
ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
{
	char dummy[1];
	if (!bufsize) {
		buf = dummy;
		bufsize = 1;
	}
#ifdef SYS_readlink
	long r = __syscall4(SYS_readlink, (long)path, (long)strlen(path), (long)buf, (long)bufsize);
#else
	long r = __syscall(SYS_readlinkat, AT_FDCWD, path, buf, bufsize);
#endif
	if (buf == dummy && r > 0) r = 0;
	return __syscall_ret(r);
}
