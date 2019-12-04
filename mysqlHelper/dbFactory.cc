//
// Created by Passerby on 2019/11/6.
//

#include "dbFactory.h"

#include <sstream>

DB_OPERATOR_RESULT DbFactory::getProviderByPrefix(const string &pre, string &provider, int &count) {
    if (!pre.length())
        return DB_OPERATOR_RESULT_PARAM_ERROR;


    // 先初始化
    provider = "";
    count = 0;

    // get db instance
    PLMySql mysql_db;
    MysqlApi::DataBase *new_db = mysql_db.get_db_instance();
    stringstream select_sql;
    select_sql << "select isp from phone_number_region where prefix='" << pre << "' group by isp;";

    MysqlApi::RecordSet rs(new_db->GetMysql());
    int retCount = rs.ExecuteSQL(select_sql.str());
    if (-1 == retCount)
        return DB_OPERATOR_RESULT_FATAL_ERROR;
    else if (0 == retCount)
        return DB_OPERATOR_RESULT_NO_RESULT;

    provider = rs.GetCurrentFieldValue(0);
    count = retCount;

    return DB_OPERATOR_RESULT_OK;
}

DB_OPERATOR_RESULT DbFactory::getProviderByPhone(const string &phone, string &provider) {
    if (!phone.length())
        return DB_OPERATOR_RESULT_PARAM_ERROR;

    provider = "";

    // get db instance
    PLMySql mysql_db;
    MysqlApi::DataBase *new_db = mysql_db.get_db_instance();
    stringstream select_sql;
    select_sql << "select isp from phone_number_region where phone='" << phone << "';";

    MysqlApi::RecordSet rs(new_db->GetMysql());
    int retCount = rs.ExecuteSQL(select_sql.str());
    if (-1 == retCount)
        return DB_OPERATOR_RESULT_FATAL_ERROR;
    else if (0 == retCount)
        return DB_OPERATOR_RESULT_NO_RESULT;
    else if (1 != retCount) {
        cout << "getProviderByPhone::phone=" << phone << "结果不唯一:" << retCount << endl;
        return DB_OPERATOR_RESULT_FATAL_ERROR;
    }

    provider = rs.GetCurrentFieldValue(0);
    return DB_OPERATOR_RESULT_OK;
}

DB_OPERATOR_RESULT DbFactory::getRegionByPhone(const string &phone, string &province, string &city) {
    if (!phone.length())
        return DB_OPERATOR_RESULT_PARAM_ERROR;

    province = "";
    city = "";

    // get db instance
    PLMySql mysql_db;
    MysqlApi::DataBase *new_db = mysql_db.get_db_instance();
    stringstream select_sql;
    select_sql << "select province,city from phone_number_region where phone='" << phone << "';";

    MysqlApi::RecordSet rs(new_db->GetMysql());
    int retCount = rs.ExecuteSQL(select_sql.str());
    if (-1 == retCount)
        return DB_OPERATOR_RESULT_FATAL_ERROR;
    else if (0 == retCount)
        return DB_OPERATOR_RESULT_NO_RESULT;
    else if (1 != retCount) {
        cout << "getRegionByPhone::phone=" << phone << "结果不唯一:" << retCount << endl;
        return DB_OPERATOR_RESULT_FATAL_ERROR;
    }

    province = rs.GetCurrentFieldValue(0);
    city = rs.GetCurrentFieldValue(1);
    return DB_OPERATOR_RESULT_OK;
}