#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#ifdef __MACH__
#include <sched.h>
#endif

#include "c11/threads.h"

struct _thrd_wrapper_info {
    thrd_start_t func;
    void *param;
};

typedef union {
    int ival;
    void *ptrval;
} _thrd_retval;

static void *_thrd_wrapper(void *ptr)
{
    _thrd_retval rv;
    struct _thrd_wrapper_info info = *(struct _thrd_wrapper_info *)ptr;
    free(ptr);
    rv.ival = info.func(info.param);
    return rv.ptrval;
}

int thrd_create(thrd_t *_thr, thrd_start_t _func, void *_arg)
{
    struct _thrd_wrapper_info *info = NULL;

    if (!_thr)
        return thrd_error;

    info = malloc(sizeof(struct _thrd_wrapper_info));
    info->func = _func;
    info->param = _arg;

    if (pthread_create((pthread_t *)_thr, NULL, _thrd_wrapper, info) != 0) {
        free(info);
        return thrd_error;
    }

    return thrd_success;
}

thrd_t thrd_current(void)
{
    return (thrd_t)pthread_self();
}

int thrd_detach(thrd_t thr)
{
    return pthread_detach((pthread_t)thr) == 0 ? thrd_success : thrd_error;
}

int thrd_equal(thrd_t a, thrd_t b)
{
    return pthread_equal((pthread_t)a, (pthread_t)b);
}

void thrd_exit(int res)
{
    _thrd_retval rv;
    rv.ival = res;
    pthread_exit(rv.ptrval);
}

int thrd_join(thrd_t thr, int *res)
{
    _thrd_retval rv;

    if (pthread_join((pthread_t)thr, &rv.ptrval) != 0)
        return thrd_error;

    if (res) {
        *res = rv.ival;
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
#ifdef __MACH__
    sched_yield();
#else
    pthread_yield();
#endif
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

#ifdef PTHREAD_MUTEX_TIMED_NP
    if ((_type & mtx_timed) && pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP)) {
        rv = thrd_error;
        goto error1;
    }
#else
    /* Platforms without PTHREAD_MUTEX_TIMED_NP can't do timed mutexes. */
    if (_type & mtx_timed) {
        rv = thrd_error;
        goto error1;
    }
#endif

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

int mtx_timedlock(mtx_t *__restrict _mtx, const struct timespec *__restrict _ts)
{
#ifdef PTHREAD_MUTEX_TIMED_NP
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    switch (pthread_mutex_timedlock(mtx, _ts)) {
    case EBUSY:
        return thrd_busy;
    case 0:
        return thrd_success;
    default:
        return thrd_error;
    }
#else
    return thrd_error;
#endif
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

int cnd_init(cnd_t *_cond)
{
    int rv = thrd_success;
    pthread_cond_t *cond;

    if (!_cond) {
        rv = thrd_error;
        goto error0;
    }

    cond = malloc(sizeof(pthread_cond_t));
    if (!cond) {
        rv = thrd_nomem;
        goto error0;
    }

    if (pthread_cond_init(cond, NULL)) {
        rv = thrd_error;
        goto error1;
    }

    *_cond = (uintptr_t)cond;
    cond = NULL;

error1:
    free(cond);
error0:
    return rv;
}

int cnd_signal(cnd_t *_cond)
{
    pthread_cond_t *cond = (pthread_cond_t *)*_cond;
    if (pthread_cond_signal(cond))
        return thrd_error;
    return thrd_success;
}

int cnd_broadcast(cnd_t *_cond)
{
    pthread_cond_t *cond = (pthread_cond_t *)*_cond;
    if (pthread_cond_broadcast(cond))
        return thrd_error;
    return thrd_success;
}

int cnd_wait(cnd_t *_cond, mtx_t *_mtx)
{
    pthread_cond_t *cond = (pthread_cond_t *)*_cond;
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    if (pthread_cond_wait(cond, mtx))
        return thrd_error;
    return thrd_success;
}

int cnd_timedwait(cnd_t * __restrict _cond, mtx_t * __restrict _mtx, const struct timespec *__restrict _ts)
{
    pthread_cond_t *cond = (pthread_cond_t *)*_cond;
    pthread_mutex_t *mtx = (pthread_mutex_t *)*_mtx;
    switch (pthread_cond_timedwait(cond, mtx, _ts)) {
    case ETIMEDOUT:
        return thrd_busy;
    case 0:
        return thrd_success;
    default:
        return thrd_error;
    }
}

void cnd_destroy(cnd_t *_cond)
{
    pthread_cond_t *cond = (pthread_cond_t *)*_cond;
    pthread_cond_destroy(cond);
    free(cond);
}

/* vim: set ts=4 sw=4 noai expandtab: */
