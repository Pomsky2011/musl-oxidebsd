/* OxideBSD patch: this kernel signals a syscall failure via the carry flag (CF=1, positive errno
 * in %rax) instead of Linux's plain "negative %rax" convention -- see src/syscall.rs's module doc
 * comment in the OxideBSD tree. Every wrapper below gets "jnc 1f; neg %rax; 1:" appended right
 * after the `syscall` instruction, converting our convention into the shape the rest of this
 * unmodified musl source tree already expects (__syscall_ret, in syscall.h, treats a small
 * negative return as -errno regardless of *why* the kernel signaled failure). `%=` makes the local
 * label unique per inlining site, since these are `static __inline` functions -- a bare numeric
 * label would collide across multiple call sites the assembler sees after inlining. `"cc"` is
 * added to every clobber list since `neg` touches flags. The trap instruction itself (`syscall`)
 * and the argument-register placement are both unchanged from upstream musl -- OxideBSD's own ABI
 * already uses SYSCALL/SYSRETQ with the same RDI/RSI/RDX/R10/R8/R9 argument placement real BSD
 * uses, so nothing else here needed to change. */
#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

static __inline long __syscall0(long n)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n) : "rcx", "r11", "cc", "memory");
	return ret;
}

static __inline long __syscall1(long n, long a1)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "cc", "memory");
	return ret;
}

static __inline long __syscall2(long n, long a1, long a2)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
						  : "rcx", "r11", "cc", "memory");
	return ret;
}

static __inline long __syscall3(long n, long a1, long a2, long a3)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3) : "rcx", "r11", "cc", "memory");
	return ret;
}

static __inline long __syscall4(long n, long a1, long a2, long a3, long a4)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10): "rcx", "r11", "cc", "memory");
	return ret;
}

static __inline long __syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "cc", "memory");
	return ret;
}

static __inline long __syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	register long r9 __asm__("r9") = a6;
	__asm__ __volatile__ ("syscall\n\tjnc 1%=f\n\tneg %%rax\n1%=:"
		: "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "cc", "memory");
	return ret;
}

/* No VDSO on OxideBSD -- there's no mapped vdso page at all, so musl must not try to resolve or
 * call through one. Upstream gates this whole mechanism behind VDSO_USEFUL being defined, so
 * simply not defining it (unlike upstream's x86_64 target) is enough. */

#define IPC_64 0
