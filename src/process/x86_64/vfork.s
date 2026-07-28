.global vfork
.type vfork,@function
vfork:
	pop %rdx
	/* OxideBSD has no true vfork (shared-address-space-until-exec) semantics -- this alias to
	 * SYS_FORK's own real number (2, not real Linux vfork's 58) makes vfork() behave exactly
	 * like a real fork() instead, which is a valid, POSIX-legal implementation of vfork() (the
	 * standard explicitly allows treating it as a synonym for fork()). Hardcoded here, not
	 * fixable via arch/x86_64/bits/syscall.h.in's __NR_* table like every other remapped
	 * syscall -- this file is hand-written asm invoking the raw number directly. */
	mov $2,%eax
	syscall
	push %rdx
	mov %rax,%rdi
	.hidden __syscall_ret
	jmp __syscall_ret
