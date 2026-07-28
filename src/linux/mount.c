#include <sys/mount.h>
#include <string.h>
#include <errno.h>
#include "syscall.h"

/* OxideBSD patch: real mount()'s wire format is (special, dir, fstype, flags, data) -- five
 * conceptual args, doesn't fit this ABI's 4 registers (RDI/RSI/RDX/R10). modules/oxfs's own mount
 * table (modules/oxfs/src/lib.rs in the OxideBSD tree) only ever supports the two shapes BusyBox's
 * own util-linux/mount.c actually issues -- `mount(source, target, NULL, MS_BIND, NULL)` for
 * `--bind`, `mount("tmpfs", target, "tmpfs", flags, options)` for `-t tmpfs` -- so this dispatches
 * to one of two dedicated syscalls instead of forcing one idealized shape the way chown()/rename()
 * were remapped. Reuses the real, still-distinctly-named create_module/init_module syscall slots
 * (174/175) rather than inventing fictional new macro names -- see arch/x86_64/bits/syscall.h.in's
 * own create_module comment for the full reasoning (why those numbers, and why not the
 * SYS_utimensat=167-adjacent 168-170 range every prior addition otherwise continues into). Any
 * other real mount() shape (a real block-device mount, any other fstype) isn't supported by this
 * port's kernel side at all -- fails here with ENODEV rather than ever reaching the kernel with a
 * request it has no way to honor.
 */
int mount(const char *special, const char *dir, const char *fstype, unsigned long flags, const void *data)
{
	(void)data;
	if (fstype && !strcmp(fstype, "tmpfs")) {
		long ret = __syscall2(SYS_init_module, (long)dir, (long)strlen(dir));
		return __syscall_ret(ret);
	}
	if (flags & MS_BIND) {
		long ret = __syscall4(SYS_create_module, (long)special, (long)strlen(special),
			(long)dir, (long)strlen(dir));
		return __syscall_ret(ret);
	}
	errno = ENODEV;
	return -1;
}

/* OxideBSD patch: real umount()/umount2()'s own (special[, flags]) wire format fits this ABI's 4
 * registers whole once path_len is added -- the same "compute strlen() explicitly" patch
 * chown()/rename() already use, no shape change needed otherwise. Reuses the real delete_module
 * syscall slot (176), same reasoning as mount()'s own comment above -- this file's own SYS_umount2
 * macro stays at its original, inert real-Linux value (166), unreferenced from here on.
 */
int umount(const char *special)
{
	long ret = __syscall4(SYS_delete_module, (long)special, (long)strlen(special), 0, 0);
	return __syscall_ret(ret);
}

int umount2(const char *special, int flags)
{
	long ret = __syscall4(SYS_delete_module, (long)special, (long)strlen(special), (long)flags, 0);
	return __syscall_ret(ret);
}
