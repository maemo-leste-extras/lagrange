/** @file c11threads.c  pthread-based implementation of C11 threads.

@authors Copyright (c) 2018 Jaakko Keränen <jaakko.keranen@iki.fi>
Original version by: John Tsiombikas <nuclear@member.fsf.org>

@par License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "the_Foundation/c11threads.h"
#include <errno.h>
#include <pthread.h>
#include <sched.h> /* for sched_yield */
#include <sys/time.h>
#include <time.h>

#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT

#ifdef C11THREADS_NO_TIMED_MUTEX
#  define PTHREAD_MUTEX_TIMED_NP PTHREAD_MUTEX_NORMAL
#  define C11THREADS_TIMEDLOCK_POLL_INTERVAL 5000000 /* 5 ms */
#endif

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
    int res = pthread_create(thr, 0, (void *(*)(void *))func, arg);
    if (res == 0) {
        return thrd_success;
    }
    return res == ENOMEM ? thrd_nomem : thrd_error;
}

void thrd_exit(int res) { pthread_exit((void *)(long)res); }

int thrd_join(thrd_t thr, int *res) {
    void *retval;
    if (pthread_join(thr, &retval) != 0) {
        return thrd_error;
    }
    if (res) {
        *res = (int) ((intptr_t) retval);
    }
    return thrd_success;
}

int thrd_detach(thrd_t thr) {
    return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
}

thrd_t thrd_current(void) { return pthread_self(); }

int thrd_equal(thrd_t a, thrd_t b) { return pthread_equal(a, b); }

void thrd_sleep(const struct timespec *ts_in, struct timespec *rem_out) {
    int res;
    struct timespec rem, ts = *ts_in;
    do {
        res = nanosleep(&ts, &rem);
        ts = rem;
    } while (res == -1 && errno == EINTR);
    if (rem_out) {
        *rem_out = rem;
    }
}

void thrd_yield(void) { sched_yield(); }

int mtx_init(mtx_t *mtx, int type) {
    int res;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (type & mtx_timed) {
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);
    }
    if (type & mtx_recursive) {
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    }
    res = pthread_mutex_init(mtx, &attr) == 0 ? thrd_success : thrd_error;
    pthread_mutexattr_destroy(&attr);
    return res;
}

void mtx_destroy(mtx_t *mtx) { pthread_mutex_destroy(mtx); }

int mtx_lock(mtx_t *mtx) {
    int res = pthread_mutex_lock(mtx);
    if (res == EDEADLK) {
        return thrd_busy;
    }
    return res == 0 ? thrd_success : thrd_error;
}

int mtx_trylock(mtx_t *mtx) {
    int res = pthread_mutex_trylock(mtx);
    if (res == EBUSY) {
        return thrd_busy;
    }
    return res == 0 ? thrd_success : thrd_error;
}

int mtx_timedlock(mtx_t *mtx, const struct timespec *ts) {
    int res;
#ifdef C11THREADS_NO_TIMED_MUTEX
    /* fake a timedlock by polling trylock in a loop and waiting for a bit */
    struct timeval now;
    struct timespec sleeptime;
    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = C11THREADS_TIMEDLOCK_POLL_INTERVAL;
    while ((res = pthread_mutex_trylock(mtx)) == EBUSY) {
        gettimeofday(&now, NULL);
        if (now.tv_sec > ts->tv_sec ||
            (now.tv_sec == ts->tv_sec && (now.tv_usec * 1000) >= ts->tv_nsec)) {
            return thrd_timedout;
        }
        nanosleep(&sleeptime, NULL);
    }
#else
    if ((res = pthread_mutex_timedlock(mtx, ts)) == ETIMEDOUT) {
        return thrd_timedout;
    }
#endif
    return res == 0 ? thrd_success : thrd_error;
}

int mtx_unlock(mtx_t *mtx) {
    return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;
}

int cnd_init(cnd_t *cond) {
    return pthread_cond_init(cond, 0) == 0 ? thrd_success : thrd_error;
}

void cnd_destroy(cnd_t *cond) { pthread_cond_destroy(cond); }

int cnd_signal(cnd_t *cond) {
    return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
}

int cnd_broadcast(cnd_t *cond) {
    return pthread_cond_broadcast(cond) == 0 ? thrd_success : thrd_error;
}

int cnd_wait(cnd_t *cond, mtx_t *mtx) {
    return pthread_cond_wait(cond, mtx) == 0 ? thrd_success : thrd_error;
}

int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts) {
    int res;
    if ((res = pthread_cond_timedwait(cond, mtx, ts)) != 0) {
        return res == ETIMEDOUT ? thrd_timedout : thrd_error;
    }
    return thrd_success;
}

int tss_create(tss_t *key, tss_dtor_t dtor) {
    return pthread_key_create(key, dtor) == 0 ? thrd_success : thrd_error;
}

void tss_delete(tss_t key) { pthread_key_delete(key); }

int tss_set(tss_t key, void *val) {
    return pthread_setspecific(key, val) == 0 ? thrd_success : thrd_error;
}

void *tss_get(tss_t key) { return pthread_getspecific(key); }

void call_once(once_flag *flag, void (*func)(void)) {
    pthread_once(flag, func);
}
