#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real open()'s wire format is (path, flags, mode) in RDI/RSI/RDX. OxideBSD's own
 * SYS_OPEN (modules/oxfs/oxfs_open in the OxideBSD tree) instead expects (path_ptr, path_len,
 * flags, mode) -- a length-prefixed pointer, no null-terminator requirement, and mode moved to a
 * real 4th argument (R10) rather than packed into the 3-argument RDI/RSI/RDX shape real open(2)
 * uses. The generic __sys_open_cp/__sys_open3 macros (src/internal/syscall.h) assume the real
 * 3-register layout too and can't be reused here -- this calls __syscall4 directly with OxideBSD's
 * own argument shape instead, computing path_len via strlen() since the caller only ever gives us
 * a null-terminated C string. O_CLOEXEC's own follow-up fcntl() is dropped too, since SYS_FCNTL
 * isn't implemented for this -- it would just log an unrecognized-syscall-number line for no
 * benefit (no real fd table entry it could mark CLOEXEC on yet). This is the *public* open(2)
 * entry point specifically -- internal stdio callers (fopen()/tmpfile()/__fopen_rb_ca()) bypass
 * this file entirely via sys_open()/__sys_open(), a separate macro-based path that needed the
 * exact same fix, applied directly in src/internal/syscall.h (see that file's own comment for the
 * real bug this caused: fopen()'s own sys_open() call reached oxfs_open with real flags/mode
 * reinterpreted as OxideBSD's path length/flags, walking off the end of the real filename's mapped
 * memory).
 *
 * mode used to be read here (to stay ABI-compatible with any caller passing the vararg) but
 * discarded outright -- a real, found-live bug, not a deliberate simplification: OxideBSD's own
 * SYS_OPEN had no way to carry it at all, so every `open(O_CREAT, mode)` silently got the
 * filesystem's own fixed default permission bits regardless of what the caller actually asked
 * for. Confirmed breaking real POSIX conformance (`sem_open/3-1.c`, Open POSIX Test Suite --
 * a semaphore created 0444 needs its own restricted mode to actually land on the inode for a
 * later write-access re-open to correctly EACCES). Fixed by threading it through as a real
 * 4th syscall argument instead of discarding it -- see modules/oxfs's own `oxfs_open` doc comment
 * on the OxideBSD side for the kernel-side half of this fix.
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
	mode_t mode = 0;
	if ((flags & O_CREAT) || (flags & O_TMPFILE) == O_TMPFILE) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
	}

	if (!filename)
		return __syscall_ret(-EFAULT);

	int fd = __syscall4(SYS_open, (long)filename, (long)strlen(filename), flags, mode);

	return __syscall_ret(fd);
}
