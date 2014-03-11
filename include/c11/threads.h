#ifndef __included_c11threads_h__
#define __included_c11threads_h__

#include <sys/cdefs.h>
#include <stdint.h>
#include <time.h>

#ifdef __GNUC__
#define __noreturn __attribute__((noreturn))
#else
#define __noreturn
#endif

#ifndef __restrict
# if ! (2 < __GNUC__ || (2 == __GNUC__ && 95 <= __GNUC_MINOR__))
#  if defined restrict || 199901L <= __STDC_VERSION__
#   define __restrict restrict
#  else
#   define __restrict
#  endif
# endif
#endif

typedef uintptr_t thrd_t;
typedef uintptr_t mtx_t;
typedef uintptr_t cnd_t;

typedef int (*thrd_start_t)(void *);

enum {
    thrd_success = 0,
    thrd_error = 1,
    thrd_busy = 2,
    thrd_nomem = 3
};

enum {
    mtx_plain = 1,
    mtx_timed = 2,
    mtx_recursive = 4
};

__BEGIN_DECLS

int    thrd_create(thrd_t *, thrd_start_t, void *);
thrd_t thrd_current(void);
int    thrd_detach(thrd_t);
int    thrd_equal(thrd_t, thrd_t);
__noreturn void thrd_exit(int);
int    thrd_join(thrd_t, int *);
int    thrd_sleep(const struct timespec *, struct timespec *);
void   thrd_yield(void);

int    mtx_init(mtx_t *, int);
int    mtx_lock(mtx_t *);
int    mtx_timedlock(mtx_t *__restrict, const struct timespec *__restrict);
int    mtx_trylock(mtx_t *);
int    mtx_unlock(mtx_t *);
void   mtx_destroy(mtx_t *);

int    cnd_init(cnd_t *);
int    cnd_signal(cnd_t *);
int    cnd_broadcast(cnd_t *);
int    cnd_wait(cnd_t *, mtx_t *);
int    cnd_timedwait(cnd_t * __restrict, mtx_t * __restrict, const struct timespec *__restrict);
void   cnd_destroy(cnd_t *);

__END_DECLS

#endif

/* vim: set ts=4 sw=4 noai expandtab: */
