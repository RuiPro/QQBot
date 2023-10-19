#ifndef SQLITE_CLIENT_H
#define SQLITE_CLIENT_H

#include <sqlite3.h>
#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <sstream>
#include "all2str.h"
using namespace std;
using namespace chrono;

#include "format.h"
#include <json.hpp>
using json = nlohmann::json;

class SQL : public Format {
public:
	SQL(const string& str) : Format(str) {}
	SQL(string&& str) : Format(std::move(str)) {}
	SQL(const char* str_arr) : Format(std::move(string(str_arr))) {}
	~SQL() {}

	template<class T>
	void arg(int index, const T& t) {
		string str = all2str(t);
		string sql_arg;
		for (auto& element : str) {
			if (element == '\'') {
				sql_arg.push_back('\'');
			}
			sql_arg.push_back(element);
		}
		m_arg_set.insert(std::move(std::pair<int, string>(index, sql_arg)));
	}
};

struct SQLiteQueryResult {
	sqlite3_stmt* m_stmt = nullptr;
	unsigned int m_column_count = 0;				// 字段数
	vector<string> m_column_name;					// 字段数组

	// 遍历结果集
    bool nextRow();
	// 从结果集的某一行中取出某个值
    string rowValue(unsigned int index);
	// 清除结果集
    void clear();
	// 获取值类型
    int column_type(unsigned int index);
};

class SQLiteClient {
public:
	SQLiteClient();
	~SQLiteClient();
	// 打开数据库
	bool opendb(const string& dbName, int flag = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	// 更新数据库: insert, update, delete
	bool update(const string& sql);
	bool update(const char* sql);
	bool update(const SQL& sql);
	// 查询数据库
	bool query(const string& sql);
	bool query(const char* sql);
	bool query(const SQL& sql);
	// 事务操作
	bool transaction();
	// 提交事务
	bool commit();
	// 事务回滚 
	bool rollback();
	// 获取错误信息
	string errmsg();

	// 判断是否含有数据表
	bool hasTable(const string& table_name);

    static string sqliteVersion();

	// 用于数据库连接池
	// 刷新起始的空闲时间点
	void resetIdleTime();
	// 计算连接空闲的时长
	long long getIdleTime();

	// 查询的结果集
	SQLiteQueryResult* query_result = nullptr;
private:
	sqlite3* m_db = nullptr;
	// 事务锁：防止一个连接被多线程调用时发生事务冲突
	// 在开始事务成功时上锁，提交或回滚时解锁；
	// 如果需要并发事务，请建立多个连接
	mutex m_transaction_lock;					
	steady_clock::time_point m_alivetime;
};

#endif  // !SQLITE_CLIENT_H