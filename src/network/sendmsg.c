#include <sys/socket.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "syscall.h"

/* OxideBSD patch: no real sendmsg(2) syscall exists on this ABI, same story as recvmsg.c's own
 * comment. Only ever reached from res_msend.c's start_tcp() (a 2-iovec message: a 2-byte length
 * prefix, then the DNS query itself) along the rare truncated-response TCP fallback path -- DNS
 * queries themselves are always small, so coalescing into a fixed stack buffer and delegating to
 * already-patched sendto() is simpler than inventing a real multi-iovec syscall for what is, in
 * practice, one caller. Ancillary data (SCM_RIGHTS fd-passing, ...) isn't supported at all -- a
 * clean error instead of silently dropping it. */
ssize_t sendmsg(int fd, const struct msghdr *msg, int flags)
{
	if (msg->msg_controllen) {
		errno = ENOTSUP;
		return -1;
	}
	unsigned char buf[1500];
	size_t total = 0;
	for (int i = 0; i < msg->msg_iovlen; i++) {
		size_t len = msg->msg_iov[i].iov_len;
		if (total + len > sizeof buf) {
			errno = EMSGSIZE;
			return -1;
		}
		memcpy(buf + total, msg->msg_iov[i].iov_base, len);
		total += len;
	}
	return sendto(fd, buf, total, flags, msg->msg_name, msg->msg_namelen);
}
