#include <curl.h>
#include <chrono>
#include "QQ.h"

vector<QQGroupMember>::iterator QQGroup::FindMember(unsigned int QQid) {
	vector<QQGroupMember>::iterator iter = group_member_list_.begin();
	for (; iter != group_member_list_.end(); iter++) {
		if (iter->id_ == QQid) return iter;
	}
	return iter;
}

bool QQGroup::GetGroupHasMember(unsigned int QQid) const {
	auto iter = group_member_list_.cbegin();
	for (; iter != group_member_list_.end(); iter++) {
		if (iter->id_ == QQid) return true;
	}
	return false;
}

QQFriend QQBot::NULLFriend = QQFriend(0);
QQGroup QQBot::NULLGroup = QQGroup(0);

/************************QQBot类的成员函数***************************/
// 这些函数用以获取Bot中已设置的信息
unsigned int QQBot::GetBotID() {
	unique_lock<mutex> auto_lock(QQ_lock_);
	unsigned int ret = QQBot_id_;
	return ret;
}
string QQBot::GetBotNickname() {
	unique_lock<mutex> auto_lock(QQ_lock_);
	string ret = QQBot_nickname_;
	return ret;
}
bool QQBot::GetBotHasAdmin() {
	bool ret = true;
	unique_lock<mutex> auto_lock(QQ_lock_);
	if (administrator_.id_ == 0) ret = false;
	return ret;
}
QQFriend QQBot::GetBotAdmin() {
	unique_lock<mutex> auto_lock(QQ_lock_);
	QQFriend ret = administrator_;
	return ret;
}
int QQBot::GetBotFriendNum() {
	unique_lock<mutex> auto_lock(QQ_lock_);
	int ret = QQBot_friend_list_.size();
	return ret;
}
int QQBot::GetBotUFriendNum() {
	unique_lock<mutex> auto_lock(QQ_lock_);
	int ret = QQBot_Ufriend_list_.size();
	return ret;
}
int QQBot::GetBotGroupNum() {
	unique_lock<mutex> auto_lock(QQ_lock_);
	int ret = QQBot_group_list_.size();
	return ret;
}
bool QQBot::GetBotHasQQFriend(unsigned int friend_id) {
	unique_lock<mutex> auto_lock(QQ_lock_);
	auto iter = FindQQFriend(friend_id);
	if (iter != QQBot_friend_list_.end())return true;
	return false;
}
bool QQBot::GetBotHasQQUFriend(unsigned int friend_id) {
	unique_lock<mutex> auto_lock(QQ_lock_);
	auto iter = FindQQUFriend(friend_id);
	if (iter != QQBot_Ufriend_list_.end())return true;
	return false;
}
bool QQBot::GetBotHasQQGroup(unsigned int group_id) {
	unique_lock<mutex> auto_lock(QQ_lock_);
	auto iter = FindQQGroup(group_id);
	if (iter != QQBot_group_list_.end())return true;
	return false;
}
const QQFriend QQBot::GetBotFriend(unsigned int friend_id) {
	unique_lock<mutex> auto_lock(QQ_lock_);
	auto iter = FindQQFriend(friend_id);
	if (iter != QQBot_friend_list_.end())return (*iter);
	return NULLFriend;
}
const QQGroup QQBot::GetBotGroup(unsigned int group_id) {
	unique_lock<mutex> auto_lock(QQ_lock_);
	auto iter = FindQQGroup(group_id);
	if (iter != QQBot_group_list_.end())return (*iter);
	return NULLGroup;
}
const vector<unsigned int> QQBot::GetBotGroupIDList() {
	vector<unsigned int> ret;
	unique_lock<mutex> auto_lock(QQ_lock_);
	for (auto& value : QQBot_group_list_) {
		ret.push_back(value.id_);
	}
	return ret;
}
const vector<unsigned int> QQBot::GetBotFriendIDList() {
	vector<unsigned int> ret;
	unique_lock<mutex> auto_lock(QQ_lock_);
	for (auto& element : QQBot_friend_list_) {
		ret.push_back(element.id_);
	}
	return ret;
}
const vector<unsigned int> QQBot::GetBotUFriendIDList() {
	vector<unsigned int> ret;
	unique_lock<mutex> auto_lock(QQ_lock_);
	for (auto& element : QQBot_Ufriend_list_) {
		ret.push_back(element.id_);
	}
	return ret;
}
string QQBot::GetBotAccessToken() const {
	return access_token_;
}
void QQBot::PrintFriendList() {
	Qlog.Info() << " ┌─ \033[34mQQ friends list\033[0m" << endl;
	unique_lock<mutex> auto_lock(QQ_lock_);
	for (auto& element : QQBot_friend_list_) {
		Qlog.Info() << " ├─ " << element.name_ << "(" << element.id_ << ")" << endl;
	}
	Qlog.Info() << " └─ \033[34mTotal num: " << QQBot_friend_list_.size() << "\033[0m" << endl;
}
void QQBot::PrintUFriendList() {
	Qlog.Info() << " ┌─ \033[34mQQ unidirectional friends list\033[0m" << endl;
	unique_lock<mutex> auto_lock(QQ_lock_);
	for (auto& element : QQBot_Ufriend_list_) {
		Qlog.Info() << " ├─ " << element.name_ << "(" << element.id_ << ")" << endl;
	}
	Qlog.Info() << " └─ \033[34mTotal num: " << QQBot_Ufriend_list_.size() << "\033[0m" << endl;
}
void QQBot::PrintGroupList() {
	Qlog.Info() << " ┌─ \033[34mQQ groups list\033[0m" << endl;
	unique_lock<mutex> auto_lock(QQ_lock_);
	for (auto& element : QQBot_group_list_) {
		Qlog.Info() << " ├─ " << element.name_ << "(" << element.id_ << ")" << endl;
	}
	Qlog.Info() << " └─ \033[34mTotal num: " << QQBot_group_list_.size() << "\033[0m" << endl;
}

// 这些函数用以设置Bot的信息
int QQBot::SetBotAdmin(unsigned int admin_id) {
	unique_lock<mutex> auto_lock(QQ_lock_);
	if (FindQQFriend(admin_id) != QQBot_friend_list_.end()) {
		administrator_ = QQFriend(admin_id);
	}
	else {
		Qlog.Error() << "The administrator is not a friend of QQ bot, setting failed." << endl;
		return -1;
	}
	return 0;
}
int QQBot::SetBotBasicInfo() {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_login_info";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			unique_lock<mutex> auto_lock(QQ_lock_);
			QQBot_id_ = json_data["data"]["user_id"];
			QQBot_nickname_ = json_data["data"]["nickname"];
			Qlog.Info() << "Bot information: " << QQBot_nickname_ << "(" << QQBot_id_ << ")" << endl;
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetBotBasicInfo()." << endl;
		return -1;
	}
}
int QQBot::SetBotFriendList() {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_friend_list";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			unique_lock<mutex> auto_lock(QQ_lock_);
			QQBot_friend_list_.clear();
			for (auto& element : json_data["data"]) {
				QQBot_friend_list_.push_back(QQFriend(element["user_id"], element["nickname"], element["remark"]));
			}
			Qlog.Info() << "Bot's number of friends: " << QQBot_friend_list_.size() << endl;
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetBotFriendList()." << endl;
		return -1;
	}
}
int QQBot::SetBotUFriendList() {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_unidirectional_friend_list";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			unique_lock<mutex> auto_lock(QQ_lock_);
			QQBot_Ufriend_list_.clear();
			for (auto& element : json_data["data"]) {
				QQBot_Ufriend_list_.push_back(QQUFriend(element["user_id"], element["nickname"], element["source"]));
			}
			Qlog.Info() << "Bot's number of unidirectional friends: " << QQBot_Ufriend_list_.size() << endl;
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetBotUFriendList()." << endl;
		return -1;
	}
}
int QQBot::SetBotGroupList() {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_group_list";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			unique_lock<mutex> auto_lock(QQ_lock_);
			QQBot_group_list_.clear();
			for (auto& element : json_data["data"]) {
				QQGroup group(element["group_id"], element["group_name"], element["member_count"], element["max_member_count"]);
				QQBot_group_list_.push_back(move(group));
			}
			auto_lock.unlock();
			for (auto& group : QQBot_group_list_) {
				SetGroupMemberList(group.id_);
			}
			for (auto& group : QQBot_group_list_) {
				for (auto& member : group.group_member_list_) {
					SetGroupMemberInfo(group.id_, member.id_);
				}
			}
			auto_lock.lock();
			Qlog.Info() << "Bot's number of groups: " << QQBot_group_list_.size() << endl;
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetBotGroupList()." << endl;
		return -1;
	}
}
int QQBot::SetBotAllInfo() {
	if (cqhttp_addr_.empty()) {
		Qlog.Error() << "go-cqhttp address not set!" << endl;
		return -1;
	}
	if (SetBotBasicInfo() != 0) {
		Qlog.Error() << "Failed to get QQBot information from go-cqhttp!" << endl;
		return -1;
	}
	if (SetBotFriendList() != 0) {
		Qlog.Error() << "Failed to get QQBot friend list from go-cqhttp!" << endl;
		return -1;
	}
	if (SetBotUFriendList() != 0) {
		Qlog.Error() << "Failed to get QQBot unidirectional friend list from go-cqhttp!" << endl;
		return -1;
	}
	if (SetBotGroupList() != 0) {
		Qlog.Error() << "Failed to get QQBot group list from go-cqhttp!" << endl;
		return -1;
	}
	return 0;
}
int QQBot::SetGroupInfo(QQGroup& group) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_group_info";
		string data_buffer;
		json post_data;
		post_data["group_id"] = group.id_;
		post_data["no_cache"] = !use_cache_;
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			group.name_ = json_data["data"]["group_name"];
			group.member_count_ = json_data["data"]["member_count"];
			group.max_member_count_ = json_data["data"]["max_member_count"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetGroupInfo(QQGroup&)." << endl;
		return -1;
	}
}
int QQBot::SetGroupMemberList(unsigned int group_id) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		if (group_id == 0) return -1;
		unique_lock<mutex> auto_lock(QQ_lock_);
		auto group_iter = FindQQGroup(group_id);
		if (group_iter == QQBot_group_list_.end()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_group_member_list";
		string data_buffer;
		json post_data;
		post_data["group_id"] = group_id;
		post_data["no_cache"] = !use_cache_;
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			group_iter->group_member_list_.clear();
			for (auto& x : json_data["data"]) {
				QQGroupMember member(x["user_id"], x["nickname"], x["card"], x["group_id"], x["title"]);
				string role = x["role"];
				if (role == "member") {
					member.role_ = 0;
				}
				else if (role == "admin") {
					member.role_ = 1;
				}
				else if (role == "owner") {
					member.role_ = 2;
				}
				group_iter->group_member_list_.push_back(move(member));
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetGroupMemberList(unsigned int)." << endl;
		return -1;
	}
}
int QQBot::SetGroupMemberInfo(unsigned int group_id, unsigned int member_id) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		unique_lock<mutex> auto_lock(QQ_lock_);
		auto group_iter = FindQQGroup(group_id);
		if (group_iter == QQBot_group_list_.end()) {
			return 0;
		};
		if (!group_iter->GetGroupHasMember(member_id)) {
			return 0;
		}
		auto member_iter = group_iter->FindMember(member_id);
		string URL = "http://" + cqhttp_addr_ + "/get_group_member_info";
		string data_buffer;
		json post_data;
		post_data["group_id"] = group_id;
		post_data["user_id"] = member_id;
		post_data["no_cache"] = !use_cache_;
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			member_iter->group_nickname_ = json_data["data"]["card"];
			member_iter->group_title_ = json_data["data"]["title"];
			string role = json_data["data"]["role"];
			if (role == "member") {
				member_iter->role_ = 0;
			}
			else if (role == "admin") {
				member_iter->role_ = 1;
			}
			else if (role == "owner") {
				member_iter->role_ = 2;
			}
			member_iter->age_ = json_data["data"]["age"];
			member_iter->area_ = json_data["data"]["area"];
			member_iter->join_time_ = json_data["data"]["join_time"];
			member_iter->last_active_time_ = json_data["data"]["last_sent_time"];
			member_iter->level_ = json_data["data"]["level"];
			if (json_data["data"]["unfriendly"]) {
				member_iter->unfriendly_ = true;
			}
			member_iter->title_expire_time_ = json_data["data"]["title_expire_time"];
			member_iter->card_changeable_ = json_data["data"]["card_changeable"];
			member_iter->shut_up_timestamp_ = json_data["data"]["shut_up_timestamp"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetBotBasicInfo()." << endl;
		return -1;
	}
}
int QQBot::ResetBotGroup(unsigned int group_id) {
	try {
		if (group_id == 0) return -1;
		unique_lock<mutex> auto_lock(QQ_lock_);
		auto iter = FindQQGroup(group_id);
		if (iter == QQBot_group_list_.end()) {
			return -1;
		}
		SetGroupInfo(*iter);
		auto_lock.release();
		QQ_lock_.unlock();
		SetGroupMemberList(group_id);
		for (auto& member : iter->group_member_list_) {
			SetGroupMemberInfo(iter->id_, member.id_);
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function ResetBotGroup(unsigned int)." << endl;
		return -1;
	}
}

// 这些函数用以向go-cqhttp请求实现Bot的某些主动动作或获得一些信息
int QQBot::SendPrivateMsg(const QQFriend& qfriend, QQMessage& msg) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		if (!msg.CanSendToPrivate()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/send_private_msg";
		json send_json;
		send_json["user_id"] = qfriend.id_;
		send_json["message"] = msg.GetMsg();
		send_json["auto_escape"] = !msg.IsCQMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.SetMsgID(json_data["data"]["message_id"]);
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendPrivateMsg(const QQFriend&, QQMessage&)." << endl;
		return -1;
	}
}
int QQBot::SendGroupMsg(const QQGroup& group, QQMessage& msg) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		if (!msg.CanSendToGroup()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/send_group_msg";

		json send_json;
		send_json["group_id"] = group.id_;
		send_json["message"] = msg.GetMsg();
		send_json["auto_escape"] = !msg.IsCQMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.SetMsgID(json_data["data"]["message_id"]);
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendGroupMsg(const QQGroup&, QQMessage&)." << endl;
		return -1;
	}
}
int QQBot::DeleteFriend(const QQFriend& qfriend) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		unique_lock<mutex> auto_lock(QQ_lock_);
		auto iter = FindQQFriend(qfriend.id_);
		auto_lock.unlock();
		if (iter == QQBot_friend_list_.end()) return 0;
		Qlog.Info() << "Delete friend " << qfriend.name_ << "(" << qfriend.id_ << ")" << endl;
		string URL = "http://" + cqhttp_addr_ + "/delete_friend";
		json send_json;
		send_json["user_id"] = qfriend.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		auto_lock.lock();
		QQBot_friend_list_.erase(iter);
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function DeleteFriend(const QQFriend&)." << endl;
		return -1;
	}
}
int QQBot::DeleteUFriend(const QQUFriend& ufriend) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		unique_lock<mutex> auto_lock(QQ_lock_);
		auto iter = FindQQUFriend(ufriend.id_);
		auto_lock.unlock();
		if (iter == QQBot_Ufriend_list_.end()) return 0;
		Qlog.Info() << "Delete unidirectional friend " << ufriend.name_ << "(" << ufriend.id_ << ")" << endl;
		string URL = "http://" + cqhttp_addr_ + "/delete_unidirectional_friend";
		json send_json;
		send_json["user_id"] = ufriend.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		auto_lock.lock();
		QQBot_Ufriend_list_.erase(iter);
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function DeleteUFriend(const QQFriend&)." << endl;
		return -1;
	}
}
int QQBot::DeleteGroup(const QQGroup& group, bool dissolve) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		unique_lock<mutex> auto_lock(QQ_lock_);
		auto iter = FindQQGroup(group.id_);
		auto_lock.unlock();
		if (iter == QQBot_group_list_.end()) return 0;
		Qlog.Warn() << "leave group " << group.name_ << "(" << group.id_ << ")" << endl;
		string URL = "http://" + cqhttp_addr_ + "/set_group_leave";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["is_dismiss"] = dissolve;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		auto_lock.lock();
		QQBot_group_list_.erase(iter);
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function DeleteGroup(const QQGroup&)." << endl;
		return -1;
	}
}
int QQBot::WithdrawMsg(int message_id) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/delete_msg";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function WithdrawMsg(int)." << endl;
		return -1;
	}
}
bool QQBot::CanSendImage() {
	try {
		if (cqhttp_addr_.empty()) return false;
		string URL = "http://" + cqhttp_addr_ + "/can_send_image";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			return json_data["data"]["yes"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return false;
		}
		return true;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function CanSendImage()." << endl;
		return false;
	}
}
bool QQBot::CanSendRecord() {
	try {
		if (cqhttp_addr_.empty()) return false;
		string URL = "http://" + cqhttp_addr_ + "/can_send_record";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			return json_data["data"]["yes"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return false;
		}
		return true;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function CanSendRecord()." << endl;
		return false;
	}
}
int QQBot::PrintCqhttpVersion() {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_version_info";
		string data_buffer;
		if (SendGETRequest(URL, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			string version = json_data["data"]["version"];
			version.pop_back();
			version.erase(0, 1);
			Qlog.Info() << "Using go-cqhttp: " << version << endl;
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function PrintCqhttpVersion()." << endl;
		return -1;
	}
}
int QQBot::SetQQRrofile(const string& nickname, const string& company, const string& email, const string& college, const string& personal_note) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_qq_profile";
		json send_json;
		send_json["nickname"] = nickname;
		send_json["company"] = company;
		send_json["email"] = email;
		send_json["college"] = college;
		send_json["personal_note"] = personal_note;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetQQRrofile(const string&, const string&, const string&, const string&, const string&)." << endl;
		return -1;
	}
}
QQUser QQBot::GetQQUserInfo(unsigned int QQid) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_stranger_info";
		string data_buffer;
		json post_data;
		post_data["user_id"] = QQid;
		post_data["no_cache"] = !use_cache_;
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		QQUser user(QQid);
		if (json_data["status"] == "ok") {
			user.name_ = json_data["data"]["nickname"];
			user.age_ = json_data["data"]["age"];
			string gender = json_data["data"]["sex"];
			if (gender == "unknown") user.gender_ = 0;
			if (gender == "male") user.gender_ = 1;
			if (gender == "female") user.gender_ = 2;
			user.qid_ = json_data["data"]["qid"];
			user.level_ = json_data["data"]["level"];
			user.login_days_ = json_data["data"]["login_days"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return user;
		}
		return user;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetQQUserInfo(unsigned int)." << endl;
		return QQUser(QQid);
	}
}
QQRawMessage QQBot::GetMessageInfo(int message_id) {
	QQRawMessage msg;
	try {
		if (cqhttp_addr_.empty()) return msg;
		string URL = "http://" + cqhttp_addr_ + "/get_msg";
		string data_buffer;
		json post_data;
		post_data["message_id"] = message_id;
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return msg;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.message_id_ = json_data["data"]["message_id"];
			msg.is_group_msg = json_data["data"]["group"];
			if (msg.is_group_msg) {
				msg.group_id = json_data["data"]["group_id"];
			}
			msg.real_id_ = json_data["data"]["real_id"];
			msg.sender_id_ = json_data["data"]["sender"]["user_id"];
			msg.sender_nickname_ = json_data["data"]["sender"]["nickname"];
			msg.send_time_ = json_data["data"]["time"];
			msg.message_ = json_data["data"]["message"];
			msg.raw_message_ = json_data["data"]["raw_message"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return msg;
		}
		return msg;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetMessageInfo(int)." << endl;
		return msg;
	}
}
int QQBot::SetAddFriendRequest(const string& flag, const bool approve, const string& remark) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_friend_add_request";
		string data_buffer;
		json post_data;
		post_data["flag"] = flag;
		post_data["approve"] = approve;
		if (approve) {
			post_data["remark"] = remark;
		}
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetAddFriendRequest(const string&, const bool, const string&)." << endl;
		return -1;
	}
}
int QQBot::SetAddGroupRequest(const string& flag, const string& sub_type, const bool approve, const string& reason) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_add_request";
		string data_buffer;
		json post_data;
		post_data["flag"] = flag;
		post_data["approve"] = approve;
		post_data["sub_type"] = sub_type;
		if (!approve) {
			post_data["reason"] = reason;
		}
		if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetAddGroupRequest(const string&, const string&, const bool, const string&)." << endl;
		return -1;
	}
}
vector<pair<string, bool>> QQBot::GetDeviceShowList(const string& device_name) {
	vector<pair<string, bool>> ret_set;
	try {
		if (cqhttp_addr_.empty()) return ret_set;
		string URL = "http://" + cqhttp_addr_ + "/_get_model_show";
		json send_json;
		send_json["model"] = device_name;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return ret_set;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			auto device_list = json_data["data"]["variants"];
			for (auto& element : device_list) {
				ret_set.push_back(make_pair<string, bool>(element["model_show"], element["need_pay"]));
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
		}
		return ret_set;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetDeviceShowList(const string&)." << endl;
		return ret_set;
	}
}
int QQBot::SetDeviceShowName(const string& device_name, const string& device_name_element) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/_set_model_show";
		json send_json;
		send_json["model"] = device_name;
		send_json["model_show"] = device_name_element;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetDeviceShowName(const string&, const string&)." << endl;
		return -1;
	}
	return 0;
}
vector<QQClient> QQBot::GetOnlineClients() {
	vector<QQClient> clients;
	try {
		if (cqhttp_addr_.empty()) return clients;
		string URL = "http://" + cqhttp_addr_ + "/get_online_clients";
		json send_json;
		send_json["no_cache"] = !use_cache_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return clients;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			auto client_list = json_data["data"]["clients"];
			for (auto& element : client_list) {
				clients.push_back(QQClient(element["app_id"], element["device_name"], element["device_kind"]));
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return clients;
		}
		return clients;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetOnlineClients()." << endl;
		return clients;
	}
}
int QQBot::MarkMsgAsRead(int message_id) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/mark_msg_as_read";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function MarkMsgAsRead(int)." << endl;
		return -1;
	}
}
vector<QQForwardMsgNode> QQBot::GetForwardMsg(const string& forward_id) {
	vector<QQForwardMsgNode> forward_msg_list;
	try {
		if (cqhttp_addr_.empty()) return forward_msg_list;
		string URL = "http://" + cqhttp_addr_ + "/get_forward_msg";
		json send_json;
		send_json["message_id"] = forward_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return forward_msg_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]["messages"]) {
				QQForwardMsgNode qfm;
				qfm.sender_id_ = element["sender"]["user_id"];
				qfm.sender_nickname_ = element["sender"]["nickname"];
				qfm.message_ = element["content"];
				qfm.send_time_ = element["time"];
				forward_msg_list.push_back(qfm);
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return forward_msg_list;
		}
		return forward_msg_list;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendGroupMsg(const QQGroup&, QQMessage&)." << endl;
		return forward_msg_list;
	}
}
int QQBot::SendPrivateForwardMsg(const QQFriend& qfriend, QQMessage& msg) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		if (!msg.CanSendToPrivate()) return -1;
		if (!msg.IsForwardMsg()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/send_private_forward_msg";
		json send_json;
		send_json["user_id"] = qfriend.id_;
		send_json["message"] = msg.GetMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.SetMsgID(json_data["data"]["message_id"]);
			msg.SetForwardMsgID(json_data["data"]["forward_id"]);
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendPrivateForwardMsg(const QQFriend&, QQMessage&)." << endl;
		return -1;
	}
}
int QQBot::SendGroupeForwardMsg(const QQGroup& group, QQMessage& msg) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		if (!msg.CanSendToGroup()) return -1;
		if (!msg.IsForwardMsg()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/send_group_forward_msg";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["message"] = msg.GetMsg();
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			msg.SetMsgID(json_data["data"]["message_id"]);
			msg.SetForwardMsgID(json_data["data"]["forward_id"]);
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendGroupeForwardMsg(const QQGroup&, QQMessage&)." << endl;
		return -1;
	}
}
vector<QQRawMessage> QQBot::GetGroupHistoryMsg(const QQGroup& group, unsigned int msg_seq) {
	vector<QQRawMessage> history_msg_list;
	try {
		if (cqhttp_addr_.empty()) return history_msg_list;
		string URL = "http://" + cqhttp_addr_ + "/get_group_msg_history";

		json send_json;
		send_json["group_id"] = group.id_;
		if (msg_seq != 0) {
			send_json["message_seq"] = msg_seq;
		}
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return history_msg_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]) {
				QQRawMessage msg;
				msg.message_id_ = element["message_id"];
				msg.is_group_msg = element["group"];
				if (msg.is_group_msg) {
					msg.group_id = element["group_id"];
				}
				msg.real_id_ = element["real_id"];
				msg.sender_id_ = element["sender"]["user_id"];
				msg.sender_nickname_ = element["sender"]["nickname"];
				msg.send_time_ = element["time"];
				msg.message_ = element["message"];
				msg.raw_message_ = element["raw_message"];
				history_msg_list.push_back(msg);
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return history_msg_list;
		}
		return history_msg_list;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetGroupHistoryMsg(const QQGroup&, unsigned int)." << endl;
		return history_msg_list;
	}
}
int QQBot::GetImageInfo(const string& file, int& size, string& filename, string& url) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_image";
		json send_json;
		send_json["file"] = file;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			size = json_data["data"]["size"];
			filename = json_data["data"]["filename"];
			url = json_data["data"]["url"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetImageInfo(const string&, int&, string&, string&)." << endl;
		return -1;
	}
}
QQImageOCR QQBot::GetImageOCR(const string& image_id) {
	return QQImageOCR();
}
string QQBot::GetRecordInfo(const string& file, const string& out_format) {
	string file_addr;
	try {
		if (cqhttp_addr_.empty()) return file_addr;
		string URL = "http://" + cqhttp_addr_ + "/get_record";
		json send_json;
		send_json["file"] = file;
		send_json["out_format"] = out_format;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return file_addr;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			file_addr = json_data["data"]["file"];
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return file_addr;
		}
		return file_addr;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetImageOCR(const string&)." << endl;
		return file_addr;
	}
}
QQGroupHonor QQBot::GetGroupHonorInfo(const QQGroup& group, const string& type) {
	QQGroupHonor QGH;
	try {
		if (cqhttp_addr_.empty()) return QGH;
		string URL = "http://" + cqhttp_addr_ + "/get_record";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["type"] = type;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return QGH;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			QGH.group_id_ = group.id_;
			QGH.dragon_king_.user_id_ = json_data["data"]["current_talkative"]["user_id"];
			QGH.dragon_king_.nickname_ = json_data["data"]["current_talkative"]["nickname"];
			QGH.dragon_king_.header_url_ = json_data["data"]["current_talkative"]["avatar"];
			QGH.dragon_king_.day_count_ = json_data["data"]["current_talkative"]["day_count"];
			for (auto& element : json_data["data"]["talkative_list"]) {
				QQOtherHonor QOH;
				QOH.user_id_ = element["user_id"];
				QOH.nickname_ = element["nickname"];
				QOH.header_url_ = element["avatar"];
				QOH.description_ = element["description"];
				QGH.history_dragon_king_.push_back(QOH);
			}
			for (auto& element : json_data["data"]["performer_list"]) {
				QQOtherHonor QOH;
				QOH.user_id_ = element["user_id"];
				QOH.nickname_ = element["nickname"];
				QOH.header_url_ = element["avatar"];
				QOH.description_ = element["description"];
				QGH.chat_flame_.push_back(QOH);
			}
			for (auto& element : json_data["data"]["legend_list"]) {
				QQOtherHonor QOH;
				QOH.user_id_ = element["user_id"];
				QOH.nickname_ = element["nickname"];
				QOH.header_url_ = element["avatar"];
				QOH.description_ = element["description"];
				QGH.chat_large_flame_.push_back(QOH);
			}
			for (auto& element : json_data["data"]["strong_newbie_list"]) {
				QQOtherHonor QOH;
				QOH.user_id_ = element["user_id"];
				QOH.nickname_ = element["nickname"];
				QOH.header_url_ = element["avatar"];
				QOH.description_ = element["description"];
				QGH.bamboo_shoot_.push_back(QOH);
			}
			for (auto& element : json_data["data"]["emotion_list"]) {
				QQOtherHonor QOH;
				QOH.user_id_ = element["user_id"];
				QOH.nickname_ = element["nickname"];
				QOH.header_url_ = element["avatar"];
				QOH.description_ = element["description"];
				QGH.happy_sorce_.push_back(QOH);
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return QGH;
		}
		return QGH;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetGroupHonorInfo(const QQGroup&, const string&, string&)." << endl;
		return QGH;
	}
}
QQGroupSystemMsg QQBot::GetGroupSystemMsg(const QQGroup& group) {
	QQGroupSystemMsg QGSM;
	try {
		if (cqhttp_addr_.empty()) return QGSM;
		string URL = "http://" + cqhttp_addr_ + "/get_group_system_msg";
		json send_json;
		send_json["group_id"] = group.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return QGSM;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]["invited_requests"]) {
				QQGroupInvitedRequest QGIR;
				QGIR.request_id_ = element["request_id"];
				QGIR.invitor_id_ = element["invitor_uin"];
				QGIR.invitor_nickname_ = element["invitor_nick"];
				QGIR.group_id_ = element["group_id"];
				QGIR.group_name_ = element["group_name"];
				QGIR.has_checked_ = element["checked"];
				QGIR.operator_id_ = element["actor"];
				QGSM.InvitedList.push_back(QGIR);
			}
			for (auto& element : json_data["data"]["join_requests"]) {
				QQGroupJoinRequest QGJR;
				QGJR.request_id_ = element["request_id"];
				QGJR.applicant_id_ = element["requester_uin"];
				QGJR.applicant_nickname_ = element["requester_nick"];
				QGJR.applicant_message_ = element["message"];
				QGJR.group_id_ = element["group_id"];
				QGJR.group_name_ = element["group_name"];
				QGJR.has_checked_ = element["checked"];
				QGJR.operator_id_ = element["actor"];
				QGSM.JoinList.push_back(QGJR);
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return QGSM;
		}
		return QGSM;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetGroupSystemMsg(const QQGroup&)." << endl;
		return QGSM;
	}
}
vector<QQEssenceMsg> QQBot::GetGroupEssenceMsg(const QQGroup& group) {
	vector<QQEssenceMsg> essence_msg_list;
	try {
		if (cqhttp_addr_.empty()) return essence_msg_list;
		string URL = "http://" + cqhttp_addr_ + "/get_essence_msg_list";
		json send_json;
		send_json["group_id"] = group.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return essence_msg_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]) {
				QQEssenceMsg essence_msg;
				essence_msg.sender_id_ = element["sender_id"];
				essence_msg.sender_nickname_ = element["sender_nick"];
				essence_msg.send_time_ = element["sender_time"];
				essence_msg.operator_id_ = element["operator_id"];
				essence_msg.operator_nickname_ = element["operator_nick"];
				essence_msg.operator_settime_ = element["operator_time"];
				essence_msg.message_id_ = element["message_id"];
				essence_msg_list.push_back(essence_msg);
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return essence_msg_list;
		}
		return essence_msg_list;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetGroupEssenceMsg(const QQGroup&)." << endl;
		return essence_msg_list;
	}
}
int QQBot::GetGroupAtAllChance(const QQGroup& group) {
	int ret = 0;
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/get_essence_msg_list";
		json send_json;
		send_json["group_id"] = group.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
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
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return -1;
		}
		return ret;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetGroupEssenceMsg(const QQGroup&)." << endl;
		return -1;
	}
}
int QQBot::SetGroupName(const QQGroup& group, const string& name) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_name";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["group_name"] = name;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetGroupName(const QQGroup&, const string&)." << endl;
		return -1;
	}
}
int QQBot::SetGroupHeader(const QQGroup& group, const string& URI, int type) {
	return -1;
}
int QQBot::SetGroupAdmin(const QQGroup& group, const QQGroupMember& member, bool to_set) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_admin";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["user_id"] = member.id_;
		send_json["enable"] = to_set;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetGroupAdmin(const QQGroup&, const QQGroupMember&, bool)." << endl;
		return -1;
	}
}
int QQBot::SetGroupMemberNickname(const QQGroup& group, const QQGroupMember& member, const string& nickname) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_card";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["user_id"] = member.id_;
		send_json["card"] = nickname;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetGroupMemberNickname(const QQGroup&, const QQGroupMember&, const string&)." << endl;
		return -1;
	}
}
int QQBot::SetGroupMemberTitle(const QQGroup& group, const QQGroupMember& member, const string& title, int time) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_special_title";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["user_id"] = member.id_;
		send_json["special_title"] = title;
		send_json["duration"] = time;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetGroupMemberTitle(const QQGroup&, const QQGroupMember&, const string&, int)." << endl;
		return -1;
	}
}
int QQBot::GroupMuteMember(const QQGroup& group, const QQGroupMember& member, unsigned int time) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_ban";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["user_id"] = member.id_;
		send_json["duration"] = time;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GroupMuteMember(const QQGroup&, const QQGroupMember&, unsigned int)." << endl;
		return -1;
	}
}
int QQBot::GroupMuteAll(const QQGroup& group, bool mute) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_whole_ban";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["enable"] = mute;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GroupMuteAll(const QQGroup&, bool)." << endl;
		return -1;
	}
}
int QQBot::GroupMuteAnonymous(const QQGroup& group, const string& anonymous_json_object, const string& anonymous_flag, unsigned int time) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_anonymous_ban";
		json send_json;
		send_json["group_id"] = group.id_;
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
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GroupMuteAnonymous(const QQGroup&, const string&, const string&, unsigned int)." << endl;
		return -1;
	}
}
int QQBot::SetEssenceMsg(int message_id) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_essence_msg";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SetEssenceMsg(int)." << endl;
		return -1;
	}
}
int QQBot::RemoveEssenceMsg(int message_id) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/delete_essence_msg";
		json send_json;
		send_json["message_id"] = message_id;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function RemoveEssenceMsg(int)." << endl;
		return -1;
	}
}
int QQBot::ClockInGroup(const QQGroup& group) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/send_group_sign";
		json send_json;
		send_json["group_id"] = group.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function ClockInGroup(const QQGroup&)." << endl;
		return -1;
	}
}
int QQBot::SendGroupNotice(const QQGroup& group, const string& content, const string& image_url) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/_send_group_notice";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["content"] = content;
		if (!image_url.empty()) {
			send_json["image"] = image_url;
		}
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendGroupNotice(const QQGroup&, const string&, const string&)." << endl;
		return -1;
	}
}
vector<QQGroupNotice> QQBot::GetGroupNotice(const QQGroup& group) {
	vector<QQGroupNotice> group_notice_list;
	try {
		if (cqhttp_addr_.empty()) return group_notice_list;
		string URL = "http://" + cqhttp_addr_ + "/_get_group_notice";
		json send_json;
		send_json["group_id"] = group.id_;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return group_notice_list;
		json json_data = json::parse(data_buffer, NULL, false);
		if (json_data["status"] == "ok") {
			for (auto& element : json_data["data"]) {
				QQGroupNotice group_notice;
				group_notice.sender_id_ = element["sender_id"];
				group_notice.send_time_ = element["publish_time"];
				group_notice.content_ = element["message"]["text"];
				group_notice.image_id_ = element["message"]["images"]["id"];
				group_notice_list.push_back(group_notice);
			}
		}
		else if (json_data["status"] == "failed") {
			Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
				<< json_data["msg"] << ":" << json_data["wording"] << endl;
			return group_notice_list;
		}
		return group_notice_list;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GetGroupNotice(const QQGroup&)." << endl;
		return group_notice_list;
	}
}
int QQBot::GroupKickMember(const QQGroup& group, const QQGroupMember& member, bool allow_join_again) {
	try {
		if (cqhttp_addr_.empty()) return -1;
		string URL = "http://" + cqhttp_addr_ + "/set_group_kick";
		json send_json;
		send_json["group_id"] = group.id_;
		send_json["user_id"] = member.id_;
		send_json["reject_add_request"] = !allow_join_again;
		string send_buffer = send_json.dump();
		string data_buffer;
		if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
		return 0;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function GroupKickMember(const QQGroup&, const QQGroupMember&, bool)." << endl;
		return -1;
	}
}

// 静态成员函数
string QQBot::GetQQHeaderImageURL(unsigned int QQid) {
	// https://qlogo3.store.qq.com/qzone/(%QQID%)/(%QQID%)/640.jfif		//OK
	// https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=(%QQID%)&spec=640		//OK
	// https://q1.qlogo.cn/g?b=qq&nk=(%QQID%)&s=640
	// https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=(%QQID%)&spec=640
	return "https://q1.qlogo.cn/g?b=qq&nk=" + to_string(QQid) + "&s=640";
}

// 私有成员函数
size_t curl_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	((string*)userdata)->append(ptr, size * nmemb);
	return size * nmemb;
}
int QQBot::SendGETRequest(const string& URL, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		Qlog.Error() << "CURL init faild." << endl;
		return -1;
	}
	try {
		if (!access_token_.empty()) {										// 消息报头
			struct curl_slist* headers = NULL;
			string token_header = "Authorization: Bearer " + access_token_;
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
					Qlog.Error() << "From go-cqhttp: Missing access token!" << endl;
					break;
				}
				else if (reply_code == 402) {
					Qlog.Error() << "From go-cqhttp: Wrong access token!" << endl;
					break;
				}
				else if (reply_code == 403) {
					Qlog.Error() << "From go-cqhttp: Content-Type unsupport!" << endl;
					break;
				}
				else if (reply_code == 404) {
					Qlog.Error() << "From go-cqhttp: This API is not exist: " << URL << endl;
					break;
				}
			}
			else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
				Qlog.Warn() << "Waiting go-cqhttp time out!" << endl;
				break;
			}
			else {
				Qlog.Error() << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code << endl;
				break;
			}
			curl_easy_cleanup(handle);
			return 0;
		} while (false);
		curl_easy_cleanup(handle);
		return -1;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendGETRequest(const string&, string&)." << endl;
		curl_easy_cleanup(handle);
		return -1;
	}
}
int QQBot::SendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		Qlog.Warn() << "CURL init faild." << endl;
		return -1;
	}
	try {
		curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());					// 设置URL
		curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "POST");			// 设置请求方法
		curl_easy_setopt(handle, CURLOPT_POSTFIELDS, send_buffer.c_str());
		struct curl_slist* headers = NULL;									// 消息报头
		headers = curl_slist_append(headers, "Content-Type: application/json,application/octet-stream");
		if (!access_token_.empty()) {
			string token_header = "Authorization: Bearer " + access_token_;
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
					Qlog.Error() << "From go-cqhttp: Missing access token!" << endl;
					break;
				}
				else if (reply_code == 402) {
					Qlog.Error() << "From go-cqhttp: Wrong access token!" << endl;
					break;
				}
				else if (reply_code == 403) {
					Qlog.Error() << "From go-cqhttp: Content-Type unsupport!" << endl;
					break;
				}
				else if (reply_code == 404) {
					Qlog.Error() << "From go-cqhttp: This API is not exist: " << URL << endl;
					break;
				}
			}
			else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
				Qlog.Warn() << "Waiting go-cqhttp time out!" << endl;
				break;
			}
			else {
				Qlog.Error() << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code << endl;
				break;
			}
			curl_easy_cleanup(handle);
			return 0;
		} while (false);
		curl_easy_cleanup(handle);
		return -1;
	}
	catch (...) {
		Qlog.Error() << "Exception in QQBot function SendPOSTRequest(const string&, const string&, string&)." << endl;
		curl_easy_cleanup(handle);
		return -1;
	}
}
vector<QQFriend>::iterator QQBot::FindQQFriend(unsigned int friend_id) {
	vector<QQFriend>::iterator iter = QQBot_friend_list_.begin();
	for (; iter != QQBot_friend_list_.end(); iter++) {
		if (iter->id_ == friend_id) return iter;
	}
	return iter;
}
vector<QQUFriend>::iterator QQBot::FindQQUFriend(unsigned int ufriend_id) {
	vector<QQUFriend>::iterator iter = QQBot_Ufriend_list_.begin();
	for (; iter != QQBot_Ufriend_list_.end(); iter++) {
		if (iter->id_ == ufriend_id) return iter;
	}
	return iter;
}
vector<QQGroup>::iterator QQBot::FindQQGroup(unsigned int group_id) {
	vector<QQGroup>::iterator iter = QQBot_group_list_.begin();
	for (; iter != QQBot_group_list_.end(); iter++) {
		if (iter->id_ == group_id) return iter;
	}
	return iter;
}
