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
using std::pair;
using std::vector;
using std::string;
using std::ostream;

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

        /*
         * A value-option is an option that has an requires an explicit value.
         * e.g. --foo=bar or --foo bar
         *
         * Params:
         *     short_name - single character name, set to zero if not required
         *     long_name - long string name
         *     description - a short description of the option
         */
        void addValueOption(char short_name, const string long_name, const string description) {
            Option *option = new Option(short_name, long_name, required_argument, description);
            m_vecOptions_.push_back(option);
        }

        /*
         * A switch-option does not have an explicit value and is used for boolean options.
         * e.g. --foo
         *
         * Params:
         *     short_name - single character name, set to zero if not required
         *     long_name - long string name
         *     description - a short description of the option
         */
        void addSwitchOption(char short_name, const string long_name, const string description) {
            Option *option = new Option(short_name, long_name, no_argument, description);
            m_vecOptions_.push_back(option);
        }

        // Accessors for retrieving an option's value.
        const string getValueOption(char short_name) const {
            Option *option = find(short_name);
            if (option)
                return option->value;
            return "";
        }

        const string getValueOption(const string &long_name) const {
            Option *option = find(long_name);
            if (option)
                return option->value;
            return "";
        }

        // Accessors for testing if a switch-option was set
        bool getSwitchOption(char short_name) const {
            Option *option = find(short_name);
            if (option)
                return option->found;
            return false;
        }

        bool getSwitchOption(const string &long_name) const {
            Option *option = find(long_name);
            if (option)
                return option->found;
            return false;
        }

        // Retrieve all options that were found
        void getAll(map <string, string> &values) const {
            values.clear();
            for (vector<Option *>::const_iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it) {
                Option *option = *it;
                if (option->found)
                    values.insert(pair<string, string>(option->long_name, option->value));
            }
        }

        // Read-in command line options.
        int readOptions(int argc, char *argv[]) {
            readConfigFileOptions();
            readEnvironmentOptions();
            string strOption;
            ::option *option_array = new option[m_vecOptions_.size() + 1];
            ::option *pOption = option_array;

            for (vector<Option *>::const_iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it) {
                Option *opt = *it;
                pOption->name = opt->long_name.c_str();
                pOption->has_arg = opt->has_argument ? required_argument : no_argument;
                pOption->flag = 0;
                pOption->val = opt->short_name;
                if (opt->short_name) {
                    strOption += opt->short_name;
                    if (pOption->has_arg == required_argument)
                        strOption += ':';
                }
                ++pOption;
            }
            pOption->name = 0;
            pOption->has_arg = 0;
            pOption->flag = 0;
            pOption->val = 0;

            ::opterr = 0; // Stops getopt from printing messages to stderr.

            while (1) {
                int option_index = -1;
                int c = ::getopt_long(argc, argv, strOption.c_str(), option_array, &option_index);
                if (c == -1)
                    break;
                Option *o = find(c);
                if (!o && (option_index != -1))
                    o = find(option_array[option_index].name);
                if (o) {
                    o->found = true;
                    if (o->has_argument)
                        o->value = optarg ? optarg : "null";
                }
            }
            delete[]option_array;

            return ::optind;
        }

        // Prints all options in a format suitable for help screens.
        friend ostream &operator<<(ostream &s, const Options *o);

        // Get list of all available options
        const vector<Option *> &getOptions() {
            return m_vecOptions_;
        }

        ostream &print(ostream &s) const {
            for (vector<Option *>::const_iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it) {
                Option *option = *it;
                if (option->short_name)
                    s << "-" << option->short_name << ", ";
                s << "--" << option->long_name;
                s << "\t" << option->description;
                s << std::endl;
            }
            return s;
        }

    private:
        // Lookup an option struct by its short_name
        Option *find(char short_name) const {
            if (short_name != '\0') {
                for (vector<Option *>::const_iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it) {
                    Option *option = *it;
                    if (option->short_name == short_name)
                        return option;
                }
            }
            return NULL;
        }

        // Lookup an option struct by its long_name
        Option *find(const string &long_name) const {
            for (vector<Option *>::const_iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it) {
                Option *option = *it;
                if (option->long_name == long_name)
                    return option;
            }
            return NULL;
        }

        // Read options from environment variables
        int readEnvironmentOptions() {
            int ret = 0;

            for (vector<Option *>::const_iterator it = m_vecOptions_.begin(); m_vecOptions_.end() != it; ++it) {
                Option *option = *it;
                char *opt = ::getenv(option->long_name.c_str());
                if (opt) {
                    ret++;
                    if (option->has_argument) {
                        option->found = true;
                        option->value = opt;
                    } else
                        option->found = (strlen(opt) > 0 && strcasecmp(opt, "no") != 0);
                }
                ++opt;
            }
            return ret;
        }

        // Read options from config file
        int readConfigFileOptions() {
            int ret = 0;
            if (m_strConfigFile_.length()) {
                FILE *fh = NULL;
                if ((fh = ::fopen(m_strConfigFile_.c_str(), "r")) > 0) {
                    char *line = NULL;
                    size_t len = 0;
                    ssize_t read_length = 0;
                    while ((read_length = ::getline(&line, &len, fh)) != -1) {
                        // strip out comments
                        char *comment1 = ::strchr(line, ';');
                        char *comment2 = ::strchr(line, '#');
                        if (comment1)
                            (*comment1) = '\0';
                        if (comment2)
                            (*comment2) = '\0';

                        char *tmp = ::strchr(line, '=');
                        if (tmp) {
                            char *name = line;
                            char *val = tmp + 1;
                            (*tmp) = '\0';
                            /*
                             * Remove all blank and new line characters from
                             * the beginning and the end of name and value.
                             */
                            while (*name == ' ' || *name == '\t')
                                name++;
                            while (--tmp >= line && (*tmp == ' ' || *tmp == '\t'))
                                *tmp = '\0';
                            // Remove all blank cars from the beginning and the end of name.
                            while (*val == ' ' || *val == '\t')
                                val++;
                            tmp = val + strlen(val);
                            while (--tmp >= val && (*tmp == ' ' || *tmp == '\t' || *tmp == '\r' || *tmp == '\n'))
                                *tmp = '\0';

                            Option *option = find(name);
                            if (option) {
                                ret++;
                                if (option->has_argument) {
                                    option->found = true;
                                    option->value = val;
                                } else
                                    option->found = (strlen(val) > 0 && strcasecmp(val, "no") != 0);
                            }
                        }
                    }
                    if (line)
                        free(line);
                    fclose(fh);
                }
            }
            return ret;
        }

    private:
        string m_strConfigFile_;
        vector<Option *> m_vecOptions_;
    };

    // Overload << for printing
    inline ostream &operator<<(ostream &s, const Options *o) {
        return o->print(s);
    }
}

#endif //ZCUTILS_OPTIONS_H
