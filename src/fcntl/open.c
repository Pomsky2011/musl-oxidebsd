#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real open()'s wire format is (path, flags, mode) in RDI/RSI/RDX. OxideBSD's own
 * SYS_OPEN (modules/oxfs/oxfs_open in the OxideBSD tree) instead expects (path_ptr, path_len,
 * flags) -- a length-prefixed pointer, no null-terminator requirement, and no mode argument at all
 * (this filesystem doesn't model permissions). The generic __sys_open_cp/__sys_open3 macros
 * (src/internal/syscall.h) assume the real layout too and can't be reused here -- this calls
 * __syscall3 directly with OxideBSD's own argument shape instead, computing path_len via strlen()
 * since the caller only ever gives us a null-terminated C string. mode is read (to stay
 * ABI-compatible with any caller passing the vararg) but discarded; O_CLOEXEC's own follow-up
 * fcntl() is dropped too, since SYS_FCNTL isn't implemented -- it would just log an
 * unrecognized-syscall-number line for no benefit (no real fd table entry it could mark CLOEXEC on
 * yet). This is the *public* open(2) entry point specifically -- internal stdio callers
 * (fopen()/tmpfile()/__fopen_rb_ca()) bypass this file entirely via sys_open()/__sys_open(), a
 * separate macro-based path that needed the exact same fix, applied directly in
 * src/internal/syscall.h (see that file's own comment for the real bug this caused: fopen()'s own
 * sys_open() call reached oxfs_open with real flags/mode reinterpreted as OxideBSD's path
 * length/flags, walking off the end of the real filename's mapped memory).
 *
 * A real open(2) leaves NULL-pointer validation to the kernel, which faults on the bad address
 * and hands back EFAULT to the caller -- callers rely on getting that error back, not a crash
 * (e.g. BusyBox vi's edit_file() path calls open(NULL, ...) on purpose when launched with no
 * filename argument, and expects a clean failure). Computing path_len via strlen() on the
 * userland side loses that safety net -- strlen(NULL) itself faults, in this libc rather than in
 * the kernel, before the syscall (and its normal EFAULT handling) is ever reached. Check for NULL
 * up front and synthesize the same EFAULT a real kernel would produce. */
int open(const char *filename, int flags, ...)
{
	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		va_arg(ap, mode_t);
		va_end(ap);
	}

	if (!filename)
		return __syscall_ret(-EFAULT);

	int fd = __syscall3(SYS_open, (long)filename, (long)strlen(filename), flags);

	return __syscall_ret(fd);
}
