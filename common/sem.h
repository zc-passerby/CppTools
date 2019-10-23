//
// Created by Passerby on 2019/10/22.
//

#ifndef CPPTOOLS_SEM_H
#define CPPTOOLS_SEM_H

#include <sys/time.h>
#include <semaphore.h>

namespace zcUtils {
    class Semaphore {
    public:
        Semaphore(unsigned int count = 0) {
            sem_init(&m_stSem, 0, count);
        }

        ~Semaphore() {
            sem_destroy(&m_stSem);
        }

        void post() {
            sem_post(&m_stSem);
        }

        void wait() {
            sem_wait(&m_stSem);
        }

        int getCount() {
            int ret = 0;
            sem_getvalue(&m_stSem, &ret);
            return ret;
        }

        bool tryWait(unsigned long ms = 0UL) {
            struct timeval now;
            gettimeofday(&now, 0);
            struct timespec abstime;
            abstime.tv_sec = now.tv_sec;
            abstime.tv_nsec = now.tv_usec * 1000;
            abstime.tv_sec += ms / 1000;
            abstime.tv_nsec += ms % 1000 * 1000000L;
            if (abstime.tv_nsec >= 1000000000L) {
                abstime.tv_nsec -= 1000000000L;
                abstime.tv_sec++;
            }
            return (0 == sem_timedwait(&sem_, &abstime));
        }

    private:
        sem_t m_stSem;
    };
}

#endif //CPPTOOLS_SEM_H
