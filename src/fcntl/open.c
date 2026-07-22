#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real open()'s wire format is (path, flags, mode) in RDI/RSI/RDX. OxideBSD's own
 * SYS_OPEN (modules/fat32/fat32_open in the OxideBSD tree) instead expects (path_ptr, path_len,
 * flags) -- a length-prefixed pointer, no null-terminator requirement, and no mode argument at all
 * (this filesystem doesn't model permissions). __sys_open_cp/__sys_open3 (bits/syscall.h's
 * generic macros) assume the real layout and can't be reused here -- this calls __syscall3
 * directly with OxideBSD's own argument shape instead, computing path_len via strlen() since the
 * caller only ever gives us a null-terminated C string. mode is read (to stay ABI-compatible with
 * any caller passing the vararg) but discarded; O_CLOEXEC's own follow-up fcntl() is dropped too,
 * since SYS_FCNTL isn't implemented -- it would just log an unrecognized-syscall-number line for
 * no benefit (no real fd table entry it could mark CLOEXEC on yet). */
int open(const char *filename, int flags, ...)
{
	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		va_arg(ap, mode_t);
		va_end(ap);
	}

	int fd = __syscall3(SYS_open, (long)filename, (long)strlen(filename), flags);

	return __syscall_ret(fd);
}
