/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.text
.global __unmapself
.type   __unmapself,@function
__unmapself:
	/* OxideBSD: real Linux's raw munmap(11)/exit(60) numbers bypass bits/syscall.h.in's remap
	 * table (same bug class already fixed once for vfork.s) -- call this ABI's own real,
	 * already-working SYS_MUNMAP=101/SYS_EXIT=1 handlers directly instead. Argument registers
	 * already match both signatures (rdi=addr,rsi=len / rdi=code), no shuffling needed. */
	movl $101,%eax  /* SYS_MUNMAP */
	syscall         /* munmap(arg2,arg3) */
	xor %rdi,%rdi   /* exit() args: always return success */
	movl $1,%eax    /* SYS_EXIT */
	syscall         /* exit(0) */
