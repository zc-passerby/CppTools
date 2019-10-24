//
// Created by Passerby on 2019/10/24.
//

#ifndef ZCUTILS_SETUSERGROUP_H
#define ZCUTILS_SETUSERGROUP_H

#include <string>

using std::string;

namespace zcUtils {
    /*
     * Brief:
     *     Change the user and group we're running as.
     * Params:
     *     user_name - name of user to run as.
     *     group_name - name of group to run as (optional).
     * Return:
     *     true, if successful.
     *     false, if requested failed.
     */
    bool setUserGroup(const string &user_name, const string &group_name);
}

#endif //ZCUTILS_SETUSERGROUP_H
