#ifndef SQLITE_CLIENT_H
#define SQLITE_CLIENT_H

#include <sqlite3.h>
#include <string>
#include <chrono>
#include <vector>
#include <mutex>
#include <json.hpp>
using json = nlohmann::json;
using namespace std;
using namespace chrono;

struct SQLiteQueryResult {
	sqlite3_stmt* m_stmt = nullptr;
	unsigned int m_column_count = 0;				// 字段数
	vector<string> m_column_name;					// 字段数组

	// 遍历结果集
	bool nextRow() {
		if (m_stmt != nullptr) {
			if (sqlite3_step(m_stmt) == SQLITE_ROW) {
				return true;
			}
		}
		return false;
	}
	// 从结果集的某一行中取出某个值
	string rowValue(unsigned int index) {
		if (index >= m_column_count) {
			return string();
		}
		const char* val = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, index));
		if (val == nullptr) return string();
		return string(val, sqlite3_column_bytes(m_stmt, index));
	}
	// 清除结果集
	void clear() {
		m_column_count = 0;
		m_column_name.clear();
	}
	// 获取值类型
	int column_type(unsigned int index) {
		if (index >= m_column_count) {
			return -1;
		}
		return sqlite3_column_type(m_stmt, index);
	}
};

class SQL {
public:
    inline SQL() {
        m_arg_set = new string[50];
    }

    inline explicit SQL(const string& str) : m_sql_str(str) {
        m_arg_set = new string[50];
    }

    inline explicit SQL(const char* char_arr) : m_sql_str(char_arr) {
        m_arg_set = new string[50];
    }

    inline explicit SQL(string&& str) : m_sql_str(std::move(str)) {
        m_arg_set = new string[50];
    }

    ~SQL() {
        delete[] m_arg_set;
    }

    SQL& operator=(const string& str) {
        m_sql_str = str;
        return *this;
    }

    SQL& operator=(const char* char_arr) {
        m_sql_str = char_arr;
        return *this;
    }

    string getStr() const {
        string ret;
        for (int i = 0; i < m_sql_str.size(); ++i) {
            if (m_sql_str[i] != '%') {
                ret.push_back(m_sql_str[i]);
                continue;
            }
            // 取出%后面的两个数字
            int number1 = -1, number2 = -1;
            // 第一位必须为1~9
            if (m_sql_str[i + 1] > 48 && m_sql_str[i + 1] < 58) {
                number1 = m_sql_str[i + 1] - 48;
            }
            else {
                ret.push_back(m_sql_str[i]);
                continue;
            }
            // 第二位
            if (m_sql_str[i + 2] >= 48 && m_sql_str[i + 2] < 58) {
                number2 = m_sql_str[i + 2] - 48;
            }
            // 取值
            // 如果两位数都合法且已经确定了参数
            if (number2 != -1 && (number1 * 10 + number2 - 1) < 50 && !m_arg_set[number1 * 10 + number2 - 1].empty()) {
                ret.append(m_arg_set[number1 * 10 + number2 - 1]);
                i += 2;
            }
            // 如果只有第一位数合法且已经确定了参数
            else if (number1 != -1 && !m_arg_set[number1 - 1].empty()) {
                ret.append(m_arg_set[number1 - 1]);
                i += 1;
            }
            else {
                ret.push_back(m_sql_str[i]);
                continue;
            }
        }
        return ret;
    }
    // 代替语句中的占位符source->target
    // transfer：是否使用转义，将SQL语句中的'转义成''
    void args(int index, const string& target, bool transfer = true) {
        if (index <= 0 || index > 50) return;
        string target_edit;
        if (transfer) {
            for (auto& element : target) {
                if (element == '\'') {
                    m_arg_set[index - 1].push_back('\'');
                }
                m_arg_set[index - 1].push_back(element);
            }
        }
        else {
            m_arg_set[index - 1] = std::move(target);
        }
    }
    void args(int index, json json_object, bool transfer = true) {
        args(index, json_object.dump(), transfer);
    }
    void args(int index, const char* str_arr, bool transfer = true) {
        args(index, string(str_arr), transfer);
    }
private:
    string m_sql_str;
    string* m_arg_set;
};

class SQLiteClient {
public:
	SQLiteClient();
	~SQLiteClient();
	// 打开数据库
	bool opendb(const string& dbName, int flag = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	// 更新数据库: insert, update, delete
	bool update(const string& sql);
	bool update(const SQL& sql);
	// 查询数据库
	bool query(const string& sql);
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