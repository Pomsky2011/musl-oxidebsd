	nop
.global __restore_rt
.hidden __restore_rt
.type __restore_rt,@function
__restore_rt:
	/* OxideBSD's own SYS_SIGRETURN (119), not Linux's real __NR_rt_sigreturn (15) -- every arch's
	 * restore.s hardcodes its own trap number as a literal here rather than going through
	 * bits/syscall.h.in's __NR_rt_sigreturn macro (confirmed by grepping every other arch's own
	 * restore.s in this tree), so remapping that header alone would not have reached this file. */
	mov $119, %rax
	syscall
.size __restore_rt,.-__restore_rt
