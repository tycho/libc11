#include <windows.h>

#include "c11/threads.h"

struct _thrd_wrapper_info {
	thrd_start_t func;
	void *param;
};

typedef union {
	int ival;
	DWORD winval;
} _thrd_retval;

static DWORD WINAPI _thrd_wrapper(LPVOID ptr)
{
	_thrd_retval rv;
	struct _thrd_wrapper_info info = *(struct _thrd_wrapper_info *)ptr;
	free(ptr);
	rv.ival = info.func(info.param);
	return rv.winval;
}

int thrd_create(thrd_t *_thr, thrd_start_t _func, void *_arg)
{
	struct _thrd_wrapper_info *info = NULL;

	if (!_thr)
		return thrd_error;

	info = malloc(sizeof(struct _thrd_wrapper_info));
	info->func = _func;
	info->param = _arg;

	*_thr = (thrd_t)CreateThread(NULL, 0, _thrd_wrapper, (LPVOID)info, 0, NULL);
	if (*_thr == (thrd_t)NULL) {
		free(info);
		return thrd_error;
	}

	return thrd_success;
}

thrd_t thrd_current(void)
{
	return (thrd_t)GetCurrentThread();
}

int thrd_detach(thrd_t _thr)
{
	return CloseHandle((HANDLE)_thr) != 0 ? thrd_success : thrd_error;
}

int thrd_equal(thrd_t a, thrd_t b)
{
	return a == b;
}

__noreturn void thrd_exit(int res)
{
	_thrd_retval rv;
	rv.ival = res;
	ExitThread(rv.winval);
}

int thrd_join(thrd_t _thr, int *_rv)
{
	_thrd_retval rv;

	if (WaitForSingleObject((HANDLE)_thr, INFINITE) != WAIT_OBJECT_0)
		return thrd_error;

	if (GetExitCodeThread((HANDLE)_thr, &rv.winval) != 0) {
		*_rv = rv.ival;
	}

	return thrd_success;
}

int thrd_sleep(const struct timespec *time_point, struct timespec *remaining)
{
	return thrd_success;
}

void thrd_yield(void)
{
	SwitchToThread();
}

typedef struct {
	HANDLE event;
	LONG lock_idx;
	LONG recursive_count;
	int type;
} _mtx_internal_t;

int mtx_init(mtx_t *_mtx, int _type)
{
	int rv = thrd_success;
	_mtx_internal_t *mtx = NULL;

	if (!_mtx)
		return thrd_error;

	/* mtx_plain and mtx_timed are mutually exclusive options, and exactly one
	* of the two must be set.
	*/
	if (!((_type & mtx_plain) ^ (_type & mtx_timed)))
		return thrd_error;

	mtx = malloc(sizeof(_mtx_internal_t));
	if (!mtx) {
		rv = thrd_nomem;
		goto error0;
	}

	mtx->event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!mtx->event) {
		rv = thrd_nomem;
		goto error1;
	}

	mtx->type = _type;
	mtx->lock_idx = 0;

	*_mtx = (mtx_t)mtx;
	mtx = NULL;

error1:
	free(mtx);
error0:
	return rv;
}

int mtx_lock(mtx_t *_mtx)
{
	int rv = thrd_success;
	_mtx_internal_t *mtx = (_mtx_internal_t *)*_mtx;

	if (mtx->type == mtx_plain) {
		if (InterlockedExchange(&mtx->lock_idx, 1) != 0) {
			while (InterlockedExchange(&mtx->lock_idx, -1) != 0) {
				if (WaitForSingleObject(mtx->event, INFINITE) != WAIT_OBJECT_0) {
					rv = thrd_busy;
					break;
				}
			}
		}
	} else if (mtx->type == mtx_recursive) {
		rv = thrd_error; // Unimplemented
	} else {
		rv = thrd_error; // Unimplemented
	}
	return rv;
}

int mtx_timedlock(mtx_t *__restrict, const struct timespec *__restrict);
int mtx_trylock(mtx_t *);

int mtx_unlock(mtx_t *_mtx)
{
	int rv = thrd_success;
	_mtx_internal_t *mtx = (_mtx_internal_t *)*_mtx;

	if (mtx->type == mtx_plain) {
		LONG idx;

		idx = InterlockedExchange(&mtx->lock_idx, 0);
		if (idx < 0) {
			/* Someone is waiting. */
			if (SetEvent(mtx->event) == 0) {
				rv = thrd_error;
			}
		} else if (idx == 0) {
			rv = thrd_error;
		}
	}

	return rv;
}

void mtx_destroy(mtx_t *_mtx)
{
	_mtx_internal_t *mtx = (_mtx_internal_t *)*_mtx;

	CloseHandle(mtx->event);
	free(mtx);
}

enum {
	SIGNAL = 0,
	BROADCAST = 1,
	MAX_EVENTS = 2
};

typedef struct {
	HANDLE events[MAX_EVENTS];
} _cnd_internal_t;

int cnd_init(cnd_t *_cnd)
{
	int rv = thrd_success;
	_cnd_internal_t *cnd = NULL;

	if (!_cnd)
		return thrd_error;

	cnd = malloc(sizeof(_cnd_internal_t));
	if (!cnd) {
		rv = thrd_nomem;
		goto error0;
	}

	cnd->events[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!cnd->events[0]) {
		rv = thrd_nomem;
		goto error1;
	}

	cnd->events[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!cnd->events[1]) {
		rv = thrd_nomem;
		goto error2;
	}

	*_cnd = (cnd_t)cnd;
	return rv;

//error3:
//	CloseHandle(cnd->events[1]);
error2:
	CloseHandle(cnd->events[0]);
error1:
	free(cnd);
error0:
	return rv;
}

int cnd_signal(cnd_t *_cnd)
{
	_cnd_internal_t *cnd = (_cnd_internal_t *)*_cnd;

	if (PulseEvent(cnd->events[SIGNAL]) == 0)
		return thrd_error;

	return thrd_success;
}

int cnd_broadcast(cnd_t *_cnd)
{
	_cnd_internal_t *cnd = (_cnd_internal_t *)*_cnd;

	if (PulseEvent(cnd->events[BROADCAST]) == 0)
		return thrd_error;

	return thrd_success;
}

int cnd_wait(cnd_t *_cnd, mtx_t *_mtx)
{
	int rv = thrd_success;
	_cnd_internal_t *cnd = (_cnd_internal_t *)*_cnd;

	mtx_unlock(_mtx);
	switch (WaitForMultipleObjects(2, cnd->events, FALSE, INFINITE)) {
	case WAIT_TIMEOUT:
		rv = thrd_busy;
		break;
	case WAIT_FAILED:
		rv = thrd_error;
		break;
	}
	mtx_lock(_mtx);

	return rv;
}

int cnd_timedwait(cnd_t * __restrict, mtx_t * __restrict, const struct timespec *__restrict);

void cnd_destroy(cnd_t *_cnd)
{
	_cnd_internal_t *cnd = (_cnd_internal_t *)*_cnd;
	CloseHandle(cnd->events[SIGNAL]);
	CloseHandle(cnd->events[BROADCAST]);
	free(cnd);
}
