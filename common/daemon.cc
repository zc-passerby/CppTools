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
        m_cOptions_.addSwitchOption('\0', "\0", "带有*参数的指令表示不执行程序，仅打印信息");
        m_cOptions_.addSwitchOption('h', "help", "显示帮助信息*");
        m_cOptions_.addSwitchOption('v', "version", "显示版本信息*");
        m_cOptions_.addSwitchOption('i', "info", "显示所有使用的参数信息*");
        m_cOptions_.addSwitchOption('d', "daemon", "使用守护进程在后台执行");

//        m_cOptions_.addValueOption('u', "user", "切换到指定用户执行");
//        m_cOptions_.addValueOption('g', "group", "切换到指定组执行");

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
        if (printInfo())
            ret = false;
        else {
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
                                int minutes = (up_time % 3600) / 60;
                                int seconds = up_time % 60;
                                cout << getName() << " daemon: shut down with signal " << signal_num << ". Up time:"
                                     << days << "days " << hours << "hours " << minutes << "minutes " << seconds
                                     << "seconds." << endl;
                            } else
                                m_nErrorCode_ = ERR_INITIALISE_FAILED_E;
                        } else
                            m_nErrorCode_ = ERR_CREATE_LOCK_FAILED_E;
                    }
                }
            }
        }
        return ret;
    }

    int Daemon::waitForShutdown() {
        return Signal::instance()->waitSignal(Signal::INFINITE_TIMEOUT);
    }

    void Daemon::setupTracing() {}

    bool Daemon::reload() {
        bool ret = false;
        restart();
        return ret;
    }

    bool Daemon::shutdown() {
        m_pFileLock_->unlock();
        terminate();
        return true;
    }

    bool Daemon::printInfo() {
        bool ret = false;
        if (m_cOptions_.getSwitchOption("help")) {
            cout << &m_cOptions_ << endl;
            ret = true;
        }

        if (m_cOptions_.getSwitchOption("version")) {
            cout << getName() << " Version: " << getVersion() << " BuildTime: " << getBuildTime() << endl;
            ret = true;
        }

        if (m_cOptions_.getSwitchOption("info")) {
            const vector<Options::Option *> options_vec = m_cOptions_.getOptions();
            vector<Options::Option *>::const_iterator it;
            cout << "Options set:" << endl;
            for (it = options_vec.begin(); options_vec.end() != it; ++it) {
                Options::Option *option = *it;
                cout << option->long_name << " : ";
                if (option->has_argument)
                    cout << option->value;
                else {
                    if (option->found)
                        cout << "Yes";
                    else
                        cout << "No";
                }
                cout << endl;
            }
            ret = true;
        }
        return ret;
    }

    bool Daemon::personalize() {
        bool ret = true;
        string option_user = m_cOptions_.getValueOption("user");
        string option_group = m_cOptions_.getValueOption("group");
        if (!option_user.empty()) {
            if (setUserGroup(option_user, option_group))
                cout << "Changed personality to:" << option_group << "/" << option_user << endl;
            else {
                cout << "Failed to change group/username to:" << option_group << "/" << option_user << endl;
                m_nErrorCode_ = ERR_PERSONALISATION_FAILED_E;
                ret = false;
            }
        }
        return ret;
    }

    bool Daemon::setPwd() {
        bool ret = false;
        // Change the current working directory
        if (m_strWorkingDir_.empty() || chdir(m_strWorkingDir_.c_str()) == 0) {
            cout << "Changing working directory to:'" << m_strWorkingDir_ << "'" << endl;
            ret = true;
        } else {
            cout << "Failed to change directory to:'" << m_strWorkingDir_ << "'" << endl;
            m_nErrorCode_ = ERR_CHDIR_FAILED_E;
        }
        return ret;
    }

    void Daemon::setupLog() {}

    void Daemon::setupSignals() {
        Signal::instance()->setHandler(SIGTSTP, SIG_IGNORE_HANDLER, true);
        Signal::instance()->setHandler(SIGTTOU, SIG_IGNORE_HANDLER, true);
        Signal::instance()->setHandler(SIGTTIN, SIG_IGNORE_HANDLER, true);
        Signal::instance()->setHandler(SIGPIPE, SIG_IGNORE_HANDLER, true); /* send()/write() on socket */
        Signal::instance()->setHandler(SIGURG, SIG_IGNORE_HANDLER, true);  /* socket got urgent data */
        Signal::instance()->setHandler(SIGUSR1, SIG_IGNORE_HANDLER, true);
        Signal::instance()->setHandler(SIGUSR2, SIG_IGNORE_HANDLER, true);

        Signal::instance()->setHandler(SIGABRT, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGALRM, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGFPE, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGHUP, &sighupHandler, true);
        Signal::instance()->setHandler(SIGILL, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGINT, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGQUIT, &sigquitHandler, true);
        Signal::instance()->setHandler(SIGSEGV, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGTERM, &sigexitHandler, true);
        Signal::instance()->setHandler(SIGXCPU, &sigexitHandler, true); /* cpu time */
        Signal::instance()->setHandler(SIGXFSZ, &sigexitHandler, true); /* file size */
    }

    bool Daemon::daemonize() {
        bool ret = true;
        // Our process ID and Session ID
        pid_t pid, sid;
        // Fork off the parent process
        Signal::instance()->setHandler(SIGUSR1, SIG_IGNORE_HANDLER, true);
        pid = fork();
        if (pid < 0) {
            cout << "Daemon: Failed to create child process. Error:" << errno << endl;
            m_nErrorCode_ = ERR_FORK_FAILED_E;
            ret = false;
        } else if (pid > 0) { // If we got a good PID, then we can exit the parent process.
            ret = false;
            cout << "Waiting for daemon initialisation..." << endl;
            // We will give some time for early init and check if process is still working.
            Signal::instance()->setHandler(SIGUSR1, &sigusr1Handler, true);
            Signal::instance()->setHandler(SIGCHLD, &sigusr1Handler, true);
            // Waiting max MAX_INIT_TIMEOUT seconds for the child initialisation or termination.
            Signal::instance()->waitSignal(MAX_INIT_TIMEOUT);

            int status = 0;
            pid_t res_pid = waitpid(pid, &status, WNOHANG);
            if (res_pid == pid) {
                if (WIFEXITED(status)) { // Process exited normally.
                    m_nErrorCode_ = (DAEMON_ERROR_CODES) WEXITSTATUS(status);
                    cout << "Daemon:: Child process exited with an error code:"
                         << nameOf(m_nErrorCode_) << " (" << m_nErrorCode_ << ")" << endl;
                    if (m_nErrorCode_ == ERR_SUCCESS_E)
                        m_nErrorCode_ = ERR_INIT_FAILED_E;
                }
                if (WIFSIGNALED(status)) { // Process terminated by signal.
                    int terminate_signal = WTERMSIG(status);
                    cout << "Daemon:: Child terminated by signal:" << terminate_signal;
                    if (WCOREDUMP(status))
                        cout << "Daemon:: Coredump for child process was generated." << endl;
                    m_nErrorCode_ = ERR_INIT_FAILED_E;
                }
            } else if (res_pid == 0) { // Process is still working.
                if (m_pFileLock_->isLocked()) {
                    cout << "Initialized successfully. PID:" << pid << endl;
                    m_nErrorCode_ = ERR_SUCCESS_E;
                } else {
                    cout << "Initialisation time excited " << MAX_INIT_TIMEOUT << "."
                         << " Daemon process is still working, but pid file has not been locked." << endl;
                    m_nErrorCode_ = ERR_INIT_FAILED_E;
                }
            } else {
                cout << "Daemon:: Log error. Can't retrieve information about child status." << endl;
                m_nErrorCode_ = ERR_FORK_FAILED_E;
            }
        } else { // We are in new process.
            // Redirect I/O to the null device.
            close(0);
            close(1);
            close(2);
            int fd = open("/dev/null", O_RDWR);
            dup(fd);
            dup(fd);
            // Change the file mode mask.
            umask(0);
            // Create a new session ID for the child process.
            sid = setsid();
            if (sid < 0) {
                cout << "daemonize failed to setsid()." << endl;
                m_nErrorCode_ = ERR_SID_FAILED_E;
            }
        }
        return ret;
    }

    const char *Daemon::nameOf(DAEMON_ERROR_CODES code) const {
        const char *szCode = "UNKNOWN";
        switch (code) {
            case ERR_SUCCESS_E:
                szCode = "SUCCESS";
                break;
            case ERR_PERSONALISATION_FAILED_E:
                szCode = "PERSONALISATION_FAILED";
                break;
            case ERR_ALREADY_RUNNING_E:
                szCode = "ALREADY_RUNNING";
                break;
            case ERR_INIT_FAILED_E:
                szCode = "INIT_FAILED";
                break;
            case ERR_CHDIR_FAILED_E:
                szCode = "CHDIR_FAILED";
                break;
            case ERR_FORK_FAILED_E:
                szCode = "FORK_FAILED";
                break;
            case ERR_SID_FAILED_E:
                szCode = "SID_FAILED";
                break;
            case ERR_CREATE_LOCK_FAILED_E:
                szCode = "CREATE_LOCK_FAILED";
                break;
            case ERR_INITIALISE_FAILED_E:
                szCode = "INITIALISE_FAILED";
                break;
            default:
                break;
        }
        return szCode;
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