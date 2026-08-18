#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include "syscall.h"

static void dummy(void) { }
weak_alias(dummy, __vm_wait);

#define UNIT SYSCALL_MMAP2_UNIT
#define OFF_MASK ((-0x2000ULL << (8*sizeof(syscall_arg_t)-1)) | (UNIT-1))

/* OxideBSD patch: real mmap(2) needs 6 args (start, len, prot, flags, fd, off), but this ABI only
 * carries 4 registers to a syscall handler (see src/syscall.rs's own module doc comment in the
 * OxideBSD tree). `flags` itself is dropped entirely -- OxideBSD's own SYS_MMAP distinguishes
 * anonymous vs. real fd-backed MAP_SHARED purely by `fd == -1` vs. `fd >= 0` (this file's own
 * existing MAP_ANON-implies-fd==-1 convention already guarantees that), and always treats a
 * real-fd mapping as MAP_SHARED (no real per-page fault tracking exists to support MAP_PRIVATE's
 * copy-on-write semantics against a real fd). `fd`/`off` are packed into one extra register
 * instead: low 32 bits `fd` (so `-1` becomes the real, recognizable `0xffff_ffff` bit pattern),
 * high 32 bits `off` (kernel-side requires this to be `0` -- see OxideBSD's own
 * process::mm::do_mmap_file_backed doc comment for why a nonzero value is an honest EINVAL rather
 * than silently supported; no real caller in this port's own userland ever requests one). */
void *__mmap(void *start, size_t len, int prot, int flags, int fd, off_t off)
{
	long ret;
	if (off & OFF_MASK) {
		errno = EINVAL;
		return MAP_FAILED;
	}
	if (len >= PTRDIFF_MAX) {
		errno = ENOMEM;
		return MAP_FAILED;
	}
	if (flags & MAP_FIXED) {
		__vm_wait();
	}
	{
		uint64_t packed = ((uint64_t)(uint32_t)fd) | ((uint64_t)(uint32_t)off << 32);
		ret = __syscall(SYS_mmap, start, len, prot, packed);
	}
	/* Fixup incorrect EPERM from kernel. */
	if (ret == -EPERM && !start && (flags&MAP_ANON) && !(flags&MAP_FIXED))
		ret = -ENOMEM;
	return (void *)__syscall_ret(ret);
}

weak_alias(__mmap, mmap);
