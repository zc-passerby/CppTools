//
// Created by Passerby on 2019/10/22.
//

#ifndef ZCUTILS_SEM_H
#define ZCUTILS_SEM_H

#include <sys/time.h>
#include <semaphore.h>

namespace zcUtils {
    /*
     * class Semaphore
     * Wrapper facade for system semaphores.
     */
    class Semaphore {
    public:
        /*
         * ctor. Create a semaphore.
         *
         * Param count - initial value
         */
        Semaphore(unsigned int count = 0) {
            sem_init(&m_stSem_, 0, count);
        }

        // dtor.
        ~Semaphore() {
            sem_destroy(&m_stSem_);
        }

        // Signal this semaphore
        void post() {
            sem_post(&m_stSem_);
        }

        /*
         * If this semaphore is unavailable (it's count is zero)
         * block until it's available.
         */
        void wait() {
            sem_wait(&m_stSem_);
        }

        /*
         * If this semaphore is unavailable (it's count is zero)
         * block until it's available or the period ms milliseconds has elapsed.
         *
         * param ms - maximum wait time in milliseconds
         */
        bool tryWait(unsigned long ms = 0UL) {
            struct timeval now;
            gettimeofday(&now, 0);
            struct timespec absTime;
            absTime.tv_sec = now.tv_sec;
            absTime.tv_nsec = now.tv_usec * 1000;
            absTime.tv_sec += ms / 1000;
            absTime.tv_nsec += ms % 1000 * 1000000L;
            if (absTime.tv_nsec >= 1000000000L) {
                absTime.tv_nsec -= 1000000000L;
                absTime.tv_sec++;
            }
            return (0 == sem_timedwait(&m_stSem_, &abstime));
        }

        /*
         * Return number of free slots on semaphore.
         * e.g.
         *     if semaphore is created with count 20, getCount will return 20
         *     when some thread locks the semaphore(e.g. call wait), getCount will return 19
         */
        int getCount() {
            int ret = 0;
            sem_getvalue(&m_stSem_, &ret);
            return ret;
        }

    private:
        sem_t m_stSem_;
    };
}

#endif //ZCUTILS_SEM_H
