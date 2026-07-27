#include <sys/socket.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include "syscall.h"

hidden void __convert_scm_timestamps(struct msghdr *, socklen_t);

void __convert_scm_timestamps(struct msghdr *msg, socklen_t csize)
{
	if (SCM_TIMESTAMP == SCM_TIMESTAMP_OLD) return;
	if (!msg->msg_control || !msg->msg_controllen) return;

	struct cmsghdr *cmsg, *last=0;
	long tmp;
	long long tvts[2];
	int type = 0;

	for (cmsg=CMSG_FIRSTHDR(msg); cmsg; cmsg=CMSG_NXTHDR(msg, cmsg)) {
		if (cmsg->cmsg_level==SOL_SOCKET) switch (cmsg->cmsg_type) {
		case SCM_TIMESTAMP_OLD:
			if (type) break;
			type = SCM_TIMESTAMP;
			goto common;
		case SCM_TIMESTAMPNS_OLD:
			type = SCM_TIMESTAMPNS;
		common:
			memcpy(&tmp, CMSG_DATA(cmsg), sizeof tmp);
			tvts[0] = tmp;
			memcpy(&tmp, CMSG_DATA(cmsg) + sizeof tmp, sizeof tmp);
			tvts[1] = tmp;
			break;
		}
		last = cmsg;
	}
	if (!last || !type) return;
	if (CMSG_SPACE(sizeof tvts) > csize-msg->msg_controllen) {
		msg->msg_flags |= MSG_CTRUNC;
		return;
	}
	msg->msg_controllen += CMSG_SPACE(sizeof tvts);
	cmsg = CMSG_NXTHDR(msg, last);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = type;
	cmsg->cmsg_len = CMSG_LEN(sizeof tvts);
	memcpy(CMSG_DATA(cmsg), &tvts, sizeof tvts);
}

/* OxideBSD patch: no real recvmsg(2) syscall exists on this ABI -- __NR_recvmsg's real,
 * unremapped Linux value is inert here (nothing registers it), so every call used to fail with
 * ENOSYS. This is the actual reason it needed fixing at all: real DNS resolution
 * (third_party/musl/src/network/res_msend.c) reads UDP replies via recvmsg(), not recvfrom(),
 * even though it only ever uses a single iovec and no ancillary data -- exactly the shape
 * already-patched recvfrom() (this directory's own recvfrom.c) handles. Before this fix, every
 * reply the kernel correctly delivered got silently dropped here, and since the queued reply
 * never got consumed, a caller polling the same fd in a loop (res_msend.c does exactly this) saw
 * it as perpetually ready -- a tight busy-loop until the resolver's own overall timeout gave up,
 * not a real deadlock, but indistinguishable from a hang for however long that timeout was.
 *
 * Multi-iovec/control-message callers (nothing in this port's roster needs either) get a clean
 * error instead of silently dropping data or misreading the buffer -- no `__convert_scm_timestamps`
 * call needed since a control-message request is exactly the case rejected above. */
ssize_t recvmsg(int fd, struct msghdr *msg, int flags)
{
	if (msg->msg_iovlen > 1 || msg->msg_control) {
		errno = ENOTSUP;
		return -1;
	}
	void *buf = msg->msg_iovlen ? msg->msg_iov[0].iov_base : 0;
	size_t len = msg->msg_iovlen ? msg->msg_iov[0].iov_len : 0;
	return recvfrom(fd, buf, len, flags, msg->msg_name, &msg->msg_namelen);
}
