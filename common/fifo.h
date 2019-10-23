//
// Created by Passerby on 2019/10/22.
//

#ifndef ZCUTILS_FIFO_H
#define ZCUTILS_FIFO_H

#include "sem.h"
#include "mutex.h"

#include <queue.h>

namespace zcUtils {
    /*
     * A thread-safe First-In-First-Out Queue supporting blocking dqueue operations.
     * To ensure type safety, this class cannot be instantiated directly,
     * but instead, must be accessed using objects derived from the template class Fifo.
     */
    class GenericFifo {
    protected:
        // ctor. create an empty queue.
        GenericFifo() {}

        // dtor. this does NOT delete any items in this queue!
        ~GenericFifo() {}

        /*
         * Remove the item at the head of the queue.
         * params:
         *     ms - max wait time in milliseconds
         *returns:
         *     a pointer to an object or NULL
         */
        void *get(unsigned long ms) {
            void *data = NULL;

            if (m_cSemItemInQueue_.tryWait(ms)) {
                MutexLock lock(m_cMutex_);

                if (!m_Queue_.empty()) {
                    data = m_Queue_.front();
                    m_Queue_.pop();
                }
            }
            return data;
        }

        // Get current size of the queue
        unsigned int size() {
            unsigned int ret = 0;
            MutexLock lock(m_cMutex_);
            ret = m_Queue_.size();
            return ret;
        }

        // Add an item to the end of the queue.
        bool put(void *data) {
            {
                MutexLock lock(m_cMutex_);
                m_Queue_.push(data);
            }
            m_cSemItemInQueue_.post();
            return true;
        }

    private:
        // Disable copy and assignment.
        GenericFifo(const GenericFifo &);

        GenericFifo &operator=(const GenericFifo &);

    private:
        Semaphore m_cSemItemInQueue_;
        Mutex m_cMutex_;
        std::queue<void *> m_Queue_;
    };

    // this template provides a type-safe interface to the GenericFifo class.
    template<class T>
    class Fifo : private GenericFifo {
    public:
        /*
         * Remove the item at the head of the queue.
         * params:
         *     ms - max wait time in milliseconds
         * returns:
         *     a pointer to an object or NULL
         */
        T *get(unsigned long ms) {
            return static_cast<T *>(GenericFifo::get(ms));
        }

        // return the size of the queue.
        unsigned int size() {
            return GenericFifo::size();
        }

        // Add an item to the end of the queue.
        bool put(T *object) {
            return GenericFifo::put(object);
        }
    };
}

#endif //ZCUTILS_FIFO_H
