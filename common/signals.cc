//
// Created by Passerby on 2019/10/24.
//

#include "signals.h"

#include <time.h>
#include <errno.h>

namespace zcUtils {
    SignalHandler *Signal::m_pSignalHandlers_[NSIG] = {SIG_IGNORE_HANDLER};
    SignalHandler *Signal::m_pSyncSignalHandlers_[NSIG] = {SIG_IGNORE_HANDLER};
    const int INFINITE_TIMEOUT = 0;

    // Get the single instance.
    Signal *Signal::instance() {
        static Signal instance;
        return &instance;
    }

    // Set the handler object for a signal.
    SignalHandler *Signal::setHandler(int signal_num, zcUtils::SignalHandler *signal_handler, bool synchronous) {
        SignalHandler *signalHandler = NULL;
        if (signal_num < NSIG) {
            if (synchronous) {
                signalHandler = m_pSyncSignalHandlers_[signal_num];
                // remove handler from asynchronous handler list.
                m_pSignalHandlers_[signal_num] = SIG_IGNORE_HANDLER;
                m_pSyncSignalHandlers_[signal_num] = signal_handler;

                // block signal to prevent handler asynchronous execution.
                sigset_t signal_set;
                sigemptyset(&signal_set);
                sigaddset(&signal_set, signal_num);
                sigprocmask(SIG_BLOCK, &signal_set, NULL);
            } else {
                signalHandler = m_pSignalHandlers_[signal_num];
                // remove handler from synchronous handler list.
                m_pSyncSignalHandlers_[signal_num] = SIG_IGNORE_HANDLER;
                m_pSignalHandlers_[signal_num] = signal_handler;

                // unblock signal to be executed asynchronously.
                sigset_t signal_set;
                sigemptyset(&signal_set);
                sigaddset(&signal_set, signal_num);
                sigprocmask(SIG_UNBLOCK, &signal_set, NULL);

                struct sigaction signal_action;
                signal_action.sa_handler = dispatch;
                sigemptyset(&signal_action.sa_mask);
                signal_action.sa_flags = 0;
                sigaction(signal_num, &signal_action, 0);
            }
        }
        return signalHandler;
    }

    // Wait for signal number of seconds specified in.
    int Signal::waitSignal(int timeout) {
        int signal_num;
        sigset_t signal_set;
        sigemptyset(&signal_set);
        for (int i = 0; i < NSIG; ++i) {
            if (m_pSyncSignalHandlers_[i] &&
                (m_pSyncSignalHandlers_[i] != SIG_ERROR_HANDLER) &&
                (m_pSyncSignalHandlers_[i] != SIG_DEFAULT_HANDLER) &&
                (m_pSyncSignalHandlers_[i] != SIG_IGNORE_HANDLER)) {
                sigaddset(&signal_set, i);
            }
        }
        siginfo_t signal_info;
        time_t finish_time = time(NULL) + timeout;
        // Let's prevent wait for signal to be interrupted by other signals.
        do {
            if (timeout == INFINITE_TIMEOUT)
                signal_num = sigwaitinfo(&signal_set, &signal_info);
            else {
                timespec time_spec = {finish_time - time(NULL), 0};
                /*
                 * Time elapsed, but wait was interrupted by asynchronous signal
                 * try to wait 1ns to check if signal is pending, or get timeout.
                 */
                if (time(NULL) >= finish_time) {
                    time_spec.tv_sec = 0;
                    time_spec.tv_nsec = 1;
                }
                signal_num = sigtimedwait(&signal_set, &signal_info, &time_spec);
            }
        } while (signal_num == -1 && errno == EINTR);
        // If signal raised.
        if (signal_num >= 0 && signal_num < NSIG) {
            if (m_pSyncSignalHandlers_[signal_num] &&
                (m_pSyncSignalHandlers_[signal_num] != SIG_ERROR_HANDLER) &&
                (m_pSyncSignalHandlers_[signal_num] != SIG_DEFAULT_HANDLER) &&
                (m_pSyncSignalHandlers_[signal_num] != SIG_IGNORE_HANDLER)) {
                m_pSyncSignalHandlers_[signal_num]->handleSignal(signal_num);
            }
        } else
            signal_num = errno;
        return signal_num;
    }

    // Forward the signal event to its handler object.
    void Signal::dispatch(int signal_number) {
        if (m_pSignalHandlers_[signal_number] &&
            (m_pSignalHandlers_[signal_number] != SIG_DEFAULT_HANDLER) &&
            (m_pSignalHandlers_[signal_number] != SIG_IGNORE_HANDLER) &&
            (m_pSignalHandlers_[signal_number] != SIG_ERROR_HANDLER))
        {
            m_pSignalHandlers_[signal_number]->handleSignal(signal_number);
        }
    }
}