#include <sys/socket.h>
#include "syscall.h"

/* OxideBSD patch: this ABI only carries 4 syscall arguments (RDI/RSI/RDX/R10) -- the generic
 * socketcall_cp(sendto, fd, buf, len, flags, addr, alen) call places addr/alen in %r8/%r9, both
 * unreachable to the kernel-side handler (see bits/syscall.h.in's own SYS_sendto comment in the
 * OxideBSD tree). `addr` is passed in `flags`' slot instead -- no MSG_* flag has any meaning on
 * the OxideBSD side yet, so nothing is lost dropping it; `alen` is dropped entirely, since AF_INET
 * (the only family OxideBSD's net stack supports so far) always means a fixed 16-byte
 * `struct sockaddr_in`, and the kernel-side handler (src/net/udp.rs's oxidebsd_sys_sendto in the
 * OxideBSD tree) just always reads that many bytes. */
ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t alen)
{
	(void)flags;
	(void)alen;
	return socketcall_cp(sendto, fd, buf, len, addr, 0, 0);
}
