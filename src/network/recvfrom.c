#include <netinet/in.h>
#include <sys/socket.h>
#include "syscall.h"

/* OxideBSD patch: same story as sendto.c's own comment -- `addr` takes the syscall's 4th slot
 * (R10) in `flags`' place; `alen` is dropped from the wire entirely (AF_INET, the only family
 * OxideBSD's net stack supports so far, always means a fixed 16-byte `struct sockaddr_in`, which
 * is exactly what the kernel-side handler -- src/net/udp.rs's oxidebsd_sys_recvfrom in the
 * OxideBSD tree -- always writes when `addr` is non-null). Real callers that check `*alen` still
 * need a sane value there, so this wrapper fills it in locally after a successful call instead of
 * relying on the kernel to report it. */
ssize_t recvfrom(int fd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict addr, socklen_t *restrict alen)
{
	(void)flags;
	ssize_t r = socketcall_cp(recvfrom, fd, buf, len, addr, 0, 0);
	if (r >= 0 && addr && alen) *alen = sizeof(struct sockaddr_in);
	return r;
}
