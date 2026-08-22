#include <sys/select.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include "syscall.h"

/* OxideBSD: real select(2) needs 5 real values (n, three fd_set*, and a timeout) and this ABI
 * only carries 4 registers, so this struct bundles them behind one pointer instead -- the kernel
 * side (`crate::net::oxidebsd_sys_select`) mirrors this layout exactly. `tv_sec < 0` is this
 * pair's own "no timeout, wait forever" sentinel, substituted below whenever the caller's own
 * `tv` was NULL. */
struct oxidebsd_select_req {
	int n;
	int pad;
	fd_set *rfds, *wfds, *efds;
	long tv_sec, tv_usec;
};

int select(int n, fd_set *restrict rfds, fd_set *restrict wfds, fd_set *restrict efds, struct timeval *restrict tv)
{
	long s = tv ? tv->tv_sec : -1;
	long us = tv ? tv->tv_usec : 0;

	if (tv && (tv->tv_sec < 0 || tv->tv_usec < 0)) return __syscall_ret(-EINVAL);

	struct oxidebsd_select_req req = { n, 0, rfds, wfds, efds, s, us };
	return syscall_cp(SYS_select, &req, 0, 0, 0);
}
