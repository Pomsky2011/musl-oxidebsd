#define _GNU_SOURCE
#include <sys/uio.h>
#include <unistd.h>
#include "syscall.h"

/* oxidebsd: this ABI's SYSCALL dispatch only ever forwards 4 real arguments (RDI/RSI/RDX/R10),
 * not real Linux's full 6-register convention -- real pwritev2(2)'s wire format (fd, iov, count,
 * ofs_lo, ofs_hi, flags) doesn't fit. Patched to call SYS_pwritev2 unconditionally with just
 * (fd, iov, count, ofs) -- ofs_lo/ofs_hi collapse into one real 64-bit value (the split only ever
 * existed for 32-bit-ABI portability shared across archs; off_t is already 64-bit natively here),
 * and flags is dropped entirely (OxideBSD's own oxfs has no real effect for any RWF_* flag to
 * honor). The real !flags/SYS_pwritev short-circuit below is also removed -- SYS_pwritev (296) was
 * never separately registered on this kernel either, so routing everything through the one number
 * this fork's own kernel actually implements avoids needing a second one for no real benefit. */
ssize_t pwritev2(int fd, const struct iovec *iov, int count, off_t ofs, int flags)
{
	(void)flags;
	return syscall_cp(SYS_pwritev2, fd, iov, count, (long)ofs);
}
