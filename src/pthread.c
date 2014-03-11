#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include "c11threads.h"

int thrd_create(thrd_t *_thr, thrd_start_t _func, void *_arg)
{
    if (!_thr)
        return thrd_error;

    if (pthread_create(_thr, NULL, (void*(*)(void*))_func, _arg) != 0)
        return thrd_error;

    return thrd_success;
}

thrd_t thrd_current(void)
{
    return (thrd_t)pthread_self();
}

int thrd_detach(thrd_t thr)
{
    return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
}

int thrd_equal(thrd_t a, thrd_t b)
{
    return pthread_equal(a, b);
}

void thrd_exit(int res)
{
    uintptr_t v;
    v = (uintptr_t)res;
    pthread_exit((void*)v);
}

int thrd_join(thrd_t thr, int *res)
{
    void *rv;
    uintptr_t r;

    if (pthread_join((pthread_t)thr, &rv) != 0)
        return thrd_error;

    if (res) {
        r = (uintptr_t)rv;
        *res = (int)r;
    }

    return thrd_success;
}

int thrd_sleep(const struct timespec *time_point,
                struct timespec *remaining)
{
    return nanosleep(time_point, remaining);
}

void thrd_yield(void)
{
    pthread_yield();
}

/* vim: set ts=4 sw=4 noai expandtab: */
