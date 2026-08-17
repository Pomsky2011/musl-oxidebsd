#include <sys/msg.h>
#include "syscall.h"
#include "ipc.h"

ssize_t msgrcv(int q, void *m, size_t len, long type, int flag)
{
#ifndef SYS_ipc
	/* OxideBSD's own native ABI carries only 4 register-width syscall args (RDI/RSI/RDX/R10 --
	 * see arch/x86_64/syscall_arch.h and src/syscall/mod.rs's own doc comment), one short of
	 * the 5 real Linux msgrcv(2) needs. q (a small msqid) and flag (IPC_NOWAIT/MSG_NOERROR/
	 * MSG_EXCEPT, tiny bit flags) are packed into a single register instead of dropped -- same
	 * shape as src/mq/mq_timedsend.c's own mqd+len patch: high 32 bits = flag, low 32 bits =
	 * q. */
	return syscall_cp(SYS_msgrcv, (long)(unsigned)q | ((long)(unsigned)flag << 32), m, len, type);
#else
	return syscall_cp(SYS_ipc, IPCOP_msgrcv, q, len, flag, ((long[]){ (long)m, type }));
#endif
}
