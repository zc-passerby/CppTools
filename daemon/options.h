//
// Created by Passerby on 2019/10/23.
//

#ifndef ZCUTILS_OPTIONS_H
#define ZCUTILS_OPTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <map>
#include <vector>
#include <iostream>

using std::map;
using std::vector;
using std::string;

namespace zcUtils {
    /*
     * Brief:
     *     Utility for extracting options from the command line argument array.
     *
     * Program options can be also specified as environment variables and configuration file
     * If the same options are specified in different places its value are overwritten following order:
     *     environment variables always overwrite config file variables
     *     commandline options always overwrite environment variables and config file
     *
     * If switch options were set in file or environment there is no method of resetting them from command line.
     * In such case they stays "switched on"
     * In an environment and a file switch option can be set to:
     *     FALSE - if option has empty value (e.g. export example=)
     *     TRUE  - if option has non-empty value (e.g. export example=Yes)
     */
    class Options {
    public:
        struct Option {
            char short_name; // single character name e.g. -v
            string long_name; // string name e.g. --verbose
            bool has_argument; // true, if this option has a argument
            string description; // a short description of this option
            string value; // receives the argument's value
            bool found; // true, if this option is present

            Option(char short_name, const string &long_name, bool has_argument, string description)
                    : short_name(short_name),
                      long_name(long_name),
                      has_argument(has_argument),
                      description(description),
                      found(false) {}
        };

        // ctor. default only
        Options() {}
        // dtor. remove the option objects
        ~Options() {
            for (vector<Option *>::iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it)
                delete (*it);
        }

        void setConfigFile(const string filename) {
            m_strConfigFile_ = filename;
        }

        

    private:
        string m_strConfigFile_;
        vector<Option *> m_vecOptions_;
    };
}

#endif //ZCUTILS_OPTIONS_H
