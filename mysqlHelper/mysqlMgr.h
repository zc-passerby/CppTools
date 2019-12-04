#ifndef _MYSQL_MGR_H_
#define _MYSQL_MGR_H_

#include <string>
#include "singleton.h"
#include "DbConnectPool.h"
#include "MysqlApi.h"

using namespace zcUtils;

class PLMySql
{
private:
    CdbConncetPool &pool;
    Connection *conn;

public:
    PLMySql()
        : pool(Singleton<CdbConncetPool>::instance()),
          conn(NULL)
    {
        //try
        //{
        conn = pool.GetConnection();
        //}
        //catch(...)
        //{
        //}
    };
    ~PLMySql()
    {
        //try
        //{
        pool.ReleaseConnection(conn);
        conn = NULL;
        //}
        //catch(...)
        //{
        //}
    };
    MysqlApi::DataBase *get_db_instance()
    {
        if (conn)
            return &conn->hDB;
        return NULL;
    }
};
#endif