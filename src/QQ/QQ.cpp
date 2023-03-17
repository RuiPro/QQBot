#include <curl.h>
#include <chrono>
#include "QQ.h"

int QQBot::PrintCqhttpVersion() {
	if (cqhttp_addr_.empty()) return -1;
	std::cout << "\033[34m[Info]\033[0m\t" << "Getting go-cqhttp version..." << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/get_version_info";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		string version = json_data["data"]["app_version"];
		version.pop_back();
		version.erase(0, 1);
		std::cout << "\033[34m[Info]\033[0m\t" << "Using go-cqhttp: \033[1m" << version << std::endl;
	}
	else if (json_data["status"] == "failed") {
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

int QQBot::SetAdmin(unsigned int admin_id) {
	if (QQBot_friend_list_.find(QQFriend(admin_id)) != QQBot_friend_list_.end()) {
		administrator_ = QQFriend(admin_id);
	}
	else {
		std::cout << "\033[31m[Error]\033[0m\t" << "The administrator is not a friend of QQ bot, setting failed." << std::endl;
		return -1;
	}
	return 0;
} 
unsigned int QQBot::GetQQbotID(){
	return QQBot_id_;
}
string QQBot::GetQQbotNickname() {
	return QQBot_nickname_;
}
bool QQBot::HasAdmin() {
	if (administrator_.id_ == 0)return false;
	return true;
}
QQFriend QQBot::GetAdmin() {
	return administrator_;
}
int QQBot::GetFriendNum() {
	return QQBot_friend_list_.size();
}
int QQBot::GetGroupNum() {
	return QQBot_group_list_.size();
}
void QQBot::PrintFriendList() {
	cout << " ┌───────── \033[34mQQ friends list\033[0m" << endl;
	for (auto& qfriend : QQBot_friend_list_) {
		cout << " ├─ " << qfriend.name_ << "(" << qfriend.id_ << ")" << endl;
	}
	cout << " └───────── \033[34mTotal num: " << QQBot_friend_list_.size() << "\033[0m" << endl;
}
void QQBot::PrintGroupList() {
	cout << " ┌───────── \033[34mQQ groups list\033[0m" << endl;
	for (auto& group : QQBot_group_list_) {
		cout << " ├─ " << group.name_ << "(" << group.id_ << ")" << endl;
	}
	cout << " └───────── \033[34mTotal num: " << QQBot_group_list_.size() << "\033[0m" << endl;
}

int QQBot::GetBasicInfo() {
	if (cqhttp_addr_.empty()) return -1;
	std::cout << "\033[34m[Info]\033[0m\t" << "Getting QQBot information...   " << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/get_login_info";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		QQBot_id_ = json_data["data"]["user_id"];
		QQBot_nickname_ = json_data["data"]["nickname"];
		std::cout << "\033[34m[Info]\033[0m\t" << "Received Bot information:" << QQBot_nickname_ << "(" << QQBot_id_ << ")" << std::endl;
	}
	else if (json_data["status"] == "failed") {
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

int QQBot::GetFriendList() {
	if (cqhttp_addr_.empty()) return -1;
	std::cout << "\033[34m[Info]\033[0m\t" << "Getting QQBot friend list...   " << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/get_friend_list";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		QQBot_friend_list_.clear();
		for (auto& x : json_data["data"]) {
			QQBot_friend_list_.insert(QQFriend(x["user_id"], x["nickname"], x["remark"]));
		}
		std::cout << "\033[34m[Info]\033[0m\t" << "Bot's number of friends:" << QQBot_friend_list_.size() << std::endl;
	}
	else if (json_data["status"] == "failed") {
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

int QQBot::GetGroupList() {
	if (cqhttp_addr_.empty()) return -1;
	std::cout << "\033[34m[Info]\033[0m\t" << "Getting QQBot group list...   " << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/get_group_list";
	string data_buffer;
	if (SendGETRequest(URL, data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		QQBot_group_list_.clear();
		for (auto& x : json_data["data"]) {
			QQGroup group(x["group_id"], x["group_name"], x["member_count"], x["max_member_count"]);
			GetGroupMemberList(group);
			QQBot_group_list_.insert(group);
		}
		std::cout << "\033[34m[Info]\033[0m\t" << "Bot's number of groups:" << QQBot_group_list_.size() << std::endl;
	}
	else if (json_data["status"] == "failed") {
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

int QQBot::GetAllinfo() {
	if (cqhttp_addr_.empty()) {
		std::cout << "\033[31m[Error]\033[0m\t" << "go-cqhttp address not set!" << std::endl;
		return -1;
	}
	if (GetBasicInfo() != 0) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Failed to get QQBot information from go-cqhttp!" << std::endl;
		return -1;
	}
	if (GetFriendList() != 0) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Failed to get QQBot friend list from go-cqhttp!" << std::endl;
		return -1;
	}
	if (GetGroupList() != 0) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Failed to get QQBot group list from go-cqhttp!" << std::endl;
		return -1;
	}
	return 0;
}

int QQBot::GetGroupMemberList(QQGroup& group) {
	if (cqhttp_addr_.empty()) return -1;
	string URL = "http://" + cqhttp_addr_ + "/get_group_member_list";
	string data_buffer;
	json post_data;
	post_data["group_id"] = group.id_;
	post_data["no_cache"] = false;
	if (SendPOSTRequest(URL, post_data.dump(), data_buffer) != 0) return -1;
	json json_data = json::parse(data_buffer, NULL, false);
	if (json_data["status"] == "ok") {
		group.group_member_list_.clear();
		for (auto& x : json_data["data"]) {
			group.group_member_list_.insert(QQGroupMember(x["user_id"], x["nickname"], x["card"], x["group_id"], x["title"]));
		}
	}
	else if (json_data["status"] == "failed") {
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

string QQBot::GetAccessToken() const {
	return access_token_;
}

int QQBot::SendPrivateMsg(const QQFriend& qfriend, QQMessage& msg) {
	if (cqhttp_addr_.empty()) return -1;
	if (msg.IsGroupMsg()) return -1;
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
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

int QQBot::SendGroupMsg(const QQGroup& group, QQMessage& msg) {
	if (cqhttp_addr_.empty()) return -1;
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
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return -1;
	}
	return 0;
}

size_t curl_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	((std::string*)userdata)->append(ptr, size * nmemb);
	return size * nmemb;
}

int QQBot::SendGETRequest(const string& URL, string& recv_buffer) {
	CURL* handle = curl_easy_init();
	if (handle == nullptr) {
		std::cout << "\033[31m[Error]\033[0m\t" << "CURL init faild." << std::endl;
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
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: Missing access token!" << std::endl;
				break;
			}
			else if (reply_code == 402) {
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: Wrong access token!" << std::endl;
				break;
			}
			else if (reply_code == 403) {
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: Content-Type unsupport!" << std::endl;
				break;
			}
			else if (reply_code == 404) {
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: This API is not exist: " << URL << std::endl;
				break;
			}
		}
		else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
			std::cout << "\033[33m[Warn]\033[0m\t" << "Waiting go-cqhttp time out!" << std::endl;
			break;
		}
		else {
			std::cout << "\033[31m[Error]\033[0m\t" << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code << std::endl;
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
		std::cout << "\033[33m[Warn]\033[0m\t" << "CURL init faild." << std::endl;
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
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: Missing access token!" << std::endl;
				break;
			}
			else if (reply_code == 402) {
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: Wrong access token!" << std::endl;
				break;
			}
			else if (reply_code == 403) {
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: Content-Type unsupport!" << std::endl;
				break;
			}
			else if (reply_code == 404) {
				std::cout << "\033[31m[Error]\033[0m\t" << "From go-cqhttp: This API is not exist: " << URL << std::endl;
				break;
			}
		}
		else if (ret_code == CURLE_OPERATION_TIMEDOUT) {
			std::cout << "\033[33m[Warn]\033[0m\t" << "Waiting go-cqhttp time out!" << std::endl;
			break;
		}
		else {
			std::cout << "\033[31m[Error]\033[0m\t" << "Failed to communicate with go-cqhttp. Curl return code: " << ret_code << std::endl;
			break;
		}
		curl_easy_cleanup(handle);
		return 0;
	} while (false);
	curl_easy_cleanup(handle);
	return -1;
}

int QQBot::Poke(const QQGroup& group, const QQGroupMember& member) {
	if (cqhttp_addr_.empty()) return -1;
	if (group.group_member_list_.find(member) == group.group_member_list_.end()) return -1;
	string poke = "[CQ:poke,qq=" + to_string(member.id_) + "]";
	string URL = "http://" + cqhttp_addr_ + "/send_group_msg";
	json send_json;
	send_json["group_id"] = group.id_;
	send_json["message"] = poke;
	send_json["auto_escape"] = false;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	return 0;
}

int QQBot::Poke(const QQFriend& qfriend) {
	if (cqhttp_addr_.empty()) return -1;
	if (QQBot_friend_list_.find(qfriend) == QQBot_friend_list_.end()) return -1;
	string poke = "[CQ:poke,qq=" + to_string(qfriend.id_) + "]";
	string URL = "http://" + cqhttp_addr_ + "/send_group_msg";
	json send_json;
	send_json["user_id"] = qfriend.id_;
	send_json["message"] = poke;
	send_json["auto_escape"] = false;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	return 0;
}

int QQBot::DeleteFriend(const QQFriend& qfriend) {
	if (cqhttp_addr_.empty()) return -1;
	std::cout << "\033[34m[Info]\033[0m\t" << "Delete friend " << qfriend.name_ << "(" << qfriend.id_ << ")" << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/delete_friend";
	json send_json;
	send_json["user_id"] = qfriend.id_;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	QQBot_friend_list_.erase(qfriend);
	return 0;
}

int QQBot::DeleteGroup(const QQGroup& group) {
	if (cqhttp_addr_.empty()) return -1;
	std::cout << "\033[33m[Warn]\033[0m\t" << "leave group " << group.name_ << "(" << group.id_ << ")" << std::endl;
	string URL = "http://" + cqhttp_addr_ + "/set_group_leave";
	json send_json;
	send_json["group_id"] = group.id_;
	string send_buffer = send_json.dump();
	string data_buffer;
	if (SendPOSTRequest(URL, send_buffer, data_buffer) != 0) return -1;
	QQBot_group_list_.erase(group);
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
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
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
		std::cout << "\033[33m[Warn]\033[0m\t" << "Failed to use go-cqhttp's API: " << URL << std::endl << "    --->"
			<< json_data["msg"] << ":" << json_data["wording"] << std::endl;
		return false;
	}
	return true;
}

string QQBot::GetQQHeaderImageURL(unsigned int qq_id) {
	// https://qlogo3.store.qq.com/qzone/(%QQID%)/(%QQID%)/640.jfif		//OK
	// https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=(%QQID%)&spec=640		//OK
	return "https://q2.qlogo.cn/headimg_dl.jfif?dst_uin=" + to_string(qq_id) + "&spec=640";
}