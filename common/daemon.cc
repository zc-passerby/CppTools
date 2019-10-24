//
// Created by Passerby on 2019/10/24.
//

#include "daemon.h"
#include "thread.h"
#include "signals.h"
#include "filelock.h"
#include "singleton.h"
#include "setusergroup.h"

#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>

using std::cout;
using std::endl;

namespace zcUtils {
    // signal handlers.
    static SighupHandler sighupHandler;
    static SigquitHandler sigquitHandler;
    static SigexitHandler sigexitHandler;
    static Sigusr1Handler sigusr1Handler;

    // initialisation timeout value.
    const int Daemon::MAX_INIT_TIMEOUT = 10; // seconds

    Daemon::Daemon(const string &name, const string &description, const string &version, const string &build_time,
                   const string &working_dir, const string &lock_file)
            : Application(name, description, version, build_time) {
        m_cOptions_.addSwitchOption('h', "help", "Display help information.");
        m_cOptions_.addSwitchOption('v', "version", "Display version information.");
        m_cOptions_.addSwitchOption('i', "info", "Display used parameters.");
        m_cOptions_.addSwitchOption('d', "daemon", "Run with daemon.");

        m_cOptions_.addValueOption('u', "user", "Run as this user.");
        m_cOptions_.addValueOption('g', "group", "Run as this group.");

        m_nErrorCode_ = ERR_SUCCESS_E;
        m_pFileLock_ = new FileLock(lock_file);
        m_strWorkingDir_ = working_dir;
        m_nStarterPid_ = getpid();
    }

    Daemon::~Daemon() {
        delete m_pFileLock_;
    }

    bool Daemon::addCustomOption(char short_name, const string &long_name,
                                 const string &description, bool value_option) {
        if (value_option)
            m_cOptions_.addValueOption(short_name, long_name, description);
        else
            m_cOptions_.addSwitchOption(short_name, long_name, description);
        return true;
    }

    bool Daemon::run(int argc, char *argv[]) {
        bool ret = false;
        int options_count = m_cOptions_.readOptions(argc, argv);
        if (options_count > 0) {
            argc -= options_count;
            argv += options_count;
        }

        // TODO(Passerby): setupTracing function is empty now, can add functions later.
        setupTracing();
        printInfo();

        if (m_pFileLock_->isLocked()) {
            cout << "Another copy of process is already running" << endl;
        } else {
            if (personalize() && setPwd()) {
                if (!m_cOptions_.getSwitchOption("daemon") || daemonize()) {
                    setupLog();
                    setupSignals();
                    if (m_pFileLock_->lock()) {
                        if (m_nStarterPid_ != getpid()) // we're running in daemon mode
                            kill(m_nStarterPid_,
                                 SIGUSR1); // send notifications to parent about successfully initialisation
                        ret = true;
                        if (start(m_cOptions_)) {
                            time_t start_time = time(NULL);
                            int signal_num = waitForShutdown();
                            shutdown();
                            time_t up_time = time(NULL) - start_time;
                            int days = up_time / (24 * 3600);
                            int hours = (up_time % (24 * 3600)) / 3600;
                            int minutes  = (up_time % 3600) / 60;
                            int seconds = up_time % 60;
                            cout << getName() << " daemon: shut down with signal " << signal_num << ". Up time:" << days
                                 << "days" << hours << "hours" << minutes << "minutes" << seconds << "seconds." << endl;
                        } else
                            m_nErrorCode_ = ERR_INITIALISE_FAILED_E;
                    } else
                        m_nErrorCode_ = ERR_CREATE_LOCK_FAILED_E;
                }
            }
        }
        return ret;
    }

    // for Options function and settings, just for expanding
    void Daemon::setConfigFile(const string filename) {
        m_cOptions_.setConfigFile(filename);
    }

    void Daemon::addSwitchOption(char short_name, const string long_name, const string description) {
        m_cOptions_.addSwitchOption(short_name, long_name, description);
    }

    void Daemon::addValueOption(char short_name, const string long_name, const string description) {
        m_cOptions_.addValueOption(short_name, long_name, description);
    }
}