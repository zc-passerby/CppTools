//
// Created by Passerby on 2019/10/23.
//

#ifndef ZCUTILS_QUEUE_THREAD_H
#define ZCUTILS_QUEUE_THREAD_H

#include "fifo.h"
#include "thread.h"

namespace zcUtils {
    /*
     * Extends the Thread class to provide a work queue.
     * The work queue stores pointers in a First-In-First-Out queue.
     */
    template<class T>
    class QueueThread : public Thread {
    public:
        QueueThread() {}

        virtual ~QueueThread() {}

        // Append an item to the tail of the queue.
        void put(T *t) { m_FifoQueue_.put(t); }

    protected:
        /*
         * Brief:
         *     Retrieve the item at the end of the queue with a timeout.
         * Params:
         *     ms - maximum time in milliseconds to wait for an item to become available
         * return:
         *     the item from the head of the queue
         *     or NULL if no item was available within the timeout period.
         */
        T *get(unsigned long ms) { return m_FifoQueue_.get(ms); }

        unsigned int size() { return m_FifoQueue_.size(); }

    private:
        Fifo<T> m_FifoQueue_;
    };
}

#endif //ZCUTILS_QUEUE_THREAD_H
