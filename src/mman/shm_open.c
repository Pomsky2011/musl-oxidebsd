#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

char *__shm_mapname(const char *name, char *buf)
{
	char *p;
	while (*name == '/') name++;
	p = __strchrnul(name, '/');
	/* OxideBSD: an empty name (after stripping leading slashes) can never
	 * name a real shm/semaphore object, so treat it the same as "the named
	 * object does not exist" rather than a malformed-input EINVAL -- POSIX
	 * doesn't mandate a specific errno for a malformed name either way, this
	 * is just our own platform's own choice for it. Every other malformed
	 * shape (an embedded '/', a bare "." or "..") stays EINVAL below. */
	if (p == name) {
		errno = ENOENT;
		return 0;
	}
	if (*p || (p-name <= 2 && name[0]=='.' && p[-1]=='.')) {
		errno = EINVAL;
		return 0;
	}
	if (p-name > NAME_MAX) {
		errno = ENAMETOOLONG;
		return 0;
	}
	memcpy(buf, "/dev/shm/", 9);
	memcpy(buf+9, name, p-name+1);
	return buf;
}

int shm_open(const char *name, int flag, mode_t mode)
{
	int cs;
	char buf[NAME_MAX+10];
	if (!(name = __shm_mapname(name, buf))) return -1;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	int fd = open(name, flag|O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK, mode);
	pthread_setcancelstate(cs, 0);
	return fd;
}

int shm_unlink(const char *name)
{
	char buf[NAME_MAX+10];
	if (!(name = __shm_mapname(name, buf))) return -1;
	return unlink(name);
}
