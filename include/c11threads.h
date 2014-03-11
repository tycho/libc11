#ifndef __included_c11threads_h__
#define __included_c11threads_h__

#include <sys/cdefs.h>
#include <stdint.h>
#include <time.h>

typedef uintptr_t thrd_t;
typedef uintptr_t mtx_t;

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
_Noreturn void thrd_exit(int);
int    thrd_join(thrd_t, int *);
int    thrd_sleep(const struct timespec *, struct timespec *);
void   thrd_yield(void);

int    mtx_init(mtx_t *, int);
int    mtx_lock(mtx_t *);
int    mtx_timedlock(mtx_t *restrict, const struct timespec *restrict);
int    mtx_trylock(mtx_t *);
int    mtx_unlock(mtx_t *);
void   mtx_destroy(mtx_t *);

__END_DECLS

#endif

/* vim: set ts=4 sw=4 noai expandtab: */
