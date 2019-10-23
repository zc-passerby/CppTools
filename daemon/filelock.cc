//
// Created by Passerby on 2019/10/23.
//

#include "filelock.h"

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>

namespace zcUtils {
    FileLock::FileLock(string filename)
            : m_strFileName_(filename), m_nFd_(0) {}

    FileLock::~FileLock() {
        if (!isLocked()) {
            if (m_nFd_)
                close(m_nFd_);
            unlink(m_strFileName_.c_str()); // try to remove it
        } else {
            if (m_nFd_ > 0) {
                close(m_nFd_);
                m_nFd_ = 0;
            }
        }
    }

    bool FileLock::isLocked() {
        bool locked = false;
        if (openFile())
            if (lockf(m_nFd_, F_TEST, 0) != 0)
                locked = true;
        return locked;
    }

    bool FileLock::lock() {
        bool ret = false;
        if (openFile()) {
            ret = (lockf(m_nFd_, F_LOCK, 0) == 0);
            if (ret) {
                char str[12];
                sprintf(str, "%d", getpid());
                ftruncate(m_nFd_, 0);
                lseek(m_nFd_, 0, SEEK_SET);
                write(m_nFd_, str, strlen(str));
            }
        }
        return ret;
    }

    bool FileLock::tryLock() {
        bool ret = false;
        if (openFile()) {
            ret = (lockf(m_nFd_, F_TLOCK, 0) == 0);
            if (ret) {
                char str[12];
                sprintf(str, "%d", getpid());
                ftruncate(m_nFd_, 0);
                lseek(m_nFd_, 0, SEEK_SET);
                write(m_nFd_, str, strlen(str));
            }
        }
        return ret;
    }

    bool FileLock::unlock() {
        bool ret = false;
        if (openFile()) {
            if (lockf(m_nFd_, F_ULOCK, 0) == 0) {
                ftruncate(m_nFd_, 0);
                close(m_nFd_);
                m_nFd_ = 0;
                ret = true;
            }
        }
        return ret;
    }

    bool FileLock::openFile() {
        bool ret = false;
        if (m_nFd_ <= 0)
            m_nFd_ = open(m_strFileName_.c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
        ret = (m_nFd_ > 0);
        return ret;
    }
}