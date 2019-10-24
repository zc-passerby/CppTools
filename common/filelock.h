//
// Created by Passerby on 2019/10/23.
//

#ifndef ZCUTILS_FILELOCK_H
#define ZCUTILS_FILELOCK_H

#include <string>

using std::string;

namespace zcUtils {
    // Implements inter-process locking mechanism based on lock file.
    class FileLock {
    public:
        FileLock(string filename);

        virtual ~FileLock();

        // Check if file is not locked by other processes.
        bool isLocked();

        /*
         * Check if file is not locked by other processes.
         *
         * If file is locked, return false immediately,
         * otherwise, lock file then return true.
         */
        bool tryLock();

        /*
         * Try to lock file.
         *
         * If file is locked, waits until file is unlocked,
         * otherwise, lock file then return true.
         * In case of errors, return false.
         */
        bool lock();

        /*
         * Unlock the file.
         *
         * Return true if file is unlock successfully,
         * otherwise, return false.
         */
        bool unlock();

    private:
        FileLock() {}

        FileLock(const FileLock &) {}

        bool openFile();

    private:
        int m_nFd_;
        string m_strFileName_;
    };
}

#endif //ZCUTILS_FILELOCK_H
