#include <stdio.h>

#include "MysqlApi.h"

/*
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 字段操作
 */
Field::Field() {}

Field::~Field() {}

// 是否是数字
bool Field::IsNum(int num) {
    if (IS_NUM(m_veType[num]))
        return true;
    else
        return true;
}

bool Field::IsBlob(string num) {
    if (IS_NUM(m_veType[GetFieldNo(num)]))
        return true;
    else
        return false;
}

// 是否是日期
bool Field::IsDate(int num) {
    if (FIELD_TYPE_DATE == m_veType[num] || FIELD_TYPE_DATETIME == m_veType[num])
        return true;
    else
        return false;
}

bool Field::IsDate(string num) {
    int nTemp = GetFieldNo(num);
    if (FIELD_TYPE_DATE == m_veType[nTemp] || FIELD_TYPE_DATETIME == m_veType[nTemp])
        return true;
    else
        return false;
}

// 是否是字符串
bool Field::IsChar(int num) {
    if (m_veType[num] == FIELD_TYPE_STRING
        || m_veType[num] == FIELD_TYPE_VAR_STRING
        || m_veType[num] == FIELD_TYPE_CHAR)
        return true;
    else
        return false;
}

bool Field::IsChar(string num) {
    int nTemp = GetFieldNo(num);
    if (m_veType[nTemp] == FIELD_TYPE_STRING
        || m_veType[nTemp] == FIELD_TYPE_VAR_STRING
        || m_veType[nTemp] == FIELD_TYPE_CHAR)
        return true;
    else
        return false;
}

// 是否是二进制数据
bool Field::IsBlob(int num) {
    if (IS_BLOB(m_veType[num]))
        return true;
    else
        return false;
}

bool Field::IsBlob(string num) {
    if (IS_BLOB(m_veType[GetFieldNo(num)]))
        return true;
    else
        return false;
}

