//
// Created by Passerby on 2019/10/24.
//

#ifndef ZCUTILS_DAEMON_H
#define ZCUTILS_DAEMON_H

#include "options.h"
#include "filelock.h"
#include "application.h"

using std::string;

namespace zcUtils {
    class Daemon : protected Application {
    public:
        static const int MAX_INIT_TIMEOUT;

        enum DAEMON_ERROR_CODES {
            ERR_SUCCESS_E = 0,                 // Daemon started successfully
            ERR_PERSONALISATION_FAILED_E = 10, // failed to change process credentials
            ERR_ALREADY_RUNNING_E,             // copy of process already running (checking pid file)
            ERR_INIT_FAILED_E,                 // unknown failure during daemon initialisation
            ERR_CHDIR_FAILED_E,                // failed to change process working directory
            ERR_FORK_FAILED_E,                 // failed to run daemon thread
            ERR_SID_FAILED_E,                  // changing session failed
            ERR_CREATE_LOCK_FAILED_E,          // failed to create or lock pid file
            ERR_INITIALISE_FAILED_E = 100      // Demon-specific initialisation failed (start method reported errors)
        };

        Daemon(const string &name, const string &description, const string &version,
               const string &build_time, const string &working_dir, const string &lock_file);

        ~Daemon();

        virtual bool run(int argc, char *argv[]);

        virtual bool shutdown();

        virtual bool reload();

        virtual bool addCustomOption(char short_name, const string &long_name,
                                     const string &description, bool value_option);

        const char *nameOf(DAEMON_ERROR_CODES code) const;

        DAEMON_ERROR_CODES getLastError() { return m_nErrorCode_; }

        // for Options function and settings, just for expanding
        void setConfigFile(const string filename);

        void addSwitchOption(char short_name, const string long_name, const string description);

        void addValueOption(char short_name, const string long_name, const string description);

    protected:
        virtual int waitForShutdown();

        bool printInfo();

        bool personalize();

        void setupLog();

        void setupSignals();

        bool setPwd();

        bool daemonize();

        void setupTracing();

    protected:
        Options m_cOptions_;
        pid_t m_nStarterPid_;
        FileLock *m_pFileLock_;
        string m_strWorkingDir_;
        DAEMON_ERROR_CODES m_nErrorCode_;
    };
}

#endif //ZCUTILS_DAEMON_H
