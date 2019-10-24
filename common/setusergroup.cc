//
// Created by Passerby on 2019/10/24.
//

#include "setusergroup.h"

#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace zcUtils {
    /*
     * Validate the user name along with the group name if it has been specified.
     * Change to group's settings must occur before changing the user's settings.
     */
    bool setUserGroup(const string &user_name, const string &group_name) {
        struct passwd *password = NULL;
        if (!user_name.empty())
            password = getpwnam(user_name.c_str());
        if (!password)
            return false;

        struct group *gr = NULL;
        if (!group_name.empty())
            gr = getgrnam(group_name.c_str());
        if (!gr)
            return false;

        // Check if already running as the specified user and group (if provided).
        if ((getuid() == password->pw_uid) && (getgid() == gr->gr_gid))
            return true;

        // Drop all groups. Must be done before other changes.
        if (setgroups(0, NULL) != 0)
            return false;

        if (setgid(gr->gr_gid) != 0)
            return false;

        if (setuid(password->pw_uid) != 0)
            return false;

        return true;
    }
}