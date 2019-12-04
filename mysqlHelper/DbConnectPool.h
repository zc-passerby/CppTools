#ifndef _DB_CONNECT_POOL_H_
#define _DB_CONNECT_POOL_H_

#include "threadUtil.h"
#include <pthread.h>
#include <memory>
#include "MysqlApi.h"

using namespace std;

class Connection
{
public:
    int nNumber;
    int nCount;
    MysqlApi::DataBase hDB;

    Connection() : nNumber(-1), nCount(0) {}
    virtual ~Connection() {}
};

class CdbConncetPool
{
public:
protected:
private:
    pthread_mutex_t m_lock;

    static auto_ptr<CdbConncetPool> gInstance;

    string m_DbUser;
    string m_DbPwd;
    string m_DbServer;
    string m_DbDataBase;
    unsigned int m_DbPort;
    unsigned int m_ConnectTimeout;
    unsigned int m_ReadTimeout;
    unsigned int m_WriteTimeout;

    SafeQueue<Connection *> m_queue;
    sem_t m_sem;
    bool m_initialized;
    bool m_stop;
    int m_num;

public:
    CdbConncetPool() : m_initialized(false), m_stop(false), m_num(10)
    {
        pthread_mutex_init(&m_lock, NULL);
    }
    ~CdbConncetPool()
    {
        Stop();
        sem_destroy(&m_sem);
        pthread_mutex_destroy(&m_lock);
    }

    static CdbConncetPool *Instance();

    bool Init();

    bool PutMsg(Connection *pConn);

    bool CreateConnectionPool(const char *pDbServer, const char *pDbDatabase, const char *pDbUser, const char *pDbPwd, unsigned int pDport,
                              int nConnNum = 1, unsigned int unConnectTimeout = 10, unsigned int unReadTimeout = 3, unsigned int unWriteTimeout = 10);

    Connection *GetConnection();

    void ReleaseConnection(Connection *pConn);
    void TerminateConnection(Connection *pConn);
    Connection *ReCreateConnection(Connection *pConn);

    bool isInitialized()
    {
        autoLock al(m_lock);
        return m_initialized;
    }

protected:
private:
    bool Stop();
    short ConnectDB(MysqlApi::DataBase &hDB);
    short InitDatabase();
    short DisConnectDB(MysqlApi::DataBase &hDB);
    Connection *CreateConnection();
};

#endif