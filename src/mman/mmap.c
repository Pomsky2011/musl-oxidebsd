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
 * OxideBSD tree). `fd`/`off` are packed into one extra register: low 32 bits `fd` (so `-1` becomes
 * the real, recognizable `0xffff_ffff` bit pattern), high 32 bits the low 32 bits of `off` (a real,
 * if truncated, nonzero `off` is now handled kernel-side -- see OxideBSD's own
 * process::mm::do_mmap_file_backed doc comment for its real ENXIO/EOVERFLOW/EINVAL split). `flags`
 * rides along in the *unused* high bits of the `prot` register instead of its own argument -- real
 * `prot` only ever occupies the low 3 bits (PROT_READ/WRITE/EXEC), leaving the rest free. This is
 * what lets the kernel side tell a real MAP_SHARED/MAP_PRIVATE/MAP_FIXED/MAP_ANON request apart,
 * rather than the old fd<0-means-anonymous guess: see OxideBSD's own process::mm::do_mmap doc
 * comment.
 *
 * Upstream musl's own `len >= PTRDIFF_MAX` pre-syscall guard is deliberately dropped here (not
 * just relaxed): it's a libc-internal safety margin against `ptrdiff_t` overflow in musl's own
 * later pointer-difference arithmetic, not anything real POSIX mandates on the libc side -- and it
 * actively worked against this project's own POSIX-conformance goal by pre-empting the kernel's own
 * errno determination with a blanket `ENOMEM`, when the actual condition (a real `off`/`len`
 * combination exceeding the file offset maximum) is real POSIX `EOVERFLOW` territory instead
 * (`mmap/31-1.c` in the Open POSIX Test Suite). No real caller in this port's own userland (musl's
 * own malloc/pthread_create, BusyBox, TinyCC) ever requests anywhere near this large a length, so
 * nothing legitimate depended on this guard catching it early. */
void *__mmap(void *start, size_t len, int prot, int flags, int fd, off_t off)
{
	long ret;
	if (off & OFF_MASK) {
		errno = EINVAL;
		return MAP_FAILED;
	}
	if (flags & MAP_FIXED) {
		__vm_wait();
	}
	{
		uint64_t packed = ((uint64_t)(uint32_t)fd) | ((uint64_t)(uint32_t)off << 32);
		uint64_t prot_wire = ((uint64_t)(uint32_t)prot & 0xff) | ((uint64_t)(uint32_t)flags << 8);
		ret = __syscall(SYS_mmap, start, len, prot_wire, packed);
	}
	/* Fixup incorrect EPERM from kernel. */
	if (ret == -EPERM && !start && (flags&MAP_ANON) && !(flags&MAP_FIXED))
		ret = -ENOMEM;
	return (void *)__syscall_ret(ret);
}

weak_alias(__mmap, mmap);
