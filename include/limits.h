#ifndef _LIMITS_H
#define _LIMITS_H

#include <features.h>

#include <bits/alltypes.h> /* __LONG_MAX */

/* Support signed or unsigned plain-char */

#if '\xff' > 0
#define CHAR_MIN 0
#define CHAR_MAX 255
#else
#define CHAR_MIN (-128)
#define CHAR_MAX 127
#endif

#define CHAR_BIT 8
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define SHRT_MIN  (-1-0x7fff)
#define SHRT_MAX  0x7fff
#define USHRT_MAX 0xffff
#define INT_MIN  (-1-0x7fffffff)
#define INT_MAX  0x7fffffff
#define UINT_MAX 0xffffffffU
#define LONG_MIN (-LONG_MAX-1)
#define LONG_MAX __LONG_MAX
#define ULONG_MAX (2UL*LONG_MAX+1)
#define LLONG_MIN (-LLONG_MAX-1)
#define LLONG_MAX  0x7fffffffffffffffLL
#define ULLONG_MAX (2ULL*LLONG_MAX+1)

#define MB_LEN_MAX 4

#if defined(_POSIX_SOURCE) || defined(_POSIX_C_SOURCE) \
 || defined(_XOPEN_SOURCE) || defined(_GNU_SOURCE) || defined(_BSD_SOURCE)

#include <bits/limits.h>

#define PIPE_BUF 4096
#define FILESIZEBITS 64
#ifndef NAME_MAX
#define NAME_MAX 255
#endif
#define PATH_MAX 4096
#define NGROUPS_MAX 32
#define ARG_MAX 131072
#define IOV_MAX 1024
#define SYMLOOP_MAX 40
#define WORD_BIT 32
#define SSIZE_MAX LONG_MAX
#define TZNAME_MAX 6
#define TTY_NAME_MAX 32
#define HOST_NAME_MAX 255

#if LONG_MAX == 0x7fffffffL
#define LONG_BIT 32
#else
#define LONG_BIT 64
#endif

/* Implementation choices... */

#define PTHREAD_KEYS_MAX 128
/* OxideBSD patch: real, unmodified musl upstream defines this as 2048 -- not a multiple of any
 * real page size (4 KiB on this target; 16 KiB/64 KiB on future ports this project's own
 * multi-arch goal targets, e.g. Apple Silicon-class ARM64). The Open POSIX Test Suite's own
 * `threads_scenarii.c` helper (shared by pthread_create/pthread_detach/pthread_exit/pthread_join)
 * refuses to run at all -- a real, unconditional UNTESTED -- whenever
 * `sysconf(_SC_THREAD_STACK_MIN) % sysconf(_SC_PAGESIZE) != 0`, found live by comparing this
 * kernel's own full-corpus pilot run against the same corpus on a real glibc host (whose own
 * page-aligned PTHREAD_STACK_MIN, 16384, never trips this): 19 real files bail out before testing
 * anything, most of which pass cleanly on the host. 64 KiB is a multiple of every real page size
 * this project has ever cared about (4 KiB/16 KiB/64 KiB), so this stays correct across every
 * future architecture port, not just this one's fix. Still comfortably under
 * `DEFAULT_STACK_SIZE` (128 KiB, src/internal/pthread_impl.h) -- a real caller's own minimum
 * request can never exceed the implementation's own unrequested default. **Also required a real
 * companion fix**: `src/conf/sysconf.c`'s own `values[]` table (real, previously-unpatched musl
 * core, not just this arch's own patch surface) stored every `_SC_*` entry as a 16-bit `short` --
 * 65536 silently truncates to `0` in one, confirmed live the hard way (this exact value change,
 * on its own, measured zero effect on a real full-corpus pilot re-run). See that file's own doc
 * comment for the widening this needed. Confirmed via an isolated A/B canary run that the real,
 * pre-existing `pthread_cond_broadcast/{1-2,2-3,4-2}.c` TIMEOUTs (a 10,000-real-thread stress
 * test, `MAX_THREAD_CHILDREN`) are unaffected by this value either way -- all three time out
 * identically at `2048` and at `65536`, so this bump isn't the cause of that pre-existing,
 * already-documented, bounded characteristic. */
#define PTHREAD_STACK_MIN 65536
#define PTHREAD_DESTRUCTOR_ITERATIONS 4
#define SEM_VALUE_MAX 0x7fffffff
#define SEM_NSEMS_MAX 256
#define DELAYTIMER_MAX 0x7fffffff
#define MQ_PRIO_MAX 32768
#define LOGIN_NAME_MAX 256

/* Arbitrary numbers... */

#define BC_BASE_MAX 99
#define BC_DIM_MAX 2048
#define BC_SCALE_MAX 99
#define BC_STRING_MAX 1000
#define CHARCLASS_NAME_MAX 14
#define COLL_WEIGHTS_MAX 2
#define EXPR_NEST_MAX 32
#define LINE_MAX 4096
#define RE_DUP_MAX 255

#define NL_ARGMAX 9
#define NL_MSGMAX 32767
#define NL_SETMAX 255
#define NL_TEXTMAX 2048

#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) || defined(_XOPEN_SOURCE)

#ifdef PAGESIZE
#define PAGE_SIZE PAGESIZE
#endif
#define NZERO 20
#define NL_LANGMAX 32

#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE) \
 || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 < 700)

#define NL_NMAX 16

#endif

/* POSIX/SUS requirements follow. These numbers come directly
 * from SUS and have nothing to do with the host system. */

#define _POSIX_AIO_LISTIO_MAX   2
#define _POSIX_AIO_MAX          1
#define _POSIX_ARG_MAX          4096
#define _POSIX_CHILD_MAX        25
#define _POSIX_CLOCKRES_MIN     20000000
#define _POSIX_DELAYTIMER_MAX   32
#define _POSIX_HOST_NAME_MAX    255
#define _POSIX_LINK_MAX         8
#define _POSIX_LOGIN_NAME_MAX   9
#define _POSIX_MAX_CANON        255
#define _POSIX_MAX_INPUT        255
#define _POSIX_MQ_OPEN_MAX      8
#define _POSIX_MQ_PRIO_MAX      32
#define _POSIX_NAME_MAX         14
#define _POSIX_NGROUPS_MAX      8
#define _POSIX_OPEN_MAX         20
#define _POSIX_PATH_MAX         256
#define _POSIX_PIPE_BUF         512
#define _POSIX_RE_DUP_MAX       255
#define _POSIX_RTSIG_MAX        8
#define _POSIX_SEM_NSEMS_MAX    256
#define _POSIX_SEM_VALUE_MAX    32767
#define _POSIX_SIGQUEUE_MAX     32
#define _POSIX_SSIZE_MAX        32767
#define _POSIX_STREAM_MAX       8
#define _POSIX_SS_REPL_MAX      4
#define _POSIX_SYMLINK_MAX      255
#define _POSIX_SYMLOOP_MAX      8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS 4
#define _POSIX_THREAD_KEYS_MAX  128
#define _POSIX_THREAD_THREADS_MAX 64
#define _POSIX_TIMER_MAX        32
#define _POSIX_TRACE_EVENT_NAME_MAX 30
#define _POSIX_TRACE_NAME_MAX   8
#define _POSIX_TRACE_SYS_MAX    8
#define _POSIX_TRACE_USER_EVENT_MAX 32
#define _POSIX_TTY_NAME_MAX     9
#define _POSIX_TZNAME_MAX       6
#define _POSIX2_BC_BASE_MAX     99
#define _POSIX2_BC_DIM_MAX      2048
#define _POSIX2_BC_SCALE_MAX    99
#define _POSIX2_BC_STRING_MAX   1000
#define _POSIX2_CHARCLASS_NAME_MAX 14
#define _POSIX2_COLL_WEIGHTS_MAX 2
#define _POSIX2_EXPR_NEST_MAX   32
#define _POSIX2_LINE_MAX        2048
#define _POSIX2_RE_DUP_MAX      255

#define _XOPEN_IOV_MAX          16
#define _XOPEN_NAME_MAX         255
#define _XOPEN_PATH_MAX         1024

#endif
