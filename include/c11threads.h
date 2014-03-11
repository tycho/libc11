#ifndef __included_c11threads_h__
#define __included_c11threads_h__

#include <sys/cdefs.h>
#include <stdint.h>
#include <time.h>

typedef uintptr_t thrd_t;

typedef int (*thrd_start_t)(void *);

enum {
    thrd_success = 0,
    thrd_error = 1,
    thrd_nomem = 3
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

__END_DECLS

#endif

/* vim: set ts=4 sw=4 noai expandtab: */
