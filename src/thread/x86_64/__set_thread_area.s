/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
/* OxideBSD patch: real x86_64 Linux sets the FS base via arch_prctl(ARCH_SET_FS, addr) (syscall
 * 158, subcommand 0x1002). OxideBSD has no arch_prctl at all -- this is one of the syscalls this
 * kernel invents for itself (see src/syscall.rs / modules/native_abi in the OxideBSD tree):
 * SYS_SET_FS_BASE (103) takes the TLS base address directly in %rdi, no subcommand needed, so the
 * subcommand-selection step upstream's version did is gone entirely. Still needs the same
 * jnc/neg carry-flag-to-negative-rax adaptation src/syscall_arch.h's C wrappers get, since this
 * bypasses those wrappers and issues `syscall` directly. */
.text
.global __set_thread_area
.hidden __set_thread_area
.type __set_thread_area,@function
__set_thread_area:
	movl $103,%eax          /* OxideBSD SYS_SET_FS_BASE -- TLS base already in %rdi */
	syscall
	jnc 1f
	neg %rax
1:	ret
