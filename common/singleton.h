#ifndef __ZC_SINGLETON_H__
#define __ZC_SINGLETON_H__

#include "mutex.h"

/*
 * Brief: Ensure a class only has one instance, and provide a global point of access to it
 *
 * - This will provide a simple, fixed policy implementation.
 * - Objects are created on the heap and exists for the process lifetime.
 * - The class is thread safe, although this does incur the cost of acquiring a lock of each call.
 * - It would be nice to specify a policy for this so locking could be avoided in cases where all
 *   singleton objects get created at startup from a single main thread.
 * - Don't bother with double checked locking, you're on a road to no where!
 *
 * Usage: Singleton<myClass>::instance().myFunction();
 */
template <class T>
class Singleton {
public:
    static T &instance() {
        zcUtils::MutexLock lock(ms_cGuard_);

        if (NULL == ms_ptInstance_) {
            if (ms_bDestroyed_)
                throw std::logic_error("Singleton already destroyed!");
            ms_ptInstance_ = new T();
            // decide not to call the destructor of class, because it exists the whole process lifetime.
            // std::atexit(destroy);
        }
        return *ms_ptInstance_;
    }

private:
    static void destroy() {
        zcUtils::MutexLock lock(ms_cGuard_);
        if (NULL != ms_ptInstance_) {
            fprintf(stderr, "Singleton destroyed, funName:%s\n", __PRETTY_FUNCTION__);
            delete ms_ptInstance_;
            ms_ptInstance_ = NULL;
            ms_bDestroyed_ = true;
        }
    }

private:
    Singleton();

    ~Singleton();

    Singleton(Singleton const &);

    Singleton &operator=(Singleton const &);

private:
    static T *ms_ptInstance_;
    static bool ms_bDestroyed_;
    static zcUtils::Mutex ms_cGuard_;
};

/*
 * initialize all Singleton object params
 */
template <typename T>
T *Singleton<T>::ms_ptInstance_ = NULL;
template <typename T>
bool Singleton<T>::ms_bDestroyed_ = false;
template <typename T>
zcUtils::Mutex Singleton<T>::ms_cGuard_;

#endif // __ZC_SINGLETON_H__