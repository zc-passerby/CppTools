#ifndef __ZC_MYSQL_API_H__
#define __ZC_MYSQL_API_H__

#include <mysql/mysql.h>

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;

/*
 * 字段操作
 */
class Field {
public:
    // 字段名称
    vector<string> m_vsName;
    // 字段类型
    vector<enum_field_types> m_veType;
    // 字段所属表名
    vector<string> m_vsTable;

public:
    Field();

    ~Field();

    // 是否是数字
    bool IsNum(int num);

    // 是否是数字
    bool IsNum(string num);

    // 是否是日期
    bool IsDate(int num);

    // 是否是日期
    bool IsDate(string num);

    // 是否是字符串
    bool IsChar(int num);

    // 是否是字符串
    bool IsChar(string num);

    // 是否为二进制数据
    bool IsBlob(int num);

    // 是否为二进制数据
    bool IsBlob(string num);

    // 得到指定字段的序号
    int GetFieldNo(string fieldName);
};

/*
 * 1. 单条记录
 * 2. 重载了[]操作
 */
class Record {
public:
    // 结果集
    vector<string> m_rs;
    // 字段信息 占用4字节的内存 当记录数很大是会产生性能问题
    Field *m_field;

public:
    Record() {}

    Record(Field *m_f);

    ~Record();

    // 结果集中添加数据
    void SetData(string value);

    // []操作
    string operator[](string s);

    string operator[](int num);

    // null值的判断
    bool IsNull(int num);

    bool IsNull(string s);

    // 用 value<tab>value的形式返回结果
    string GetTabText();
};

/*
 * 1. 记录集合
 * 2. 重载了[]操作
 * 3. 表结构操作
 * 4. 数据的插入和修改
 */
class RecordSet {
private:
    // 记录集
    vector<Record> m_vcSet;
    // 游标位置
    unsigned long m_ulPos;
    // 记录数
    int m_nRecordCount;
    // 字段数
    int m_nFieldNum;
    // 字段信息
    Field m_cField;

    MYSQL_RES *m_pMysqlRes;
    MYSQL_FIELD *m_pMysqlField;
    MYSQL_ROW *m_pMysqlRow;
    MYSQL *m_pMysql;

public:
    RecordSet();

    RecordSet(MYSQ *hSQL);

    ~RecordSet();

    // 返回指定序号的记录
    Record operator[](int num);

    // 处理返回多行的查询，返回影响的行数
    int ExecuteSql(string SQL);

    // 得到记录数目
    int GetRecordCount();

    // 得到字段数目
    int GetFieldNum();

    // 向后移动游标
    long MoveNext();

    // 移动游标
    long Move(long length);

    // 移动游标到开始位置
    bool MoveFirst();

    // 移动游标到结束位置
    bool MoveLast();

    // 获取当前游标位置
    unsigned long GetCurrentPos() const;

    // 获取当前游标的对应字段数据
    string GetCurrentFieldValue(string sFieldName);

    string GetCurrentFieldValue(int nFieldNum);

    // 获取游标的对应字段数据
    bool GetFieldValue(long index, const char *szFieldName, char *szValue);

    bool GetFieldValue(long index, int nFieldNum, char *szValue);

    // 是否到达游标尾部
    bool IsEof();

    // 返回字段
    Field *GetField();

    // 返回字段名
    const char *GetFieldName(int nNum);

    // 返回字段类型
    const int GetFieldType(char *szName);

    const int GetFieldType(int nNum);

};

/*
 * 1. 负责数据库的连接关闭
 * 2. 执行sql语句(不返回结果)
 * 3. 处理事务
 */
class DataBase {
public:
    DataBase();

    ~DataBase();

private:
    // mysql连接句柄
    MYSQL *m_pMysql;

public:
    // 返回句柄
    MYSQL *GetMysqlHandle();

    // 连接数据库
    int Connet(string strHost, string strUser,
               string strPasswd, string strDb,
               unsigned int unPort,
               unsigned int unConnectTimeout,
               unsigned int unReadTimeout,
               unsigned int unWriteTimeout,
               unsigned long ulClientFlag);

    // 关闭数据库连接
    void Disconnect();
    // 测试mysql服务器是否存活
    int Ping();
    // 关闭mysql服务器
    int ShutDown();
    // 重新启动mysql服务器
    int Reboot();
    // 获取客户端信息
    const char *GetClientInfo();
    // 获取客户端版本
    const unsigned long GetClientVersion();
    // 获取主机信息
    const char *GetHostInfo();
    // 获取服务器信息
    const char *GetServerInfo();
    // 获取服务器版本号
    const unsigned long GetServerVersion();
    // 获取当前连接的默认字符集
    const char *GetCharacterSetName();
    // 获取系统时间
    const char *GetSysTime();

    /*
     * 说明：以下是事务相关功能
     *      事务支持InnoDB or BDB表类型
     */
    // 开始事务
    int StartTransaction();
    // 提交事务
    int Commit();
    // 回滚事务
    int Rollback();

    // 执行非返回结果查询
    int ExecNonQuery(string sql);
    // 返回单值查询
    char *ExecQueryGetSingValue(string sql);
    // 建立新数据库
    int CreateDb(string name);
    // 删除指定的数据库
    int DropDb(string name);
};

#endif //__ZC_MYSQLAPI_H__
