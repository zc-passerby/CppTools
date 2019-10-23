//
// Created by Passerby on 2019/10/22.
//

#ifndef CPPTOOLS_FIFO_H
#define CPPTOOLS_FIFO_H

#include "sem.h"
#include "mutex.h"

#include <queue.h>

namespace zcUtils {
    class GenericFifo
    {
    protected:
        GenericFifo() {}
        ~GenericFifo() {}

        void *get(unsigned long ms) {
            void *data = 0;
        }
    };
}

#endif //CPPTOOLS_FIFO_H
