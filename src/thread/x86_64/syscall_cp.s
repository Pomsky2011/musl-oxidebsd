.text
.global __cp_begin
.hidden __cp_begin
.global __cp_end
.hidden __cp_end
.global __cp_cancel
.hidden __cp_cancel
.hidden __cancel
.global __syscall_cp_asm
.hidden __syscall_cp_asm
.type   __syscall_cp_asm,@function
__syscall_cp_asm:

__cp_begin:
	mov (%rdi),%eax
	test %eax,%eax
	jnz __cp_cancel
	mov %rdi,%r11
	mov %rsi,%rax
	mov %rdx,%rdi
	mov %rcx,%rsi
	mov %r8,%rdx
	mov %r9,%r10
	mov 8(%rsp),%r8
	mov 16(%rsp),%r9
	mov %r11,8(%rsp)
	syscall
	/* OxideBSD patch: this kernel signals a syscall failure via the carry flag (CF=1, positive
	 * errno in %rax), not Linux's plain "negative %rax" convention -- see arch/x86_64/
	 * syscall_arch.h's own identical "jnc 1f; neg %rax; 1:" patch, applied there to every
	 * regular (non-cancellable) inline syscall wrapper. This hand-written cancellation-point
	 * stub is a *separate* code path (`__syscall_cp_c`'s cancel-enabled fast path calls this
	 * asm directly, bypassing syscall_arch.h's macros entirely) that needed the identical fix
	 * -- found live via mq_timedsend/16-1.c in the Open POSIX Test Suite pilot: a real
	 * ETIMEDOUT returned by the kernel (CF=1, RAX=110) passed through this stub unconverted
	 * (RAX=110, a small positive value), which __syscall_ret's real-Linux-style "is this a
	 * huge unsigned value near ULONG_MAX" check then read as an ordinary *successful* return
	 * value instead of a real error -- silently breaking the return-value contract of every
	 * musl function that funnels a real error through this exact path (mq_timedsend/receive,
	 * nanosleep-family, read/write/close, wait4, poll/select, sigtimedwait/sigsuspend, ...).
	 * Kept inside the __cp_begin/__cp_end cancellation window (the two ALU instructions below
	 * have no side effect a cancelling signal could ever need to interrupt mid-sequence). */
	jnc 1f
	neg %rax
1:
__cp_end:
	ret
__cp_cancel:
	jmp __cancel
