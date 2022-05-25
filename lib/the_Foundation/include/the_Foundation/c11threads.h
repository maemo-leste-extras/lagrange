/*
Author: John Tsiombikas <nuclear@member.fsf.org>

I place this piece of code in the public domain. Feel free to use as you see
fit.  I'd appreciate it if you keep my name at the top of the code somehwere,
but whatever.

Main project site: https://github.com/jtsiomb/c11threads
*/

#ifndef C11THREADS_H_
#define C11THREADS_H_

#include "defs.h"

#if defined (iHavePThread)
#  include <pthread.h>
#else
typedef void * pthread_t;
typedef void * pthread_mutex_t;
typedef void * pthread_cond_t;
typedef void * pthread_key_t;
typedef struct {
    pthread_mutex_t mutex;
    int state;
} pthread_once_t;
#endif

/* types */
typedef pthread_t thrd_t;
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_key_t tss_t;
typedef pthread_once_t once_flag;

typedef int (*thrd_start_t)(void*);
typedef void (*tss_dtor_t)(void*);

enum {
	mtx_plain		= 0,
	mtx_recursive	= 1,
	mtx_timed		= 2,
};

enum {
	thrd_success,
	thrd_timedout,
	thrd_busy,
	thrd_error,
	thrd_nomem
};

iBeginPublic

/* ---- thread management ---- */

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
void thrd_exit(int res);
int thrd_join(thrd_t thr, int *res);
int thrd_detach(thrd_t thr);
thrd_t thrd_current(void);
int thrd_equal(thrd_t a, thrd_t b);
void thrd_sleep(const struct timespec *ts_in, struct timespec *rem_out);
void thrd_yield(void);

/* ---- mutexes ---- */

int mtx_init(mtx_t *mtx, int type);
void mtx_destroy(mtx_t *mtx);
int mtx_lock(mtx_t *mtx);
int mtx_trylock(mtx_t *mtx);
int mtx_timedlock(mtx_t *mtx, const struct timespec *ts);
int mtx_unlock(mtx_t *mtx);

/* ---- condition variables ---- */

int cnd_init(cnd_t *cond);
void cnd_destroy(cnd_t *cond);
int cnd_signal(cnd_t *cond);
int cnd_broadcast(cnd_t *cond);
int cnd_wait(cnd_t *cond, mtx_t *mtx);
int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts);

/* ---- thread-specific data ---- */

int tss_create(tss_t *key, tss_dtor_t dtor);
void tss_delete(tss_t key);
int tss_set(tss_t key, void *val);
void *tss_get(tss_t key);

/* ---- misc ---- */

void call_once(once_flag *flag, void (*func)(void));

iEndPublic

#endif	/* C11THREADS_H_ */
