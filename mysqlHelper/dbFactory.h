//
// Created by Passerby on 2019/11/6.
//

#ifndef MAGIC_CUBE_DBFACTORY_H
#define MAGIC_CUBE_DBFACTORY_H

#include <string>

#include "mysqlMgr.h"

enum DB_OPERATOR_RESULT {
    DB_OPERATOR_RESULT_OK = 0,
    DB_OPERATOR_RESULT_FATAL_ERROR = 1,
    DB_OPERATOR_RESULT_PARAM_ERROR = 2,
    DB_OPERATOR_RESULT_NO_RESULT = 3,
};

static string dbResultToString(DB_OPERATOR_RESULT db_rst)
{
    switch (db_rst)
    {
        case DB_OPERATOR_RESULT_OK:
            return "DB_OPERATOR_RESULT_OK";
        case DB_OPERATOR_RESULT_FATAL_ERROR:
            return "DB_OPERATOR_RESULT_FATAL_ERROR";
        case DB_OPERATOR_RESULT_PARAM_ERROR:
            return "DB_OPERATOR_RESULT_PARAM_ERROR";
        case DB_OPERATOR_RESULT_NO_RESULT:
            return "DB_OPERATOR_RESULT_NO_RESULT";
        default:
            break;
    };
    return "Unknown db result!";
}

class DbFactory {
public:
    /*
     * 通过手机号前三位获取运营商，以运营商group by
     * pre:手机号前三位
     * provider:查询出来的运营商
     * count:查询的结果数，如果多于1的话，则表示多运营商共用，目前仅170号段共用了
     */
    static DB_OPERATOR_RESULT getProviderByPrefix(const string &pre, string &provider, int &count);

    /*
     * 通过手机号前7位获取运营商，手机号前七位是唯一的
     */
    static DB_OPERATOR_RESULT getProviderByPhone(const string &phone, string &provider);

    /*
     * 通过手机号前7位获取省市，手机号前七位是唯一的
     */
    static DB_OPERATOR_RESULT getRegionByPhone(const string &phone, string &province, string &city);
};

#endif //MAGIC_CUBE_DBFACTORY_H
