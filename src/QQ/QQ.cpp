#include <curl.h>
#include <chrono>
#include "QQ_sql.h"
#include "../SQLiteClient/sqlite_client.h"
#include "../Loger/loger.hpp"
#include "QQ.h"
#include <json.hpp>
using json = nlohmann::json;

ThisBot* ThisBot::sm_bot;

// curl 回调函数
size_t curl_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	((string*)userdata)->append(ptr, size * nmemb);
	return size * nmemb;
}

ThisBot::ThisBot(const string& cqhttp_addr, const string& cqhttp_access_token, bool m_cqhttp_use_cache) :
	m_cqhttp_addr(cqhttp_addr),
	m_cqhttp_access_token(cqhttp_access_token),
	m_cqhttp_use_cache(m_cqhttp_use_cache) {
	sqlite_c = new SQLiteClient;
	if (!sqlite_c->opendb(":memory:")) {
		loger.error() << "Failed to create SQLite database: " << sqlite_c->errmsg();
		exit(0);
	}
	if (!sqlite_c->update(create_table_sql)) {
		loger.error() << "Failed to create SQLite tables: " << sqlite_c->errmsg();
		exit(0);
	}
}
ThisBot::~ThisBot() {
	if (sqlite_c != nullptr) {
		delete sqlite_c;
		sqlite_c = nullptr;
	}
}

// 【Get】这些函数用以获取Bot中已设置的信息
unsigned int ThisBot::getThisBotID() const {
	unsigned int ret = m_id;
	return ret;
}
string ThisBot::getThisBotNickname() const {
	string ret = m_nickname;
	return ret;
}
bool ThisBot::getThisBotHasAdmin() const {
	bool ret = true;
	if (m_administrator_id == 0) ret = false;
	return ret;
}
QQFriend ThisBot::getThisBotAdmin() const {
	QQFriend ret = m_administrator_id;
	return ret;
}
int ThisBot::getThisBotFriendNum() const {
	int ret = 0;
	string SQL("SELECT COUNT(*) FROM friend_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	sqlite_c->query_result->nextRow();
	ret = stoi(sqlite_c->query_result->rowValue(0));
	return ret;
}
int ThisBot::getThisBotUFriendNum() const {
	int ret = 0;
	string SQL("SELECT COUNT(*) FROM ufriend_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	sqlite_c->query_result->nextRow();
	ret = stoi(sqlite_c->query_result->rowValue(0));
	return ret;
}
int ThisBot::getThisBotGroupNum() const {
	int ret = 0;
	string SQL("SELECT COUNT(*) FROM group_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	sqlite_c->query_result->nextRow();
	ret = stoi(sqlite_c->query_result->rowValue(0));
	return ret;
}
int ThisBot::getGroupMemberNum(unsigned int group_id) const {
	int ret = 0;
	string SQL("SELECT COUNT(*) FROM group_member_list WHERE group_id=");
	SQL.append(to_string(group_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	sqlite_c->query_result->nextRow();
	ret = stoi(sqlite_c->query_result->rowValue(0));
	return ret;
}
bool ThisBot::getThisBotHasFriend(unsigned int friend_id) const {
	string SQL("SELECT * FROM friend_list WHERE user_id=");
	SQL.append(to_string(friend_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	return sqlite_c->query_result->nextRow();
}
bool ThisBot::getThisBotHasUFriend(unsigned int ufriend_id) const {
	string SQL("SELECT * FROM ufriend_list WHERE user_id=");
	SQL.append(to_string(ufriend_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	return sqlite_c->query_result->nextRow();
}
bool ThisBot::getThisBotHasGroup(unsigned int group_id) const {
	string SQL("SELECT * FROM group_list WHERE group_id=");
	SQL.append(to_string(group_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	return sqlite_c->query_result->nextRow();
}
bool ThisBot::getGroupHasMember(unsigned int group_id, unsigned int member_id) const {
	string SQL("SELECT * FROM group_member_list WHERE group_id=");
	SQL.append(to_string(group_id));
	SQL.append(" AND user_id=");
	SQL.append(to_string(member_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	return sqlite_c->query_result->nextRow();
}
QQFriend ThisBot::getThisBotFriend(unsigned int friend_id) const {
	QQFriend f(0);
	string SQL("SELECT * FROM friend_list WHERE user_id=");
	SQL.append(to_string(friend_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	if (!sqlite_c->query_result->nextRow()) return f;
	f.m_id = friend_id;
	f.m_name = sqlite_c->query_result->rowValue(1);
	f.m_friend_remark = sqlite_c->query_result->rowValue(2);
	return f;
}
QQUFriend ThisBot::getThisBotUFriend(unsigned int friend_id) const {
	QQUFriend uf(0);
	string SQL("SELECT * FROM ufriend_list WHERE user_id=");
	SQL.append(to_string(friend_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	if (!sqlite_c->query_result->nextRow()) return uf;
	uf.m_id = friend_id;
	uf.m_name = sqlite_c->query_result->rowValue(1);
	uf.m_source = sqlite_c->query_result->rowValue(2);
	return uf;
}
QQGroup ThisBot::getThisBotGroup(unsigned int group_id) const {
	QQGroup g(0);
	string SQL("SELECT * FROM group_list WHERE group_id=");
	SQL.append(to_string(group_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	if (!sqlite_c->query_result->nextRow()) return g;
	g.m_id = group_id;
	g.m_name = sqlite_c->query_result->rowValue(1);
	g.m_member_count = stoi(sqlite_c->query_result->rowValue(2).empty() ? "0" : sqlite_c->query_result->rowValue(2));
	g.m_max_member_count = stoi(sqlite_c->query_result->rowValue(3).empty() ? "0" : sqlite_c->query_result->rowValue(3));
	g.m_group_create_time = static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(4)));
	g.m_group_level = stoi(sqlite_c->query_result->rowValue(5).empty() ? "0" : sqlite_c->query_result->rowValue(5));
	g.m_group_remark = sqlite_c->query_result->rowValue(6);
	return g;
}
QQGroupMember ThisBot::getGroupMember(unsigned int group_id, unsigned int member_id) const {
	QQGroupMember gm(0);
	string SQL("SELECT * FROM group_member_list WHERE group_id=");
	SQL.append(to_string(group_id));
	SQL.append(" AND user_id=");
	SQL.append(to_string(member_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	if (!sqlite_c->query_result->nextRow()) return gm;
	gm.m_id = member_id;
	gm.m_group_id = group_id;
	for (int i = 0; i < 16; ++i) {
		loger.debug() << ">>>>>" << i << ">>>>> " << sqlite_c->query_result->rowValue(i);
	}

	gm.m_name = sqlite_c->query_result->rowValue(2);
	gm.m_user_age = stoi(sqlite_c->query_result->rowValue(3).empty() ? "0" : sqlite_c->query_result->rowValue(3));
	gm.m_user_area = sqlite_c->query_result->rowValue(4);
	gm.m_user_gender = stoi(sqlite_c->query_result->rowValue(5).empty() ? "0" : sqlite_c->query_result->rowValue(5));
	gm.m_group_nickname = sqlite_c->query_result->rowValue(6);
	gm.m_group_nickname_changeable = sqlite_c->query_result->rowValue(7) == "0" ? false : true;
	gm.m_group_join_time = static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(8)));
	gm.m_group_last_active_time = static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(9).empty() ? "0" : sqlite_c->query_result->rowValue(9)));
	gm.m_group_level = sqlite_c->query_result->rowValue(10);
	loger.debug() << "===1===";
	switch (stoi(sqlite_c->query_result->rowValue(11).empty() ? "0" : sqlite_c->query_result->rowValue(11))) {
	case 0:
		gm.m_group_role = QQGroupRole::member;
		break;
	case 1:
		gm.m_group_role = QQGroupRole::administrator;
		break;
	case 2:
		gm.m_group_role = QQGroupRole::owner;
		break;
	}
	loger.debug() << "===2===";
	gm.m_group_mute_time = static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(12)));
	gm.m_group_title = sqlite_c->query_result->rowValue(13);
	gm.m_group_title_expire_time = static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(14)));
	gm.m_group_is_unfriendly = sqlite_c->query_result->rowValue(15) == "0" ? false : true;
	return gm;
}
vector<unsigned int> ThisBot::getThisBotFriendIDList() const {
	vector<unsigned int> ret;
	string SQL("SELECT user_id FROM friend_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	while (sqlite_c->query_result->nextRow()) {
		ret.push_back(static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(0))));
	}
	return ret;
}
vector<unsigned int> ThisBot::getThisBotUFriendIDList() const {
	vector<unsigned int> ret;
	string SQL("SELECT user_id FROM ufriend_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	while (sqlite_c->query_result->nextRow()) {
		ret.push_back(static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(0))));
	}
	return ret;
}
vector<unsigned int> ThisBot::getThisBotGroupIDList() const {
	vector<unsigned int> ret;
	string SQL("SELECT group_id FROM group_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	while (sqlite_c->query_result->nextRow()) {
		ret.push_back(static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(0))));
	}
	return ret;
}
vector<unsigned int> ThisBot::getGroupMemberIDList(unsigned int group_id) const {
	vector<unsigned int> ret;
	string SQL("SELECT user_id FROM group_member_list WHERE group_id=");
	SQL.append(to_string(group_id));
	SQL.append(";");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	while (sqlite_c->query_result->nextRow()) {
		ret.push_back(static_cast<unsigned int>(stol(sqlite_c->query_result->rowValue(0))));
	}
	return ret;
}
void ThisBot::printFriendList() const {
	string SQL("SELECT * FROM friend_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	loger.info() << " ┌─ \033[34mQQ friends list\033[0m";
	while (sqlite_c->query_result->nextRow()) {
		loger.info() << " ├─ " << sqlite_c->query_result->rowValue(1) << "(" << sqlite_c->query_result->rowValue(0) << ")";
	}
	loger.info() << " └─ \033[34mTotal num: " << getThisBotFriendNum() << "\033[0m";
}
void ThisBot::printUFriendList() const {
	string SQL("SELECT * FROM ufriend_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	loger.info() << " ┌─ \033[34mQQ unidirectional friends list\033[0m";
	while (sqlite_c->query_result->nextRow()) {
		loger.info() << " ├─ " << sqlite_c->query_result->rowValue(1) << "(" << sqlite_c->query_result->rowValue(0) << ")";
	}
	loger.info() << " └─ \033[34mTotal num: " << getThisBotUFriendNum() << "\033[0m";
} 
void ThisBot::printGroupList() const {
	string SQL("SELECT * FROM group_list;");
	if (!sqlite_c->query(SQL)) {
		loger.error() << "In funtion " << __FUNCTION__ << " SQLite query error: " << sqlite_c->errmsg();
	}
	loger.info() << " ┌─ \033[34mQQ groups list\033[0m";
	while (sqlite_c->query_result->nextRow()) {
		loger.info() << " ├─ " << sqlite_c->query_result->rowValue(1) << "(" << sqlite_c->query_result->rowValue(0) << ")";
	}
	loger.info() << " └─ \033[34mTotal num: " << getThisBotGroupNum() << "\033[0m";
}

// 【Set】这些函数用以手动设置Bot的某些信息
int ThisBot::setThisBotAdmin(unsigned int admin_id) {
	if (admin_id == m_id) {
		return 0;
	}
	if (getThisBotHasFriend(admin_id) >= 0) {
		m_administrator_id = admin_id;
	}
	else {
		loger.error() << "The administrator is not a friend of QQ bot, setting failed.";
		return -1;
	}
	return 0;
}
void ThisBot::setCqhttpAddr(const string& addr) {
	ThisBot::m_cqhttp_addr = addr;
}
void ThisBot::setUseCqhttpCache(bool flag) {
	ThisBot::m_cqhttp_use_cache = flag;
}
void ThisBot::setCqhttpAccessToken(const string& token) {
	ThisBot::m_cqhttp_access_token = token;
}

// 【Fetch】这些函数用以从go-cqhttp获取Bot的信息并保存或更新到Bot的成员变量内，内部都使用了互斥锁同步。
int ThisBot::fetchThisBotBasicInfo() {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_login_info";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			m_id = json_data["data"]["user_id"];
			m_nickname = json_data["data"]["nickname"];
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Exception in function " << __FUNCTION__ << ": " << e.what();
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::fetchThisBotFriendList() {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_friend_list";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			sqlite_c->transaction();
			bool flag = false;
			flag |= !sqlite_c->update("DELETE FROM friend_list;");
			for (auto& element : json_data["data"]) {
				SQL sql("INSERT INTO friend_list VALUES (%1,'%2','%3');");
				sql.args(1, element["user_id"]);
				sql.args(2, element["nickname"]);
				sql.args(3, element["remark"]);
				flag |= !sqlite_c->update(sql);
			}
			if (flag) {
				loger.warn() << "SQLite rollback in function fetchThisBotFriendList.";
				sqlite_c->rollback();
			}
			else {
				sqlite_c->commit();
				if (!getThisBotHasFriend(m_administrator_id)) {
					setThisBotAdmin(0);
				}
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Exception in function " << __FUNCTION__ << ": " << e.what();
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::fetchThisBotUFriendList() {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_unidirectional_friend_list";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			sqlite_c->transaction();
			bool flag = false;
			flag |= !sqlite_c->update("DELETE FROM ufriend_list;");
			for (auto& element : json_data["data"]) {
				SQL sql("INSERT INTO ufriend_list VALUES (%1,'%2','%3');");
				sql.args(1, element["user_id"]);
				sql.args(2, element["nickname"]);
				sql.args(3, element["source"]);
				flag |= !sqlite_c->update(sql);
			}
			if (flag) {
				loger.warn() << "SQLite rollback in function fetchThisBotUFriendList.";
				sqlite_c->rollback();
			}
			else {
				sqlite_c->commit();
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Exception in function " << __FUNCTION__ << ": " << e.what();
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::fetchThisBotGroupList() {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_group_list";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			sqlite_c->transaction();
			bool flag = false;
			flag |= !sqlite_c->update("DELETE FROM group_list;");
			flag |= !sqlite_c->update("DELETE FROM group_member_list;");
			for (auto& element : json_data["data"]) {
				SQL sql("INSERT INTO group_list VALUES (%1,'%2',%3,%4,%5,%6,'%7');");
				sql.args(1, element["group_id"]);
				sql.args(2, element["group_name"]);
				sql.args(3, element["member_count"]);
				sql.args(4, element["max_member_count"]);
				sql.args(5, element["group_create_time"]);
				sql.args(6, element["group_level"]);
				sql.args(7, element["group_memo"]);
				flag |= !sqlite_c->update(sql);
			}
			if (flag) {
				loger.warn() << "SQLite rollback in function fetchThisBotGroupList.";
				sqlite_c->rollback();
			}
			else {
				sqlite_c->commit();
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Exception in function " << __FUNCTION__ << ": " << e.what();
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::fetchThisBotGroupMemberList(unsigned int group_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_group_member_list";
		string data_buffer;
		json post_data;
		post_data["group_id"] = group_id;
		post_data["no_cache"] = !m_cqhttp_use_cache;
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			sqlite_c->transaction();
			bool flag = false;
			flag |= !sqlite_c->update("DELETE FROM group_member_list WHERE group_id=" + to_string(group_id) + ";");
			for (auto& element : json_data["data"]) {
				SQL sql("INSERT INTO group_member_list VALUES (%1,%2,'%3',%4,'%5',%6,'%7',%8,%9,%10,'%11',%12,%13,'%14',15,%16);");
				sql.args(1, element["user_id"]);
				sql.args(2, element["group_id"]);
				sql.args(3, element["nickname"]);
				sql.args(4, element["age"]);
				sql.args(5, element["area"]);
				if (element["sex"] == "unknown") {
					sql.args(6, "0");
				}
				else if (element["sex"] == "male") {
					sql.args(6, "1");
				}
				else if (element["sex"] == "female") {
					sql.args(6, "2");
				}
				sql.args(7, element["card"]);
				sql.args(8, element["card_changeable"] == "false" ? "0" : "1");
				sql.args(9, element["join_time"]);
				sql.args(10, element["last_sent_time"]);
				sql.args(11, element["level"]);
				if (element["role"] == "member") {
					sql.args(12, "0");
				}
				else if (element["role"] == "admin") {
					sql.args(12, "1");
				}
				else if (element["role"] == "owner") {
					sql.args(12, "2");
				}
				sql.args(13, element["shut_up_timestamp"]);
				sql.args(14, element["title"]);
				sql.args(15, element["title_expire_time"]);
				sql.args(16, element["unfriendly"] ? "1" : "0");
				flag |= !sqlite_c->update(sql);
			}
			if (flag) {
				loger.warn() << "SQLite rollback in function fetchThisBotGroupMemberList.";
				sqlite_c->rollback();
			}
			else {
				sqlite_c->commit();
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Exception in function " << __FUNCTION__ << ": " << e.what();
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::fetchThisBotGroupMemberInfo(unsigned int group_id, unsigned int member_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_group_member_info";
		string data_buffer;
		json post_data;
		post_data["group_id"] = group_id;
		post_data["user_id"] = member_id;
		post_data["no_cache"] = !m_cqhttp_use_cache;
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			sqlite_c->transaction();
			bool flag = false;
			flag |= !sqlite_c->update("DELETE FROM group_member_list WHERE group_id=" 
				+ to_string(group_id) 
				+ " AND user_id="
				+ to_string(member_id)
				+ ";");
			SQL sql("INSERT INTO group_member_list VALUES (%1,%2,'%3',%4,'%5',%6,'%7',%8,%9,10,'%11',%12,%13,'%14',%15,%16);");
			sql.args(1, json_data["data"]["user_id"]);
			sql.args(2, json_data["data"]["group_id"]);
			sql.args(3, json_data["data"]["nickname"]);
			sql.args(4, json_data["data"]["age"]);
			sql.args(5, json_data["data"]["area"]);
			if (json_data["data"]["sex"] == "unknown") {
				sql.args(6, "0");
			}
			else if (json_data["data"]["sex"] == "male") {
				sql.args(6, "1");
			}
			else if (json_data["data"]["sex"] == "female") {
				sql.args(6, "2");
			}
			sql.args(7, json_data["data"]["card"]);
			sql.args(8, json_data["data"]["card_changeable"] == "false" ? "0" : "1");
			sql.args(9, json_data["data"]["join_time"]);
			sql.args(10, json_data["data"]["last_send_time"]);
			sql.args(11, json_data["data"]["level"]);
			if (json_data["data"]["user_id"] == "member") {
				sql.args(12, "0");
			}
			else if (json_data["data"]["user_id"] == "admin") {
				sql.args(12, "1");
			}
			else if (json_data["data"]["user_id"] == "owner") {
				sql.args(12, "2");
			}
			sql.args(13, json_data["data"]["shut_up_timestamp"]);
			sql.args(14, json_data["data"]["title"]);
			sql.args(15, json_data["data"]["title_expire_time"]);
			sql.args(16, json_data["data"]["unfriendly"] == "false" ? "0" : "1");
			flag |= !sqlite_c->update(sql);
			if (flag) {
				loger.warn() << "SQLite rollback in function fetchThisBotGroupMemberInfo.";
				sqlite_c->rollback();
			}
			else {
				sqlite_c->commit();
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Exception in function " << __FUNCTION__ << ": " << e.what();
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}

// 【query】这些函数用以从go-cqhttp获取其他信息
QQGroup ThisBot::queryGroupInfo(unsigned int group_id) {
	try {
		if (m_cqhttp_addr.empty()) return QQGroup(0);
		string URL = "http://" + m_cqhttp_addr + "/get_group_info";
		string data_buffer;
		json post_data;
		post_data["group_id"] = group_id;
		post_data["no_cache"] = !m_cqhttp_use_cache;
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			QQGroup g(group_id);
			g.m_name = json_data["data"]["group_name"];
			g.m_member_count = json_data["data"]["member_count"];
			g.m_max_member_count = json_data["data"]["max_member_count"];
			g.m_group_create_time = json_data["data"]["group_create_time"];
			g.m_group_level = json_data["data"]["group_level"];
			g.m_group_remark = json_data["data"]["group_memo"];
			return g;
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return QQGroup(0);
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return QQGroup(0);
	}
}
QQUser ThisBot::queryUserInfo(unsigned int user_id) {
	try {
		if (m_cqhttp_addr.empty()) return QQUser(0);
		string URL = "http://" + m_cqhttp_addr + "/get_stranger_info";
		string data_buffer;
		json post_data;
		post_data["user_id"] = user_id;
		post_data["no_cache"] = !m_cqhttp_use_cache;
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			QQUser user(user_id);
			user.m_name = json_data["data"]["nickname"];
			user.m_age = json_data["data"]["age"];
			if (json_data["data"]["sex"] == "unknown") user.m_gender = 0;
			else if (json_data["data"]["sex"] == "male") user.m_gender = 1;
			else if (json_data["data"]["sex"] == "female") user.m_gender = 2;
			user.m_qid = json_data["data"]["qid"];
			user.m_level = json_data["data"]["level"];
			user.m_login_days = json_data["data"]["login_days"];
			return user;
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return QQUser(0);
		}
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return QQUser(0);
	}
}
QQRawMessage ThisBot::queryMessageInfo(int message_id) {
	try {
		if (m_cqhttp_addr.empty()) return QQRawMessage();
		string URL = "http://" + m_cqhttp_addr + "/get_msg";
		string data_buffer;
		json post_data;
		post_data["message_id"] = message_id;
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return QQRawMessage();
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			QQRawMessage msg;
			msg.m_message_id = json_data["data"]["message_id"];
			msg.m_is_group_msg = json_data["data"]["group"];
			if (msg.m_is_group_msg) {
				msg.m_group_id = json_data["data"]["group_id"];
			}
			msg.m_real_id = json_data["data"]["real_id"];
			msg.m_sender_id = json_data["data"]["sender"]["user_id"];
			msg.m_sender_nickname = json_data["data"]["sender"]["nickname"];
			msg.m_send_time = json_data["data"]["time"];
			msg.m_message = json_data["data"]["message"];
			msg.m_raw_message = json_data["data"]["raw_message"];
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return QQRawMessage();
		}
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return QQRawMessage();
	}
}
string ThisBot::queryCqhttpVersion() {
	try {
		if (m_cqhttp_addr.empty()) return string();
		string URL = "http://" + m_cqhttp_addr + "/get_version_info";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return string();
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			string version = json_data["data"]["version"];
			return version;
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return string();
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return string();
	}
}
bool ThisBot::queryCanSendImage() {
	try {
		if (m_cqhttp_addr.empty()) return false;
		string URL = "http://" + m_cqhttp_addr + "/can_send_image";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			return json_data["data"]["yes"];
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return false;
		}
		return true;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return false;
	}
}
bool ThisBot::queryCanSendRecord() {
	try {
		if (m_cqhttp_addr.empty()) return false;
		string URL = "http://" + m_cqhttp_addr + "/can_send_record";
		string data_buffer;
		if (sendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			return json_data["data"]["yes"];
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return false;
		}
		return true;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return false;
	}
}
vector<pair<string, bool>> ThisBot::queryDeviceShowList(const string& device_name) {
	vector<pair<string, bool>> ret_set;
	try {
		if (m_cqhttp_addr.empty()) return ret_set;
		string URL = "http://" + m_cqhttp_addr + "/_get_model_show";
		json send_json;
		send_json["model"] = device_name;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return ret_set;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			auto device_list = json_data["data"]["variants"];
			for (auto& element : device_list) {
				ret_set.push_back(make_pair<string, bool>(element["model_show"], element["need_pay"]));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
		}
		return ret_set;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return ret_set;
	}
}
vector<QQOnlineClient> ThisBot::queryOnlineClients() {
	vector<QQOnlineClient> clients;
	try {
		if (m_cqhttp_addr.empty()) return clients;
		string URL = "http://" + m_cqhttp_addr + "/get_online_clients";
		json send_json;
		send_json["no_cache"] = !m_cqhttp_use_cache;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return clients;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			auto client_list = json_data["data"]["clients"];
			for (auto& element : client_list) {
				QQOnlineClient qqoc;
				qqoc.m_app_id = element["app_id"];
				qqoc.m_device_name = element["device_name"];
				qqoc.m_device_kind = element["device_kind"];
				clients.push_back(std::move(qqoc));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return clients;
		}
		return clients;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return clients;
	}
}
vector<QQForwardMsgNode> ThisBot::queryForwardMsgContent(const string& forward_id) {
	vector<QQForwardMsgNode> forward_msg_list;
	try {
		if (m_cqhttp_addr.empty()) return forward_msg_list;
		string URL = "http://" + m_cqhttp_addr + "/get_forward_msg";
		json send_json;
		send_json["message_id"] = forward_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return forward_msg_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]["messages"]) {
				QQForwardMsgNode qfm;
				qfm.m_sender_id = element["sender"]["user_id"];
				qfm.m_sender_nickname = element["sender"]["nickname"];
				qfm.m_message = element["content"];
				qfm.m_send_time = element["time"];
				forward_msg_list.push_back(std::move(qfm));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return forward_msg_list;
		}
		return forward_msg_list;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return forward_msg_list;
	}
}
vector<QQRawMessage> ThisBot::queryGroupHistoryMsg(unsigned int group_id, unsigned int msg_seq) {
	vector<QQRawMessage> history_msg_list;
	try {
		if (m_cqhttp_addr.empty()) return history_msg_list;
		string URL = "http://" + m_cqhttp_addr + "/get_group_msg_history";
		json send_json;
		send_json["group_id"] = group_id;
		if (msg_seq != 0) {
			send_json["message_seq"] = msg_seq;
		}
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return history_msg_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]) {
				QQRawMessage msg;
				msg.m_message_id = element["message_id"];
				msg.m_is_group_msg = element["group"];
				if (msg.m_is_group_msg) {
					msg.m_group_id = element["group_id"];
				}
				msg.m_real_id = element["real_id"];
				msg.m_sender_id = element["sender"]["user_id"];
				msg.m_sender_nickname = element["sender"]["nickname"];
				msg.m_send_time = element["time"];
				msg.m_message = element["message"];
				msg.m_raw_message = element["raw_message"];
				history_msg_list.push_back(std::move(msg));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return history_msg_list;
		}
		return history_msg_list;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return history_msg_list;
	}
}
int ThisBot::queryImageInfo(const string& file, int& size, string& filename, string& url) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_image";
		json send_json;
		send_json["file"] = file;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			size = json_data["data"]["size"];
			filename = json_data["data"]["filename"];
			url = json_data["data"]["url"];
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
QQImageOCR ThisBot::queryImageOCR(const string& image_id) {
	return QQImageOCR();
}
string ThisBot::queryRecordInfo(const string& file, const string& out_format) {
	string file_addr;
	try {
		if (m_cqhttp_addr.empty()) return file_addr;
		string URL = "http://" + m_cqhttp_addr + "/get_record";
		json send_json;
		send_json["file"] = file;
		send_json["out_format"] = out_format;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return file_addr;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			file_addr = json_data["data"]["file"];
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return file_addr;
		}
		return file_addr;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return file_addr;
	}
}
QQGroupHonor ThisBot::queryGroupHonorInfo(unsigned int group_id, const string& type) {
	QQGroupHonor QGH;
	try {
		if (m_cqhttp_addr.empty()) return QGH;
		string URL = "http://" + m_cqhttp_addr + "/get_record";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["type"] = type;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return QGH;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			QGH.m_group_id = group_id;
			QGH.m_dragon_king.m_user_id = json_data["data"]["current_talkative"]["user_id"];
			QGH.m_dragon_king.m_nickname = json_data["data"]["current_talkative"]["nickname"];
			QGH.m_dragon_king.m_header_url = json_data["data"]["current_talkative"]["avatar"];
			QGH.m_dragon_king.m_day_count = json_data["data"]["current_talkative"]["day_count"];
			for (auto& element : json_data["data"]["talkative_list"]) {
				QQOtherHonor QOH;
				QOH.m_user_id = element["user_id"];
				QOH.m_nickname = element["nickname"];
				QOH.m_header_url = element["avatar"];
				QOH.m_description = element["description"];
				QGH.m_history_dragon_king.push_back(std::move(QOH));
			}
			for (auto& element : json_data["data"]["performer_list"]) {
				QQOtherHonor QOH;
				QOH.m_user_id = element["user_id"];
				QOH.m_nickname = element["nickname"];
				QOH.m_header_url = element["avatar"];
				QOH.m_description = element["description"];
				QGH.m_chat_flame.push_back(std::move(QOH));
			}
			for (auto& element : json_data["data"]["legend_list"]) {
				QQOtherHonor QOH;
				QOH.m_user_id = element["user_id"];
				QOH.m_nickname = element["nickname"];
				QOH.m_header_url = element["avatar"];
				QOH.m_description = element["description"];
				QGH.m_chat_large_flame.push_back(std::move(QOH));
			}
			for (auto& element : json_data["data"]["strong_newbie_list"]) {
				QQOtherHonor QOH;
				QOH.m_user_id = element["user_id"];
				QOH.m_nickname = element["nickname"];
				QOH.m_header_url = element["avatar"];
				QOH.m_description = element["description"];
				QGH.m_bamboo_shoot.push_back(std::move(QOH));
			}
			for (auto& element : json_data["data"]["emotion_list"]) {
				QQOtherHonor QOH;
				QOH.m_user_id = element["user_id"];
				QOH.m_nickname = element["nickname"];
				QOH.m_header_url = element["avatar"];
				QOH.m_description = element["description"];
				QGH.m_happy_sorce.push_back(std::move(QOH));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return QGH;
		}
		return QGH;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return QGH;
	}
}
QQGroupSystemMsg ThisBot::queryGroupSystemMsg(unsigned int group_id) {
	QQGroupSystemMsg QGSM;
	try {
		if (m_cqhttp_addr.empty()) return QGSM;
		string URL = "http://" + m_cqhttp_addr + "/get_group_system_msg";
		json send_json;
		send_json["group_id"] = group_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return QGSM;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]["invited_requests"]) {
				QQGroupInvitedRequest QGIR;
				QGIR.m_request_id = element["request_id"];
				QGIR.m_invitor_id = element["invitor_uin"];
				QGIR.m_invitor_nickname = element["invitor_nick"];
				QGIR.m_group_id = element["group_id"];
				QGIR.m_group_name = element["group_name"];
				QGIR.m_has_checked = element["checked"];
				QGIR.m_operator_id = element["actor"];
				QGSM.m_invitedList.push_back(std::move(QGIR));
			}
			for (auto& element : json_data["data"]["join_requests"]) {
				QQGroupJoinRequest QGJR;
				QGJR.m_request_id = element["request_id"];
				QGJR.m_applicant_id = element["requester_uin"];
				QGJR.m_applicant_nickname = element["requester_nick"];
				QGJR.m_applicant_message = element["message"];
				QGJR.m_group_id = element["group_id"];
				QGJR.m_group_name = element["group_name"];
				QGJR.m_has_checked = element["checked"];
				QGJR.m_operator_id = element["actor"];
				QGSM.m_joinList.push_back(std::move(QGJR));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return QGSM;
		}
		return QGSM;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return QGSM;
	}
}
vector<QQEssenceMsg> ThisBot::queryGroupEssenceMsg(unsigned int group_id) {
	vector<QQEssenceMsg> essence_msg_list;
	try {
		if (m_cqhttp_addr.empty()) return essence_msg_list;
		string URL = "http://" + m_cqhttp_addr + "/get_essence_msg_list";
		json send_json;
		send_json["group_id"] = group_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return essence_msg_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]) {
				QQEssenceMsg essence_msg;
				essence_msg.m_sender_id = element["sender_id"];
				essence_msg.m_sender_nickname = element["sender_nick"];
				essence_msg.m_send_time = element["sender_time"];
				essence_msg.m_operator_id = element["operator_id"];
				essence_msg.m_operator_nickname = element["operator_nick"];
				essence_msg.m_operator_settime = element["operator_time"];
				essence_msg.m_message_id = element["message_id"];
				essence_msg_list.push_back(std::move(essence_msg));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return essence_msg_list;
		}
		return essence_msg_list;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return essence_msg_list;
	}
}
int ThisBot::queryGroupatAllChance(unsigned int group_id) {
	int ret = 0;
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/get_essence_msg_list";
		json send_json;
		send_json["group_id"] = group_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			if (json_data["data"]["can_at_all"]) {
				ret = json_data["data"]["remain_at_all_count_for_uin"];
			}
			else {
				ret = -1;
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return ret;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
vector<QQGroupNotice> ThisBot::queryGroupNotice(unsigned int group_id) {
	vector<QQGroupNotice> group_notice_list;
	try {
		if (m_cqhttp_addr.empty()) return group_notice_list;
		string URL = "http://" + m_cqhttp_addr + "/_get_group_notice";
		json send_json;
		send_json["group_id"] = group_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return group_notice_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]) {
				QQGroupNotice group_notice;
				group_notice.m_sender_id = element["sender_id"];
				group_notice.m_send_time = element["publish_time"];
				group_notice.m_content = element["message"]["text"];
				group_notice.m_image_id = element["message"]["images"]["id"];
				group_notice_list.push_back(std::move(group_notice));
			}
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return group_notice_list;
		}
		return group_notice_list;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return group_notice_list;
	}
}

// 【apply】这些函数用以向go-cqhttp请求实现Bot的某些主动动作
int ThisBot::applySendPrivateMsg(unsigned int friend_id, QQMessage& msg) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		if (!msg.canSendToPrivate()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/send_private_msg";
		json send_json;
		send_json["user_id"] = friend_id;
		send_json["message"] = msg.getMsg();
		send_json["auto_escape"] = !msg.isCQMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.setMsgID(json_data["data"]["message_id"]);
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySendGroupMsg(unsigned int group_id, QQMessage& msg) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		if (!msg.canSendToGroup()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/send_group_msg";

		json send_json;
		send_json["group_id"] = group_id;
		send_json["message"] = msg.getMsg();
		send_json["auto_escape"] = !msg.isCQMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.setMsgID(json_data["data"]["message_id"]);
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySendPrivateForwardMsg(unsigned int friend_id, QQMessage& msg) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		if (!msg.canSendToPrivate()) return -1;
		if (!msg.isForwardMsg()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/send_private_forward_msg";
		json send_json;
		send_json["user_id"] = friend_id;
		send_json["message"] = msg.getMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.setMsgID(json_data["data"]["message_id"]);
			msg.setForwardMsgID(json_data["data"]["forward_id"]);
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySendGroupeForwardMsg(unsigned int group_id, QQMessage& msg) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		if (!msg.canSendToGroup()) return -1;
		if (!msg.isForwardMsg()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/send_group_forward_msg";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["message"] = msg.getMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.setMsgID(json_data["data"]["message_id"]);
			msg.setForwardMsgID(json_data["data"]["forward_id"]);
		}
		else if (json_data["status"] == "failed") {
			loger.warn() << "Failed to use go-cqhttp's API: " << URL << " ---> "
				<< json_data["msg"].get<string>() << ":" << json_data["wording"].get<string>();
			return -1;
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyRemoveFriend(unsigned int friend_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		QQFriend f = getThisBotFriend(friend_id);
		if (!f.isNull()) {
			loger.info() << "Delete friend " << f.m_name << "(" << f.m_id << ")";
			sqlite_c->transaction();
			if (sqlite_c->update("DELETE FROM friend_list WHERE user_id=" + to_string(friend_id) + ";")) {
				loger.warn() << "SQLite rollback in function applyRemoveFriend.";
				sqlite_c->rollback();
				return -1;
			}
			string URL = "http://" + m_cqhttp_addr + "/delete_friend";
			json send_json;
			send_json["user_id"] = friend_id;
			string send_buffer = send_json.dump();
			string data_buffer;
			if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) {
				loger.warn() << "SQLite rollback in function applyRemoveFriend.";
				sqlite_c->rollback();
				return -1;
			}
			sqlite_c->commit();
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyRemoveUFriend(unsigned int ufriend_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		QQUFriend uf = getThisBotUFriend(ufriend_id);
		if (!uf.isNull()) {
			loger.info() << "Delete ufriend " << uf.m_name << "(" << uf.m_id << ")";
			sqlite_c->transaction();
			if (sqlite_c->update("DELETE FROM friend_list WHERE user_id=" + to_string(ufriend_id) + ";")) {
				loger.warn() << "SQLite rollback in function applyRemoveUFriend.";
				sqlite_c->rollback();
				return -1;
			}
			string URL = "http://" + m_cqhttp_addr + "/delete_unidirectional_friend";
			json send_json;
			send_json["user_id"] = ufriend_id;
			string send_buffer = send_json.dump();
			string data_buffer;
			if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) {
				loger.warn() << "SQLite rollback in function applyRemoveUFriend.";
				sqlite_c->rollback();
				return -1;
			}
			sqlite_c->commit();
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyRemoveGroup(unsigned int group_id, bool dissolve) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		QQGroup g = getThisBotGroup(group_id);
		if (!g.isNull()) {
			loger.info() << "Leave group " << g.m_name << "(" << g.m_id << ")";
			sqlite_c->transaction();
			if (sqlite_c->update("DELETE FROM group_list WHERE group_id=" + to_string(group_id) + ";")) {
				loger.warn() << "SQLite rollback in function applyRemoveGroup.";
				sqlite_c->rollback();
				return -1;
			}
			if (sqlite_c->update("DELETE FROM group_member_list WHERE group_id=" + to_string(group_id) + ";")) {
				loger.warn() << "SQLite rollback in function applyRemoveGroup.";
				sqlite_c->rollback();
				return -1;
			}
			string URL = "http://" + m_cqhttp_addr + "/set_group_leave";
			json send_json;
			send_json["group_id"] = group_id;
			send_json["is_dismiss"] = dissolve;
			string send_buffer = send_json.dump();
			string data_buffer;
			if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) {
				loger.warn() << "SQLite rollback in function applyRemoveGroup.";
				sqlite_c->rollback();
				return -1;
			}
			sqlite_c->commit();
		}
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyWithdrawMsg(int message_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/delete_msg";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetThisBotProfile(const string& nickname, const string& company, const string& email, const string& college, const string& personal_note) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_qq_profile";
		json send_json;
		send_json["nickname"] = nickname;
		send_json["company"] = company;
		send_json["email"] = email;
		send_json["college"] = college;
		send_json["personal_note"] = personal_note;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		else fetchThisBotBasicInfo();
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyAddFriendRequest(const string& flag, const bool approve, const string& remark) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_friend_add_request";
		string data_buffer;
		json post_data;
		post_data["flag"] = flag;
		post_data["approve"] = approve;
		if (approve) {
			post_data["remark"] = remark;
		}
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		else fetchThisBotFriendList();
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyAddGroupRequest(const string& flag, const string& sub_type, const bool approve, const string& reason) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_add_request";
		string data_buffer;
		json post_data;
		post_data["flag"] = flag;
		post_data["approve"] = approve;
		post_data["sub_type"] = sub_type;
		if (!approve) {
			post_data["reason"] = reason;
		}
		if (sendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		else fetchThisBotGroupList();
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetDeviceShowName(const string& device_name, const string& device_name_element) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/_set_model_show";
		json send_json;
		send_json["model"] = device_name;
		send_json["model_show"] = device_name_element;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
	return 0;
}
int ThisBot::applyMarkMsgAsRead(int message_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/mark_msg_as_read";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetGroupName(unsigned int group_id, const string& name) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_name";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["group_name"] = name;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		else fetchThisBotGroupList();
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetGroupHeader(unsigned int group_id, const string& URI, int type) {
	return -1;
}
int ThisBot::applySetGroupAdmin(unsigned int group_id, unsigned int member_id, bool to_set) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_admin";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["user_id"] = member_id;
		send_json["enable"] = to_set;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		else fetchThisBotGroupMemberInfo(group_id, member_id);
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetGroupMemberNickname(unsigned int group_id, unsigned int member_id, const string& nickname) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_card";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["user_id"] = member_id;
		send_json["card"] = nickname;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		else fetchThisBotGroupMemberInfo(group_id, member_id);
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetGroupMemberTitle(unsigned int group_id, unsigned int member_id, const string& title, int time) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_special_title";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["user_id"] = member_id;
		send_json["special_title"] = title;
		send_json["duration"] = time;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		else fetchThisBotGroupMemberInfo(group_id, member_id);
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyMuteGroupMember(unsigned int group_id, unsigned int member_id, unsigned int time) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_ban";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["user_id"] = member_id;
		send_json["duration"] = time;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		else fetchThisBotGroupMemberInfo(group_id, member_id);
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyMuteGroupAll(unsigned int group_id, bool mute) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_whole_ban";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["enable"] = mute;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyMuteGroupAnonymous(unsigned int group_id, const string& anonymous_json_object, const string& anonymous_flag, unsigned int time) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_anonymous_ban";
		json send_json;
		send_json["group_id"] = group_id;
		if (anonymous_json_object.empty() && anonymous_flag.empty()) return -1;
		if (!anonymous_json_object.empty()) {
			json json_object = json::parse(anonymous_json_object);
			send_json["anonymous"] = json_object;
		}
		if (!anonymous_flag.empty()) {
			send_json["anonymous_flag "] = anonymous_flag;
		}
		send_json["duration "] = time;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySetGroupEssenceMsg(int message_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_essence_msg";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyRemoveGroupEssenceMsg(int message_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/delete_essence_msg";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyClockInGroup(unsigned int group_id) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/send_group_sign";
		json send_json;
		send_json["group_id"] = group_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applySendGroupNotice(unsigned int group_id, const string& content, const string& image_url) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/_send_group_notice";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["content"] = content;
		if (!image_url.empty()) {
			send_json["image"] = image_url;
		}
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}
int ThisBot::applyKickGroupMember(unsigned int group_id, unsigned int member_id, bool allow_join_again) {
	try {
		if (m_cqhttp_addr.empty()) return -1;
		string URL = "http://" + m_cqhttp_addr + "/set_group_kick";
		json send_json;
		send_json["group_id"] = group_id;
		send_json["user_id"] = member_id;
		send_json["reject_add_request"] = !allow_join_again;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (sendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		return -1;
	}
}

// 静态公开成员函数
string ThisBot::getQQHeaderImageURL(unsigned int QQid) {
	// https://qlogo3.store.qq.com/qzone/(%QQID%)/(%QQID%)/640.jfif		//OK
	// https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=(%QQID%)&spec=640		//OK
	// https://q1.qlogo.cn/g?b=qq&nk=(%QQID%)&s=640
	// https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=(%QQID%)&spec=640
	return "https://q1.qlogo.cn/g?b=qq&nk=" + to_string(QQid) + "&s=640";
}

// 静态私有成员函数
int ThisBot::sendGETRequest(const string& URL, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		loger.error() << "CURL init faild.";
		return -1;
	}
	try {
		if (!m_cqhttp_access_token.empty()) {										// 消息报头
			struct curl_slist* headers = NULL;
			string token_header = "Authorization: Bearer " + m_cqhttp_access_token;
			headers = curl_slist_append(headers, token_header.c_str());
			curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
		}
		curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());					// 设置URL
		curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET");				// 设置请求方法

		string reply_data;												// 接收数据的容器
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &reply_data);			// 设置得到请求结果后的回调函数的最后一个参数
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_callback);		// 得到请求结果后的回调函数
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10L);						// 请求超时时间为10秒
		curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 5L);				// 连接超时时间为5秒

		CURLcode ret_code = curl_easy_perform(handle);						 // 执行请求
		do {
			if (ret_code == CURLE_OK) {
				long reply_code; // must is long
				curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &reply_code);
				//正确响应后，输出数据
				if (reply_code == 200) {
					recv_buffer = reply_data;
				}
				else if (reply_code == 401) {
					loger.error() << "From go-cqhttp: Missing access token!";
					break;
				}
				else if (reply_code == 402) {
					loger.error() << "From go-cqhttp: Wrong access token!";
					break;
				}
				else if (reply_code == 403) {
					loger.error() << "From go-cqhttp: Content-Type unsupport!";
					break;
				}
				else if (reply_code == 404) {
					loger.error() << "From go-cqhttp: This API is not exist: " << URL;
					break;
				}
			}
			else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
				loger.warn() << "Waiting go-cqhttp time out!";
				break;
			}
			else {
				loger.error() << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code;
				break;
			}
			curl_easy_cleanup(handle);
			return 0;
		} while (false);
		curl_easy_cleanup(handle);
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		curl_easy_cleanup(handle);
		return -1;
	}
}
int ThisBot::sendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		loger.warn() << "CURL init faild.";
		return -1;
	}
	try {
		curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());					// 设置URL
		curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "POST");			// 设置请求方法
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, send_buffer.c_str());
		struct curl_slist* headers = NULL;									// 消息报头
		headers = curl_slist_append(headers, "Content-Type: application/json,application/octet-stream");
		if (!m_cqhttp_access_token.empty()) {
			string token_header = "Authorization: Bearer " + m_cqhttp_access_token;
			headers = curl_slist_append(headers, token_header.c_str());
		}
		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
		string reply_data;												// 接收数据的容器
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &reply_data);			// 设置得到请求结果后的回调函数的最后一个参数
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_callback);		// 得到请求结果后的回调函数
		curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10L);						// 请求超时时间为10秒
		curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 5L);				// 连接超时时间为5秒

		CURLcode ret_code = curl_easy_perform(handle);						// 执行请求

		do {
			if (ret_code == CURLE_OK) {
				long reply_code; // must is long
				curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &reply_code);
				//正确响应后，输出数据
				if (reply_code == 200) {
					recv_buffer = reply_data;
				}
				else if (reply_code == 401) {
					loger.error() << "From go-cqhttp: Missing access token!";
					break;
				}
				else if (reply_code == 402) {
					loger.error() << "From go-cqhttp: Wrong access token!";
					break;
				}
				else if (reply_code == 403) {
					loger.error() << "From go-cqhttp: Content-Type unsupport!";
					break;
				}
				else if (reply_code == 404) {
					loger.error() << "From go-cqhttp: This API is not exist: " << URL;
					break;
				}
			}
			else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
				loger.warn() << "Waiting go-cqhttp time out!";
				break;
			}
			else {
				loger.error() << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code;
				break;
			}
			curl_easy_cleanup(handle);
			return 0;
		} while (false);
		curl_easy_cleanup(handle);
		return -1;
	}
	catch (...) {
		loger.error() << "Exception in function " << __FUNCTION__;
		curl_easy_cleanup(handle);
		return -1;
	}
}