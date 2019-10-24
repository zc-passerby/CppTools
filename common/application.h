//
// Created by Passerby on 2019/10/23.
//

#ifndef ZCUTILS_APPLICATION_H
#define ZCUTILS_APPLICATION_H

#include "options.h"

#include <string>

using std::string;

namespace zcUtils {
    class Application {
    public:
        /*
         * Brief:
         *     The ctor should setup the core application details only.
         *     It should _not_ write anything to stdout/stderr.
         *     This is allows command line options to be used to query
         *     and display application information
         *     and exit without starting the application itself.
         * Params:
         *     name - application's name (should be the same as the binary).
         *     description - brief description of the application.
         *     version - identifies the version of the application.
         *     build_time - the build time of the program.
         */
        Application(const string &name, const string &description, const string &version, const string &build_time)
                : m_strName_(name), m_strDescription_(description), m_strVersion_(version),
                  m_strBuildTime_(build_time) {}

        virtual ~Application() {}

        // Get the application's name
        const string &getName() const { return m_strName_; }

        // Get the application's description
        const string &getDescription() const { return m_strDescription_; }

        // Get the application's version
        const string &getVersion() const { return m_strVersion_; }

        // Get the application's build time
        const string &getBuildTime() const { return m_strBuildTime_; }

        /*
         * Brief:
         *     Initial entry point.
         * Params:
         *     options - command line option.
         */
        virtual bool start(const Options &options) = 0;

        // Reload configuration.
        virtual void restart() = 0;

        // Graceful shutdown.
        virtual void terminate() = 0;

    protected:
        string m_strName_;
        string m_strDescription_;
        string m_strVersion_;
        string m_strBuildTime_;
    };
}

#endif //ZCUTILS_APPLICATION_H
