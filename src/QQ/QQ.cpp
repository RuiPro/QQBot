#include "QQ.h"
#include "QQ.h"
#include "QQ.h"
#include "QQ.h"
#include "QQ.h"
#include "QQ.h"
#include <curl.h>
#include <chrono>
#include "QQ.h"
#include "QQ.h"

vector<QQGroupMember>::iterator QQGroup::FindMember(unsigned int QQid) {
	vector<QQGroupMember>::iterator iter = group_member_list_.begin();
	for (; iter != group_member_list_.end(); iter++) {
		if (iter->id_ == QQid) return iter;
	}
	return iter;
}

bool QQGroup::GetGroupHasMember(unsigned int QQid) const  {
	auto iter = group_member_list_.cbegin();
	for (; iter != group_member_list_.end(); iter++) {
		if (iter->id_ == QQid) return true;
	}
	return false;
}

QQFriend QQBot::NULLFriend = QQFriend(0);
QQGroup QQBot::NULLGroup = QQGroup(0);

unsigned int QQBot::GetBotID() {
	QQ_lock_.lock();
	unsigned int ret = QQBot_id_;
	QQ_lock_.unlock();
	return ret;
}
string QQBot::GetBotNickname() {
	QQ_lock_.lock();
	string ret = QQBot_nickname_;
	QQ_lock_.unlock();
	return ret;
}
bool QQBot::GetBotHasAdmin() {
	bool ret = true;
	QQ_lock_.lock();
	if (administrator_.id_ == 0) ret = false;
	QQ_lock_.unlock();
	return ret;
}
QQFriend QQBot::GetBotAdmin() {
	QQ_lock_.lock();
	QQFriend ret = administrator_;
	QQ_lock_.unlock();
	return ret;
}
int QQBot::GetBotFriendNum() {
	QQ_lock_.lock();
	int ret = QQBot_friend_list_.size();
	QQ_lock_.unlock();
	return ret;
}
int QQBot::GetBotGroupNum() {
	QQ_lock_.lock();
	int ret = QQBot_group_list_.size();
	QQ_lock_.unlock();
	return ret;
}
bool QQBot::GetBotHasQQFriend(unsigned int friend_id) {
	QQ_lock_.lock();
	auto iter = FindQQFriend(friend_id);
	QQ_lock_.unlock();
	if (iter != QQBot_friend_list_.end())return true;
	return false;
}
bool QQBot::GetBotHasQQGroup(unsigned int group_id) {
	QQ_lock_.lock();
	auto iter = FindQQGroup(group_id);
	QQ_lock_.unlock();
	if (iter != QQBot_group_list_.end())return true;
	return false;
}
const QQFriend& QQBot::GetBotFriendInfo(unsigned int friend_id) {
	QQ_lock_.lock();
	auto iter = FindQQFriend(friend_id);
	QQ_lock_.unlock();
	if (iter != QQBot_friend_list_.end())return (*iter);
	return NULLFriend;
}
const QQGroup& QQBot::GetBotGroupInfo(unsigned int group_id) {
	QQ_lock_.lock();
	auto iter = FindQQGroup(group_id);
	QQ_lock_.unlock();
	if (iter != QQBot_group_list_.end())return (*iter);
	return NULLGroup;
}
const vector<unsigned int> QQBot::GetBotGroupIDList() {
	vector<unsigned int> ret;
	QQ_lock_.lock();
	for (auto& value : QQBot_group_list_) {
		ret.push_back(value.id_);
	}
	QQ_lock_.unlock();
	return ret;
}
const vector<unsigned int> QQBot::GetBotFriendIDList() {
	vector<unsigned int> ret;
	QQ_lock_.lock();
	for (auto& value : QQBot_friend_list_) {
		ret.push_back(value.id_);
	}
	QQ_lock_.unlock();
	return ret;
}
string QQBot::GetBotAccessToken() const {
	return access_token_;
}

int QQBot::SetBotAdmin(unsigned int admin_id) {
	QQ_lock_.lock();
	if (FindQQFriend(admin_id) != QQBot_friend_list_.end()) {
		administrator_ = QQFriend(admin_id);
	}
	else {
		Qlog.Error() << "The administrator is not a friend of QQ bot, setting failed." << std::endl;
		QQ_lock_.unlock();
		return -1;
	}
	QQ_lock_.unlock();
	return 0;
}
int QQBot::SetBotBasicInfo() {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_login_info";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		QQ_lock_.lock();
		QQBot_id_ = json_data["data"]["user_id"];
		QQBot_nickname_ = json_data["data"]["nickname"];
		Qlog.Info() << "Bot information: " << QQBot_nickname_ << "(" << QQBot_id_ << ")" << std::endl;
		QQ_lock_.unlock();
	}
	else if (json_data["status"] == "failed") {
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetBotFriendList() {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_friend_list";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		QQ_lock_.lock();
		QQBot_friend_list_.clear();
		for (auto& element : json_data["data"]) {
			QQBot_friend_list_.push_back(QQFriend(element["user_id"], element["nickname"], element["remark"]));
		}
		Qlog.Info() << "Bot's number of friends: " << QQBot_friend_list_.size() << std::endl;
		QQ_lock_.unlock();
	}
	else if (json_data["status"] == "failed") {
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetBotGroupList() {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_group_list";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		QQ_lock_.lock();
		QQBot_group_list_.clear();
		for (auto& element : json_data["data"]) {
			QQGroup group(element["group_id"], element["group_name"], element["member_count"], element["max_member_count"]);
			QQBot_group_list_.push_back(std::move(group));
		}
		QQ_lock_.unlock();
		for (auto& group : QQBot_group_list_) {
			SetGroupMemberList(group.id_);
		}
		for (auto& group : QQBot_group_list_) {
			for (auto& member : group.group_member_list_) {
				SetGroupMemberInfo(group.id_, member.id_);
			}
		}
		QQ_lock_.lock();
		Qlog.Info() << "Bot's number of groups: " << QQBot_group_list_.size() << std::endl;
		QQ_lock_.unlock();
	}
	else if (json_data["status"] == "failed") {
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetBotAllInfo() {
	if (cqhttp_addr_.empty()) {
		Qlog.Error() << "go-cqhttp address not set!" << std::endl;
		return -1;
	}
	if (SetBotBasicInfo() != 0) {
		Qlog.Error() << "Failed to get QQBot information from go-cqhttp!" << std::endl;
		return -1;
	}
	if (SetBotFriendList() != 0) {
		Qlog.Error() << "Failed to get QQBot friend list from go-cqhttp!" << std::endl;
		return -1;
	}
	if (SetBotGroupList() != 0) {
		Qlog.Error() << "Failed to get QQBot group list from go-cqhttp!" << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetGroupInfo(QQGroup& group) {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_group_info";
	string data_buffer;
	json post_data;
	post_data["group_id"] = group.id_;
	post_data["no_cache"] = !use_cache;
	if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		group.name_ = json_data["data"]["group_name"];
		group.member_count_ = json_data["data"]["member_count"];
		group.max_member_count_ = json_data["data"]["max_member_count"];
	}
	else if (json_data["status"] == "failed") {
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetGroupMemberList(unsigned int group_id) {
	if (cqhttp_addr_.empty()) return -1;
	if (group_id == 0) return -1;
	QQ_lock_.lock();
	auto group_iter = FindQQGroup(group_id);
	if(group_iter == QQBot_group_list_.end()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_group_member_list";
	string data_buffer;
	json post_data;
	post_data["group_id"] = group_id;
	post_data["no_cache"] = !use_cache;
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
			group_iter->group_member_list_.push_back(std::move(member));
		}
		QQ_lock_.unlock();
	}
	else if (json_data["status"] == "failed") {
		QQ_lock_.unlock();
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetGroupMemberInfo(unsigned int group_id, unsigned int member_id) {
	if (cqhttp_addr_.empty()) return -1;
	QQ_lock_.lock();
	auto group_iter = FindQQGroup(group_id);
	if (group_iter == QQBot_group_list_.end()) {
		QQ_lock_.unlock();
		return 0;
	};
	if (!group_iter->GetGroupHasMember(member_id)) {
		QQ_lock_.unlock();
		return 0; 
	}
	auto member_iter = group_iter->FindMember(member_id);
	string URL = "http://" + cqhttp_addr_ + "/get_group_member_info";
	string data_buffer;
	json post_data;
	post_data["group_id"] = group_id;
	post_data["user_id"] = member_id;
	post_data["no_cache"] = !use_cache;
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
		QQ_lock_.unlock();
	}
	else if (json_data["status"] == "failed") {
		QQ_lock_.unlock();
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::ResetBotGroup(unsigned int group_id) {
	if (group_id == 0) return -1;
	QQ_lock_.lock();
	auto iter = FindQQGroup(group_id);
	if (iter == QQBot_group_list_.end()) {
		QQ_lock_.unlock();
		return -1;
	}
	SetGroupInfo(*iter);
	QQ_lock_.unlock();
	SetGroupMemberList(group_id);
	for (auto& member : iter->group_member_list_) {
		SetGroupMemberInfo(iter->id_, member.id_);
	}
	return 0;
}

int QQBot::SendPrivateMsg(const QQFriend& qfriend, QQMessage& msg) {
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
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SendGroupMsg(const QQGroup& group, QQMessage& msg) {
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
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
void QQBot::PrintFriendList() {
	Qlog.Info() << " ┌───────── \033[34mQQ friends list\033[0m" << endl;
	QQ_lock_.lock();
	for (auto& qfriend : QQBot_friend_list_) {
		Qlog.Info() << " ├─ " << qfriend.name_ << "(" << qfriend.id_ << ")" << endl;
	}
	Qlog.Info() << " \033[34mTotal num: " << QQBot_friend_list_.size() << "\033[0m" << endl;
	QQ_lock_.unlock();
}
void QQBot::PrintGroupList() {
	Qlog.Info() << " ┌───────── \033[34mQQ groups list\033[0m" << endl;
	QQ_lock_.lock();
	for (auto& group : QQBot_group_list_) {
		Qlog.Info() << " ├─ " << group.name_ << "(" << group.id_ << ")" << endl;
	}
	Qlog.Info() << " \033[34mTotal num: " << QQBot_group_list_.size() << "\033[0m" << endl;
	QQ_lock_.unlock();
}
int QQBot::DeleteFriend(const QQFriend& qfriend) {
	if (cqhttp_addr_.empty()) return -1;
	QQ_lock_.lock();
	auto iter = FindQQFriend(qfriend.id_);
	QQ_lock_.unlock();
	if (iter == QQBot_friend_list_.end()) return 0;
	Qlog.Info() << "Delete friend " << qfriend.name_ << "(" << qfriend.id_ << ")" << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/delete_friend";
	json send_json;
	send_json["user_id"] = qfriend.id_;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	QQ_lock_.lock();
	QQBot_friend_list_.erase(iter);
	QQ_lock_.unlock();
	return 0;
}
int QQBot::DeleteGroup(const QQGroup& group) {
	if (cqhttp_addr_.empty()) return -1;
	QQ_lock_.lock();
	auto iter = FindQQGroup(group.id_);
	QQ_lock_.unlock();
	if (iter == QQBot_group_list_.end()) return 0;
	Qlog.Warn() << "leave group " << group.name_ << "(" << group.id_ << ")" << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/set_group_leave";
	json send_json;
	send_json["group_id"] = group.id_;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	QQ_lock_.lock();
	QQBot_group_list_.erase(iter);
	QQ_lock_.unlock();
	return 0;
}
int QQBot::WithdrawMsg(int message_id) {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/delete_msg";
	json send_json;
	send_json["message_id"] = message_id;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	return 0;
}
bool QQBot::CanSendImage() {
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
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return false;
	}
	return true;
}
bool QQBot::CanSendRecord() {
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
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return false;
	}
	return true;
}
int QQBot::PrintCqhttpVersion() {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_version_info";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		string version = json_data["data"]["version"];
		version.pop_back();
		version.erase(0, 1);
		Qlog.Info() << "Using go-cqhttp: " << version << std::endl;
	}
	else if (json_data["status"] == "failed") {
		Qlog.Warn() << "Failed to use go-cqhttp's API: " << URL << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}
int QQBot::SetQQRrofile(const string& nickname, const string& company, const string& email, const string& college, const string& personal_note) {
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
QQUser QQBot::GetQQUserInfo(unsigned int QQid) {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_stranger_info";
	string data_buffer;
	json post_data;
	post_data["user_id"] = QQid;
	post_data["no_cache"] = !use_cache;
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
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return user;
	}
	return user;
}
QQRawMessage QQBot::GetMessageInfo(int message_id) {
	QQRawMessage msg;
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
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return msg;
	}
	return msg;
}
int QQBot::SetAddFriendRequest(const string& flag, const bool approve, const string& remark) {
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
int QQBot::SetAddGroupRequest(const string& flag, const string& sub_type, const bool approve, const string& reason) {
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
string QQBot::GetQQHeaderImageURL(unsigned int QQid) {
	// https://qlogo3.store.qq.com/qzone/(%QQID%)/(%QQID%)/640.jfif		//OK
	// https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=(%QQID%)&spec=640		//OK
	return "https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=" + to_string(QQid) + "&spec=640";
}

size_t curl_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	((std::string*)userdata)->append(ptr, size * nmemb);
	return size * nmemb;
}
int QQBot::SendGETRequest(const string& URL, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		Qlog.Error() << "CURL init faild." << std::endl;
		return -1;
	}
	if (!access_token_.empty()) {										// 消息报头
		struct curl_slist* headers = NULL;
		string token_header = "Authorization: Bearer " + access_token_;
		headers = curl_slist_append(headers, token_header.c_str());
		curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
	}
	curl_easy_setopt(handle, CURLOPT_URL, URL.c_str());					// 设置URL
	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET");				// 设置请求方法

	std::string reply_data;												// 接收数据的容器
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
				Qlog.Error() << "From go-cqhttp: Missing access token!" << std::endl;
				break;
			}
			else if (reply_code == 402) {
				Qlog.Error() << "From go-cqhttp: Wrong access token!" << std::endl;
				break;
			}
			else if (reply_code == 403) {
				Qlog.Error() << "From go-cqhttp: Content-Type unsupport!" << std::endl;
				break;
			}
			else if (reply_code == 404) {
				Qlog.Error() << "From go-cqhttp: This API is not exist: " << URL << std::endl;
				break;
			}
		}
		else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
			Qlog.Warn() << "Waiting go-cqhttp time out!" << std::endl;
			break;
		}
		else {
			Qlog.Error() << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code << std::endl;
			break;
		}
		curl_easy_cleanup(handle);
		return 0;
	} while (false);
	curl_easy_cleanup(handle);
	return -1;
}
int QQBot::SendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		Qlog.Warn() << "CURL init faild." << std::endl;
		return -1;
	}
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
	std::string reply_data;												// 接收数据的容器
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
				Qlog.Error() << "From go-cqhttp: Missing access token!" << std::endl;
				break;
			}
			else if (reply_code == 402) {
				Qlog.Error() << "From go-cqhttp: Wrong access token!" << std::endl;
				break;
			}
			else if (reply_code == 403) {
				Qlog.Error() << "From go-cqhttp: Content-Type unsupport!" << std::endl;
				break;
			}
			else if (reply_code == 404) {
				Qlog.Error() << "From go-cqhttp: This API is not exist: " << URL << std::endl;
				break;
			}
		}
		else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
			Qlog.Warn() << "Waiting go-cqhttp time out!" << std::endl;
			break;
		}
		else {
			Qlog.Error() << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code << std::endl;
			break;
		}
		curl_easy_cleanup(handle);
		return 0;
	} while (false);
	curl_easy_cleanup(handle);
	return -1;
}
vector<QQFriend>::iterator QQBot::FindQQFriend(unsigned int friend_id) {
	vector<QQFriend>::iterator iter = QQBot_friend_list_.begin();
	for (; iter != QQBot_friend_list_.end(); iter++) {
		if (iter->id_ == friend_id) return iter;
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
