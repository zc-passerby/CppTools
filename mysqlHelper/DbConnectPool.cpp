#include <iostream>
#include <unistd.h>

#include "timeval.h"
#include "MysqlApi.h"
#include "DbConnectPool.h"

#define MAXCOUNT 100000
auto_ptr <CdbConncetPool> CdbConncetPool::gInstance(new CdbConncetPool);

CdbConncetPool *CdbConncetPool::Instance() {
    if (0 == gInstance.get()) {
        gInstance.reset(new CdbConncetPool);
    }
    return gInstance.get();
}

short CdbConncetPool::InitDatabase() {

    //MysqlApi::DataBase  hDB;
//    cout << "InitDatabase complete!" << endl;
    return 0;
}

short CdbConncetPool::ConnectDB(MysqlApi::DataBase &hDB) {
    try {
        // Create database connection
//        cout << "Connect to db,paras are:"
//             << " user:" << m_DbUser
//             << " m_DbPwd:" << m_DbPwd
//             << " m_DbServer:" << m_DbServer
//             << " m_DbDataBase:" << m_DbDataBase
//             << " m_DbPort:" << m_DbPort
//             << " m_ConnectTimeout:" << m_ConnectTimeout
//             << " m_ReadTimeout:" << m_ReadTimeout
//             << " m_WriteTimeout:" << m_WriteTimeout
//             << endl;
        int rt;
        rt = hDB.Connect(m_DbServer.c_str(), m_DbUser.c_str(), m_DbPwd.c_str(), m_DbDataBase.c_str(), m_DbPort,
                         m_ConnectTimeout, m_ReadTimeout, m_WriteTimeout, 0);
        if (rt < 0) {
            cout << "connect is fail! rt =" << rt << endl;
            return -1;
        } else {
//            cout << "connect is success! rt = " << rt << endl;
        }
        return 0;
    }
    catch (...) {
        cout << "connect error!" << endl;
        return -1;
    }
}

short CdbConncetPool::DisConnectDB(MysqlApi::DataBase &hDB) {
    try {
        hDB.DisConnect();
    }
    catch (...) {
        cout << "disconnectDB error !" << endl;
        return -1;
    }
    return 0;
}

bool CdbConncetPool::Init() {
    autoLock al(m_lock);

    if (m_initialized)
        return false;

    int err = sem_init(&m_sem, 0, 0);
    if (0 != err) {
        cout << "CdbConncetPool::Init Error: failed to do sem_init." << endl;
        return false;
    }
    InitDatabase();
    m_initialized = true;
    return true;
}

bool CdbConncetPool::PutMsg(Connection *pConn) {
    if (!m_initialized)
        return false;

    //put message into the queue, and signal semaphore
    m_queue.PutMsg(pConn);
    if (m_stop)
        return true;
    int err = sem_post(&m_sem);
    if (0 != err) {
        cout << "CdbConncetPool::PutMsg Error: failed to do sem_post." << endl;
        return false;
    }
    return true;
}

bool CdbConncetPool::Stop() {
    if (!m_initialized)
        return false;
    m_stop = true;

    Connection *pConn = NULL;

    for (int i = 0; i < m_num; i++) {
        //disconnect DB handle
        if (m_queue.GetMsg(pConn)) {
            TerminateConnection(pConn);
        } else {
            sleep(1);
            i--;
        }
    }
    m_initialized = false;
    return true;
}

Connection *CdbConncetPool::CreateConnection() {
    Connection *pConn = new Connection;
    if (NULL == pConn) {
        cout << "Create Connection object failed!" << endl;
        return NULL;
    }
    if (ConnectDB(pConn->hDB) < 0) {
        delete pConn;
        return NULL;
    }
    return pConn;
}

Connection *CdbConncetPool::GetConnection() {

    unsigned long costtime;
    unsigned long starttime = util::get_current_time_stamp();

    _RETYR_:
    int err = sem_wait(&m_sem);
    if (0 != err) {
        cout << "CdbConncetPool::Run Error: failed to do sem_wait." << endl;
        if (errno == EINTR)
            goto _RETYR_;
        else
            return NULL;
    }

    if (m_stop)
        return NULL;

    costtime = (util::get_current_time_stamp() - starttime) / 1000;
    if (costtime >= 1000)
        cout << "Warn:Use " << costtime << "s to get dbpool connection..." << endl;

    Connection *pConn = NULL;
    if (m_queue.GetMsg(pConn)) {
        /********
		if(pConn->nCount++>MAXCOUNT)
		{
			cout <<"GetConnection Times longer than the maximum,Recreate connection..." << endl;
			pConn=ReCreateConnection(pConn);
		}
********/
        return pConn;
    }
    return NULL;
}

void CdbConncetPool::ReleaseConnection(Connection *pConn) {
    if (pConn != NULL)
        PutMsg(pConn);
}

bool CdbConncetPool::CreateConnectionPool(const char *pDbServer, const char *pDbDatabase, const char *pDbUser,
                                          const char *pDbPwd, unsigned int pDbPort,
                                          int nConnNum, unsigned int unConnectTimeout, unsigned int unReadTimeout,
                                          unsigned int unWriteTimeout) {
    if (!m_initialized)
        return false;

    m_DbUser = pDbUser;
    m_DbPwd = pDbPwd;
    m_DbDataBase = pDbDatabase;
    m_DbServer = pDbServer;
    m_DbPort = pDbPort;
    m_num = nConnNum;
    m_ConnectTimeout = unConnectTimeout;
    m_ReadTimeout = unReadTimeout;
    m_WriteTimeout = unWriteTimeout;

    for (int i = 0; i < nConnNum; i++) {
        //Create DB handle connect pool
        Connection *pConn = CreateConnection();
        if (NULL == pConn) {
            return false;
        }
        pConn->nNumber = i;
        PutMsg(pConn);
    }

    return true;
}

void CdbConncetPool::TerminateConnection(Connection *pConn) {
    if (NULL == pConn)
        return;
    DisConnectDB(pConn->hDB);
    delete pConn;
    pConn = NULL;
}

Connection *CdbConncetPool::ReCreateConnection(Connection *pConn) {
    cout << "enter ReCreateConnection" << endl;
    if (NULL == pConn) {
        cout << "pConn is NULL!we will return!" << endl;
        return NULL;
    }
    int nNumber = pConn->nNumber;
    TerminateConnection(pConn);

    while (1) {
        pConn = CreateConnection();
        if (NULL == pConn) {
            cout << "CreateConnection failed,recreate connection...." << endl;
            sleep(1);
        } else {
            pConn->nNumber = nNumber;
            cout << "CreateConnection success!" << endl;
            break;
        }
    }

    return pConn;
}