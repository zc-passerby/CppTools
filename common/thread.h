//
// Created by Passerby on 2019/10/23.
//

#ifndef ZCUTILS_THREAD_H
#define ZCUTILS_THREAD_H

#include "sem.h"
#include "mutex.h"

#include <pthread.h>

#include <map>
#include <string>

namespace zcUtils {
    struct ThreadHandle {
        bool is_alive;
        int exit_code;
        pthread_t thread_id;
        Semaphore sem_stop; // Signaled by stop().

        ThreadHandle() : thread_id(0), is_alive(false), exit_code(0) {}
    };

    typedef std::map <pthread_t, ThreadHandle> ThreadHandleMap;

    /*
     * Base class for threads.
     * Derived classes implement run() to contain the thread code.
     */
    class Thread {
    public:
        virtual ~Thread() {};

        // Enable auto deletion of this object when the thread exists.
        void autoDelete() { m_bAutoDelete_ = true; }

        /*
         * Brief:
         *     Create and start execution of the thread.
         *     The m_strThreadName_ will overwrite the previous one.
         * Params:
         *     thread_name - the name of the thread
         *     thread_num - the num of start threads
         *     thread_id - it's a pointer, will contain all started thread ids
         * return:
         *     will return true if all threads start successful.
         */
        bool start(const std::string &thread_name, unsigned int thread_num = 1, pthread_t *thread_id = NULL);

        /*
         * Stop the thread.
         * This will cause isStopping() to return true.
         * if thread_id is SET TO 0, will stop all threads.
         */
        void stop(pthread_t thread_id = 0);

        /*
         * Wait a maximum of ms milliseconds for the thread to terminate.
         * This can only be called from another thread context.
         * If ms value is SET TO 0, will block till thread exit.
         * If thread_id is SET TO 0, will join all the threads.
         */
        bool join(unsigned long ms, pthread_t thread_id = 0);

        bool join2(unsigned long ms, pthread_t thread_id = 0);

        /*
         * Accessor for retrieving the value returned from run().
         * If thread_id is NOT 0, will return that thread's exitcode,
         * else will return the first thread's exit code we met.
         */
        int getExitCode(pthread_t thread_id = 0);

        /*
         * judge the thread is running.
         * If thread_id is SET TO 0, will return false unless all threads stop running.
         */
        bool isRunning(pthread_t thread_id = 0);

    protected:
        /*
         * ctor.
         * It does not create or start execution of the thread,
         * this is handled by start().
         */
        Thread() {};

        // Don't need copy or assignment
        Thread(const Thread &);

        Thread &operator=(const Thread &);

        virtual int ThreadFunction();

        /*
         * Thread body.
         * This should call isStopping() periodically
         * and if it returns true, exit gracefully.
         * The return value may be accessed using getExitCode().
         */
        virtual int run() = 0;

        /*
         * Used by run() to test if the thread should exit.
         * If you'd like to know whether the thread is running, use isRunning().
         */
        bool isStopping();

    private:
        // Static entry point for the thread.
        static void *begin(void *thread);

    private:
        ThreadHandleMap m_threadHandleMap_;
        Mutex m_threadHandleMapMutex_;
        bool m_bAutoDelete_;
        std::string m_strThreadName_;
    };
}

#endif //ZCUTILS_THREAD_H
