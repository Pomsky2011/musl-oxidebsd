#include <unistd.h>
#include <string.h>
#include "syscall.h"

/* OxideBSD patch: real execve(path, argv, envp) passes NUL-terminated char** arrays in
 * RDI/RSI/RDX. OxideBSD's own SYS_execve (src/process.rs's do_execve in the OxideBSD tree) instead
 * expects (path_ptr, path_len, argv_ptr, envp_ptr): path_ptr/path_len name the path directly (no
 * NUL-terminator requirement), and argv_ptr/envp_ptr each point at a sequence of {ptr, len} pairs
 * (16 bytes each, matching src/process.rs's RawArgvEntry exactly), terminated by a {0, 0} entry --
 * see CLAUDE.md's BusyBox section for the full story of why this call needed patching at all.
 *
 * argv[0] is always supplied by the kernel from path_ptr/path_len itself -- there's no way to give
 * a process a different argv[0] than its own exec path under this ABI (a known, separate
 * limitation, not fixed here). argv_ptr only ever describes argv[1..], so real argv[0] is simply
 * dropped below, matching that existing convention exactly (see RawArgvEntry's own doc comment in
 * the OxideBSD tree).
 */

struct raw_argv_entry {
	unsigned long ptr;
	unsigned long len;
};

/* Mirrors src/process.rs's own MAX_PTR_LEN_ENTRIES (32) -- the kernel silently stops reading past
 * that many entries regardless, so building more here would just be wasted stack space. */
#define MAX_EXECVE_ENTRIES 32

int execve(const char *path, char *const argv[], char *const envp[])
{
	struct raw_argv_entry argv_entries[MAX_EXECVE_ENTRIES + 1];
	struct raw_argv_entry envp_entries[MAX_EXECVE_ENTRIES + 1];
	int i;

	for (i = 0; argv && argv[i + 1] && i < MAX_EXECVE_ENTRIES; i++) {
		argv_entries[i].ptr = (unsigned long)argv[i + 1];
		argv_entries[i].len = strlen(argv[i + 1]);
	}
	argv_entries[i].ptr = 0;
	argv_entries[i].len = 0;

	for (i = 0; envp && envp[i] && i < MAX_EXECVE_ENTRIES; i++) {
		envp_entries[i].ptr = (unsigned long)envp[i];
		envp_entries[i].len = strlen(envp[i]);
	}
	envp_entries[i].ptr = 0;
	envp_entries[i].len = 0;

	long ret = __syscall4(SYS_execve, (long)path, (long)strlen(path),
		(long)argv_entries, (long)envp_entries);
	return __syscall_ret(ret);
}
