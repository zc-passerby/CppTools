//
// Created by Passerby on 2019/10/23.
//

#include "thread.h"

#include <errno.h>
#include <string.h>

namespace zcUtils {
    /*
     * This function provides a static entry point for the OS thread creation API.
     * It calls the run() method and collects its exit code.
     */
    void Thread::begin(void *thread) {
        Thread *pThread = static_cast<Thread *>thread;
        pThread->ThreadFunction();
    }

    // non-blocking test for a stop event.
    bool Thread::isStopping() {
        pthread_t thread_id = pthread_self();
        MutexReadLock lock(m_threadHandleMapMutex_);
        ThreadHandleMap::iterator it = m_threadHandleMap_.find(thread_id);
        if (m_threadHandleMap_.end() == it)
            return false;
        ThreadHandle &thread_handle = it->second;
        return thread_handle.sem_stop.tryWait(0);
    }

    int Thread::ThreadFunction() {
        int exit_code = 0;
        // this ensure we have added the pair to map before we run the thread.
        m_threadHandleMapMutex_.lock();
        m_threadHandleMapMutex_.unlock();
        exit_code = run();
        pthread_t thread_id = pthread_self();
        MutexReadLock lock(m_threadHandleMapMutex_);
        ThreadHandleMap::iterator it = m_threadHandleMap_.find(thread_id);
        if (m_threadHandleMap_.end() == it)
            return -1;
        ThreadHandle &thread_handle = it->second;
        thread_handle.exit_code = exit_code;
        thread_handle.is_alive = false;
        return exit_code;
    }

    // Create the thread and start running.
    bool Thread::start(const std::string &thread_name, unsigned int thread_num, pthread_t *thread_id) {
        m_strThreadName_ = thread_name;
        for (int i = 0; i < thread_num; ++i) {
            ThreadHandle thread_handle;
            MutexLock lock(m_threadHandleMapMutex_);
            if (0 == pthread_create(&thread_handle.thread_id, 0, begin, (void *) this)) {
                thread_handle.is_alive = true;
                m_threadHandleMap_.insert(std::pair<pthread_t, ThreadHandle>(thread_handle.thread_id, thread_handle));
                if (thread_id)
                    thread_id[i] = thread_handle.thread_id;
            } else
                return false;
        }
        return true;
    }

    // Signal the stop event. run() should be checking for the event.
    void Thread::stop(pthread_t thread_id) {
        if (0 != thread_id) {
            MutexReadLock lock(m_threadHandleMapMutex_);
            ThreadHandleMap::iterator it = m_threadHandleMap_.find(thread_id);
            if (m_threadHandleMap_.end() == it)
                return;
            ThreadHandle &thread_handle = it->second;
            if (thread_handle.is_alive)
                thread_handle.sem_stop.post();
        } else {
            MutexReadLock lock(m_threadHandleMapMutex_);
            for (ThreadHandleMap::iterator it = m_threadHandleMap_.begin(); m_threadHandleMap_.end() != it; ++it) {
                ThreadHandle &thread_handle = it->second;
                if (thread_handle.is_alive)
                    thread_handle.sem_stop.post();
            }
        }
        return;
    }

    bool Thread::join(unsigned long ms, pthread_t thread_id) {
        bool join_success = true;
        bool is_failed = false;
        if (0 != thread_id) {
            m_threadHandleMapMutex_.Rlock();
            ThreadHandleMap::itearator it = m_threadHandleMap_.find(thread_id);
            if (m_threadHandleMap_.end() == it) {
                m_threadHandleMapMutex_.unlock();
                return false;
            }
            ThreadHandle &thread_handle = it->second;
            m_threadHandleMapMutex_.unlock();
            pthread_cancel(thread_handle.thread_id);
            join_success = (pthread_join(thread_handle.thread_id, NULL) == 0);
            if (join_success) {
                m_threadHandleMapMutex_.lock();
                m_threadHandleMap_.erase(it);
                m_threadHandleMapMutex_.unlock();
            }
        } else {
            // TODO(Passerby): the following code is not safe, because m_threadHandleMap_ is not locked all the time
            for (ThreadHandleMap::iterator it = m_threadHandleMap_.begin(); m_threadHandleMap_.end() != it;) {
                ThreadHandle &thread_handle = it->second;
                pthread_cancel(thread_handle.thread_id);
                join_success = (pthread_join(thread_handle.thread_id, NULL) == 0);
                if (join_success) {
                    m_threadHandleMapMutex_.lock();
                    m_threadHandleMap_.erase(it++);
                    m_threadHandleMapMutex_.unlock();
                } else {
                    is_failed = true;
                    ++it;
                }
            }
            if (is_failed)
                join_success = false;
        }
        return join_success;
    }

    bool Thread::join2(unsigned long ms, pthread_t thread_id) {
        bool join_success = true;
        bool is_failed = false;
        if (0 != thread_id) {
            m_threadHandleMapMutex_.Rlock();
            ThreadHandleMap::itearator it = m_threadHandleMap_.find(thread_id);
            if (m_threadHandleMap_.end() == it) {
                m_threadHandleMapMutex_.unlock();
                return false;
            }
            ThreadHandle &thread_handle = it->second;
            m_threadHandleMapMutex_.unlock();
//            pthread_cancel(thread_handle.thread_id);
            join_success = (pthread_join(thread_handle.thread_id, NULL) == 0);
            if (join_success) {
                m_threadHandleMapMutex_.lock();
                m_threadHandleMap_.erase(it);
                m_threadHandleMapMutex_.unlock();
            }
        } else {
            // TODO(Passerby): the following code is not safe, because m_threadHandleMap_ is not locked all the time
            for (ThreadHandleMap::iterator it = m_threadHandleMap_.begin(); m_threadHandleMap_.end() != it;) {
                ThreadHandle &thread_handle = it->second;
//                pthread_cancel(thread_handle.thread_id);
                join_success = (pthread_join(thread_handle.thread_id, NULL) == 0);
                if (join_success) {
                    m_threadHandleMapMutex_.lock();
                    m_threadHandleMap_.erase(it++);
                    m_threadHandleMapMutex_.unlock();
                } else {
                    is_failed = true;
                    ++it;
                }
            }
            if (is_failed)
                join_success = false;
        }
        return join_success;
    }

    int Thread::getExitCode(pthread_t thread_id) {
        int exit_code = 0;
        if (0 != thread_id) {
            MutexReadLock lock(m_threadHandleMapMutex_);
            ThreadHandleMap::iterator it = m_threadHandleMap_.find(thread_id);
            if (m_threadHandleMap_.end() == it)
                return 0;
            ThreadHandle &thread_handle = it->second;
            if (thread_handle.is_alive)
                return 0;
            exit_code = thread_handle.exit_code;
        } else {
            MutexReadLock lock(m_threadHandleMapMutex_);
            for (ThreadHandleMap::iterator it = m_threadHandleMap_.begin(); m_threadHandleMap_.end() != it; ++it) {
                ThreadHandle &thread_handle = it->second;
                if (thread_handle.is_alive)
                    continue;
                else
                    exit_code = thread_handle.exit_code;
            }
        }
        return exit_code;
    }

    bool Thread::isRunning(pthread_t thread_id) {
        if (0 != thread_id) {
            MutexReadLock lock(m_threadHandleMapMutex_);
            ThreadHandleMap::iterator it = m_threadHandleMap_.find(thread_id);
            if (m_threadHandleMap_.end() == it)
                return false;
            ThreadHandle &thread_handle = it->second;
            if (thread_handle.is_alive)
                return true;
            else
                return false;
        } else {
            MutexReadLock lock(m_threadHandleMapMutex_);
            for (ThreadHandleMap::iterator it = m_threadHandleMap_.begin(); m_threadHandleMap_.end() != it; ++it) {
                ThreadHandle &thread_handle = it->second;
                if (!thread_handle.is_alive)
                    return false;
            }
        }
        return true;
    }
}