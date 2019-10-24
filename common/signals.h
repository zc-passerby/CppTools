//
// Created by Passerby on 2019/10/24.
//

#ifndef ZCUTILS_SIGNALS_H
#define ZCUTILS_SIGNALS_H

#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>

namespace zcUtils {
    // Base class for signal handlers
    class SignalHandler {
    public:
        virtual ~SignalHandler() {}

        // Handler for signal 'signum'
        virtual int handleSignal(int signal_num) = 0;
    };

    // Fake signal functions.
#define SIG_ERROR_HANDLER ((SignalHandler *)SIG_ERR) // Error return
#define SIG_DEFAULT_HANDLER ((SignalHandler *)SIG_DFL) // Default action
#define SIG_IGNORE_HANDLER ((SignalHandler *)SIG_IGN) // Ignore signal

    // Signal manager (singleton)
    class Signal {
    public:
        static const int INFINITE_TIMEOUT = 0;

        // Get the one and only instance of this class.
        static Signal *instance();

        /*
         * Brief:
         *     Setup the handler for 'signum'
         * Params:
         *     signal_num - signal number
         *     signal_handler - signal handler to invoke
         *     synchronous - if set to true signal will be invoke after WaitSig function
         *                   otherwise signal will be handled asynchronously
         * Return:
         *     pointer to the previously set signal handler
         */
        SignalHandler *setHandler(int signal_num, SignalHandler *signal_handler, bool synchronous = false);

        /*
         * Brief:
         *     Function implement synchronous signal handling.
         *     Function will block until one of the signal defined in setHandler function will be received,
         *     or timeout will occur.
         * Params:
         *     timeout - number of seconds to wait for signal.
         *               Setting timeout value to Signal::INFINITE_TIMEOUT will cause will not return
         *               until one of the requested signals will be received.
         * Return:
         *     signal number or EAGAIN
         */
        int waitSignal(int timeout);

    private:
        Signal() {}

        ~Signal() {}

        Signal(Signal const &);

        Signal &operator=(Signal const &);

        static void dispatch(int signal_number);

    private:
        static SignalHandler *m_pSignalHandlers_[NSIG];
        static SignalHandler *m_pSyncSignalHandlers_[NSIG];
    };

    /*
     * Handler for SIGHUP (hangup) signals.
     * This is intended to trigger reloading of the application's configuration.
     */
    class SighupHandler : public SignalHandler {
    public:
        SighupHandler() : m_stSignalFlag_(false) {}

        // Set the flag and return immediately
        virtual int handleSignal(int /*signum*/) {
            m_stSignalFlag_ = true;
            return 0;
        }

        // Test and clear the flag.
        sig_atomic_t isSet() {
            sig_atomic_t flag = m_stSignalFlag_;
            m_stSignalFlag_ = false;
            return flag;
        }

    private:
        sig_atomic_t m_stSignalFlag_;
    };

    /*
     * Handler for SIGQUIT (ctrl \) signals.
     * This is intended to trigger immediately termination with core dump.
     */
    class SigquitHandler : public SignalHandler {
    public:
        SigquitHandler() : m_stSignalFlag_(false) {}

        // Set the flag and return immediately
        virtual int handleSignal(int /*signum*/) {
            m_stSignalFlag_ = true;
            return 0;
        }

        // Test the flag.
        sig_atomic_t isSet() { return m_stSignalFlag_; }

    private:
        sig_atomic_t m_stSignalFlag_;
    };

    /*
     * Handler for any signals that should trigger a graceful shutdown of the application.
     */
    class SigexitHandler : public SignalHandler {
    public:
        SigexitHandler() : m_stSignalFlag_(false) {}

        // Set the flag and return immediately
        virtual int handleSignal(int /*signum*/) {
            m_stSignalFlag_ = true;
            return 0;
        }

        // Test the flag.
        sig_atomic_t isSet() { return m_stSignalFlag_; }

    private:
        sig_atomic_t m_stSignalFlag_;
    };

    /*
     * Handler for SIGUSR1 signals.
     * Application specific.
     */
    class Sigusr1Handler : public SignalHandler {
    public:
        Sigusr1Handler() : m_stSignalFlag_(false) {}

        // Set the flag and return immediately
        virtual int handleSignal(int /*signum*/) {
            m_stSignalFlag_ = true;
            return 0;
        }

        // Test and clear the flag.
        sig_atomic_t isSet() {
            sig_atomic_t flag = m_stSignalFlag_;
            m_stSignalFlag_ = false;
            return flag;
        }

    private:
        sig_atomic_t m_stSignalFlag_;
    };
}

#endif //ZCUTILS_SIGNALS_H
