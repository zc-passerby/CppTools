//
// Created by Passerby on 2019/10/22.
//

#ifndef ZCUTILS_MUTEX_H
#define ZCUTILS_MUTEX_H

#include <pthread.h>

#include <stdexcept>

namespace zcUtils {
    /*
     * note: 1. presume the thread has own the read lock
     *          - it will success to obtain the read lock again
     *          - if it try to obtain write lock again, it will cause deadlock
     *       2. presume the thread has own the write lock, it would fail to obtain read lock or write lock
     */
    class Mutex {
    public:
        /*
         * Constructor
         */
        Mutex() {
            pthread_rwlock_init(&m_szMutex_, NULL);
        }

        /*
         * Destructor
         */
        ~Mutex() {
            lock();
            pthread_rwlock_destroy(&m_szMutex_);
        }

        /*
         * Obtain ownership of this mutex(write lock).
         * if the mutex is unavailable, block until it becomes available
         *
         * note: if the thread has already own this lock and try to lock it again,
         *       this function will return false immediately instead of block
         */
        bool lock() {
            int i = -1;
            i = pthread_rwlock_wrlock(&m_szMutex_);
            return (0 == i);
        }

        /*
         * Release ownership of this mutex.
         */
        void unlock() {
            pthread_rwlock_unlock(&m_szMutex_);
        }

        /*
         * Non-blocking attempt to obtain ownership of this mutex.
         *
         * return true if successful, otherwise return false
         */
        bool tryLock() {
            int i = -1;
            i = pthread_rwlock_trywrlock(&m_szMutex_);
            return (0 == i);
        }

        /*
         * read lock
         * will block if other thread own the write lock
         * will success if no one owns it or other thread own the read lock
         */
        bool Rlock() {
            int i = -1;
            i = pthread_rwlock_rdlock(&m_szMutex_);
            return (0 == i);
        }

        /*
         * Non-blocking of read lock
         */
        bool tryRlock() {
            int i = -1;
            i = pthread_rwlock_tryrdlock(&m_szMutex_);
            return (0 == i);
        }

    private:
        pthread_rwlock_t m_szMutex_;
    };

    /*
     * Provides automatic write lock
     */
    class MutexLock {
    public:
        /*
         * Constructor
         * Lock the mutex object, block if it's not available
         */
        MutexLock(Mutex &mutex) : m_cMutex_(mutex) {
            if (!m_cMutex_.lock())
                throw std::logic_error("lock failed!");
        }

        /*
         * Destructor
         * Unlock the mutex object.
         */
        ~MutexLock() {
            m_cMutex_.unlock();
        }

    private:
        Mutex &m_cMutex_;
    };

    /*
     * Provides automatic read lock
     */
    class MutexReadLock {
    public:
        /*
         * Constructor
         * Lock the mutex object, block if it's not available
         */
        MutexReadLock(Mutex &mutex) : m_cMutex_(mutex) {
            if (!m_cMutex_.Rlock())
                throw std::logic_error("Rlock failed!");
        }

        /*
         * Destructor
         * Unlock the mutex object.
         */
        ~MutexReadLock() {
            m_cMutex_.unlock();
        }

    private:
        Mutex &m_cMutex_;
    };

};

#endif // ZCUTILS_MUTEX_H