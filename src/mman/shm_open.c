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
	if (*(p = __strchrnul(name, '/')) || p==name ||
	    (p-name <= 2 && name[0]=='.' && p[-1]=='.')) {
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
	if (!(name = __shm_mapname(name, buf))) {
		/* OxideBSD: shm_unlink()'s real POSIX ERRORS list (XSH6) only ever
		 * defines EACCES/ENAMETOOLONG/ENOENT for this function -- unlike
		 * shm_open() (which does list EINVAL, "the shm_open() operation is
		 * not supported for the given name"), shm_unlink() has no EINVAL
		 * case at all. A name __shm_mapname() itself rejects as malformed
		 * (empty, an embedded '/', a bare "."/"..") can never identify a
		 * real shared-memory/semaphore object either way, so report it the
		 * same way as "the named object does not exist" instead of a
		 * spec-unsanctioned EINVAL -- shm_open()'s own EINVAL behavior is
		 * untouched, only this function's own return path is remapped.
		 * ENAMETOOLONG stays exactly as __shm_mapname reported it -- that
		 * one *is* a real shm_unlink() error per spec. */
		if (errno == EINVAL) errno = ENOENT;
		return -1;
	}
	return unlink(name);
}
