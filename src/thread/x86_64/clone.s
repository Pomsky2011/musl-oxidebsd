.text
.global __clone
.hidden __clone
.type   __clone,@function
__clone:
	/* OxideBSD: hand-written asm bypasses bits/syscall.h.in's remap table -- must not use a raw
	 * Linux syscall number directly (same bug class already fixed once for vfork.s). SYS_CLONE
	 * is reserved at 555 (see docs/MISSING_POSIX_SYSCALLS.md), not real Linux's 56 -- no kernel
	 * handler exists yet, so this still cleanly ENOSYS's until real threading is implemented. */
	mov $555,%eax
	mov %rdi,%r11
	mov %rdx,%rdi
	mov %r8,%rdx
	mov %r9,%r8
	mov 8(%rsp),%r10
	mov %r11,%r9
	and $-16,%rsi
	sub $8,%rsi
	mov %rcx,(%rsi)
	syscall
	test %eax,%eax
	jnz 1f
	xor %ebp,%ebp
	pop %rdi
	call *%r9
	mov %eax,%edi
	/* thread fn returned normally: call real exit(status) via this ABI's own SYS_EXIT=1 (was
	 * raw Linux's 60 -- same bypass-the-remap-table bug, fixed the same way). */
	xor %eax,%eax
	mov $1,%al
	syscall
	hlt
1:	ret
