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

int mtx_init(mtx_t *_mtx, int _type)
{
    int rv = thrd_success;
    pthread_mutexattr_t attr;
    pthread_mutex_t *mtx;

    if (!_mtx)
        return thrd_error;

    /* mtx_plain and mtx_timed are mutually exclusive options, and exactly one
     * of the two must be set.
     */
    if (!((_type & mtx_plain) ^ (_type & mtx_timed)))
        return thrd_error;

    if (pthread_mutexattr_init(&attr)) {
        if (errno == ENOMEM)
            rv = thrd_nomem;
        else
            rv = thrd_error;
        goto error0;
    }

    if ((_type & mtx_plain) && pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL)) {
        rv = thrd_error;
        goto error1;
    }

    if ((_type & mtx_timed) && pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP)) {
        rv = thrd_error;
        goto error1;
    }

    if ((_type & mtx_recursive) && pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) {
        rv = thrd_error;
        goto error1;
    }

    mtx = malloc(sizeof(pthread_mutex_t));
    if (!mtx) {
        rv = thrd_nomem;
        goto error1;
    }

    if (pthread_mutex_init(mtx, &attr)) {
        rv = thrd_error;
        goto error2;
    }

    *_mtx = (uintptr_t)mtx;
    mtx = NULL;

error2:
    free(mtx);
error1:
    pthread_mutexattr_destroy(&attr);
error0:
    return rv;
}

int mtx_lock(mtx_t *_mtx)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    switch (pthread_mutex_lock(mtx)) {
    case EDEADLK:
        return thrd_busy;
    case 0:
        return thrd_success;
    default:
        return thrd_error;
    }
}

int mtx_timedlock(mtx_t *restrict _mtx, const struct timespec *restrict _ts)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    switch (pthread_mutex_timedlock(mtx, _ts)) {
    case EBUSY:
        return thrd_busy;
    case 0:
        return thrd_success;
    default:
        return thrd_error;
    }
}

int mtx_trylock(mtx_t *_mtx)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    switch (pthread_mutex_trylock(mtx)) {
    case EBUSY:
        return thrd_busy;
    case 0:
        return thrd_success;
    default:
        return thrd_error;
    }
}

int mtx_unlock(mtx_t *_mtx)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    switch (pthread_mutex_unlock(mtx)) {
    case 0:
        return thrd_success;
    default:
        return thrd_error;
    }
}

void mtx_destroy(mtx_t *_mtx)
{
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    pthread_mutex_destroy(mtx);
    free(mtx);
}

/* vim: set ts=4 sw=4 noai expandtab: */
