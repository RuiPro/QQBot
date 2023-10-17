#include "sqlite_client.h"

bool SQLiteQueryResult::nextRow() {
	if (m_stmt != nullptr) {
		if (sqlite3_step(m_stmt) == SQLITE_ROW) {
			return true;
		}
	}
	return false;
}
string SQLiteQueryResult::rowValue(unsigned int index) {
	if (index >= m_column_count) {
		return string();
	}
	const char* val = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, index));
	if (val == nullptr) return string();
	return string(val, sqlite3_column_bytes(m_stmt, index));
}
void SQLiteQueryResult::clear() {
	m_column_count = 0;
	m_column_name.clear();
}
int SQLiteQueryResult::column_type(unsigned int index) {
	if (index >= m_column_count) {
		return -1;
	}
	return sqlite3_column_type(m_stmt, index);
}

string SQL::getStr() const {
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

SQLiteClient::SQLiteClient() {
	query_result = new SQLiteQueryResult;
}
SQLiteClient::~SQLiteClient() {
	if (m_db != nullptr) {
		sqlite3_close(m_db);
	}
	query_result->clear();
	if (query_result != nullptr) {
		if (query_result->m_stmt != nullptr) {
			sqlite3_finalize(query_result->m_stmt);
		}
		delete query_result;
		query_result = nullptr;
	}
}
bool SQLiteClient::opendb(const string& dbName, int flag /* = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE*/) {
	int ret = sqlite3_open_v2(dbName.c_str(), &m_db, flag, NULL);
	return !ret;
}
int sqlite_exec_callback(void* arg, int column, char** data_set, char** column_set) {
	return 0;
}
bool SQLiteClient::update(const string& sql) {
	if (sqlite3_exec(m_db, sql.c_str(), sqlite_exec_callback, nullptr, nullptr) == SQLITE_OK) {
		return true;
	}
	return false;
}
bool SQLiteClient::update(const SQL& sql) {
	return update(sql.getStr());
}
bool SQLiteClient::query(const string& sql) {
	query_result->clear();		// 释放上一次的查询结果
	int ret = sqlite3_prepare_v2(m_db, sql.c_str(), sql.size(), &(query_result->m_stmt), nullptr);
	if (ret != SQLITE_OK) return false;
	query_result->m_column_count = sqlite3_column_count(query_result->m_stmt);
	for (unsigned int i = 0; i < query_result->m_column_count; ++i) {
		query_result->m_column_name.push_back(string(sqlite3_column_name(query_result->m_stmt, i)));
	}
	return true;
}
bool SQLiteClient::query(const SQL& sql) {
	return query(sql.getStr());
}
bool SQLiteClient::transaction() {
	m_transaction_lock.lock();
	int ret = sqlite3_exec(m_db, "BEGIN;", nullptr, nullptr, nullptr);
	if (ret != SQLITE_OK) {
		return true;
	}
	m_transaction_lock.unlock();
	return false;
}
bool SQLiteClient::commit() {
	int ret = sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, nullptr);
	if (ret != SQLITE_OK) {
		m_transaction_lock.unlock();
		return true;
	}
	m_transaction_lock.unlock();
	return false;
}
bool SQLiteClient::rollback() {
	int ret = sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
	if (ret != SQLITE_OK) {
		m_transaction_lock.unlock();
		return true;
	}
	m_transaction_lock.unlock();
	return false;
}
string SQLiteClient::errmsg() {
	return string(sqlite3_errmsg(m_db));
}
bool SQLiteClient::hasTable(const string& table_name) {
	string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='";
	sql.append(table_name);
	sql.append("' ORDER BY name");
	if (!query(sql)) {
		return false;
	}
	while (this->query_result->nextRow()) {
		if (table_name == this->query_result->rowValue(0)) return true;
	}
	return false;
}
string SQLiteClient::sqliteVersion() {
	return string(sqlite3_version);
}
void SQLiteClient::resetIdleTime() {
	m_alivetime = steady_clock::now();
}
long long SQLiteClient::getIdleTime() {
	nanoseconds res = steady_clock::now() - m_alivetime;
	milliseconds millsec = duration_cast<milliseconds>(res);
	return millsec.count();
}