#ifndef QQ_H
#define QQ_H

#define LIBQQ_VERSION "0.9.2"

#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include <unordered_set>
#include "QQMessage.h"
#include "../json/json.hpp"
using json = nlohmann::json;
using namespace std;

class QQMessage;

// 抽象QQ类
class QQObject {
public:
	inline QQObject() {}
	inline QQObject(unsigned int user_id) : id_(user_id) {}
	inline QQObject(unsigned int user_id, const string& user_nickname) : id_(user_id), name_(user_nickname) {}
	unsigned int id_;
	string name_;
};

// QQ好友
class QQFriend : public QQObject {
public:
	inline QQFriend() {}
	inline QQFriend(unsigned int user_id) : QQObject(user_id) {}
	inline QQFriend(unsigned int user_id, const string& user_nickname) : QQObject(user_id, user_nickname) {}
	inline QQFriend(unsigned int user_id, const string& user_nickname, const string& friend_remark) : QQObject(user_id, user_nickname), friend_remark_(friend_remark) {}
	string friend_remark_;
};

// QQ群成员
class QQGroupMember : public QQObject {
public:
	inline QQGroupMember() {}
	inline QQGroupMember(unsigned int user_id) : QQObject(user_id) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname) : QQObject(user_id, user_nickname) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname, const string& group_nickname) : QQObject(user_id, user_nickname), group_nickname_(group_nickname) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname, const string& group_nickname, unsigned int in_group_id) :
		QQObject(user_id, user_nickname), group_nickname_(group_nickname), in_group_id_(in_group_id) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname, const string& group_nickname, unsigned int in_group_id, const string& group_title) :
		QQObject(user_id, user_nickname), group_nickname_(group_nickname), in_group_id_(in_group_id), group_title_(group_title) {}
	string group_nickname_;
	unsigned int in_group_id_;
	string group_title_;
};

// 使用QQ号对好友集合和群集合容器进行哈希和判等
class QQObjectHash {
public:
	std::size_t operator()(const QQObject& obj) const {
		return std::hash<int>{}(obj.id_);
	}
};
class QQObjectEqual {
public:
	bool operator()(const QQObject& lhs, const QQObject& rhs) const {
		return lhs.id_ == rhs.id_;
	}
};

// QQ群
class QQGroup : public QQObject {
public:
	inline QQGroup() {}
	inline QQGroup(unsigned int group_id) : QQObject(group_id) {}
	inline QQGroup(unsigned int group_id, const string& group_name) : QQObject(group_id, group_name) {}
	inline QQGroup(unsigned int group_id, const string& group_name, short member_count, short max_member_count) :
		QQObject(group_id, group_name), member_count_(member_count), max_member_count_(max_member_count) {}
	short member_count_;
	short max_member_count_;
	unordered_set<QQGroupMember, QQObjectHash, QQObjectEqual> group_member_list_;
};

// 用于绑定一个go-cqhttp登录的QQ号
class QQBot {
public:
	inline QQBot(const string& cqhttp_addr) : cqhttp_addr_(cqhttp_addr) {};
	inline QQBot(const string& cqhttp_addr, const string& access_token) : cqhttp_addr_(cqhttp_addr), access_token_(access_token) {};
	
	int PrintCqhttpVersion();										// 打印go-cqhttp的版本
	int SetAdmin(unsigned int admin_id);							// 设置管理员

	unsigned int GetQQbotID();
	string GetQQbotNickname();
	bool HasAdmin();
	QQFriend GetAdmin();
	int GetFriendNum();
	int GetGroupNum();
	void PrintFriendList();
	void PrintGroupList();
	int GetBasicInfo();												// 获取QQ号和QQ昵称
	int GetFriendList();											// 获取QQ好友列表，使用map存储，哈希根据是QQ号
	int GetGroupList();												// 获取QQ群聊列表，使用map存储，哈希根据是群号
	int GetAllInfo();												// 以上三个API的封装
	int GetGroupMemberList(QQGroup& group);							// 获取群聊成员列表，使用map存储，哈希根据是QQ号
	string GetAccessToken() const;									// 获取配置的AccessToken
	int SendPrivateMsg(const QQFriend& qfriend, QQMessage& msg);	// 发送私聊信息
	int SendGroupMsg(const QQGroup& group, QQMessage& msg);			// 发送群聊信息
	int DeleteFriend(const QQFriend& qfriend);						// 删除好友
	int DeleteGroup(const QQGroup& group);							// 退出群聊
	int WithdrawMsg(int message_id);								// 撤回一条消息
	bool CanSendImage();											// 判断QQ号是否可以发送图片？
	bool CanSendRecord();											// 判断QQ号是否可以发送录音？
	
	static string GetQQHeaderImageURL(unsigned int qq_id);			// 使用QQ号获取QQ头像链接
private:
	mutex QQ_lock_;
	unsigned int QQBot_id_ = 0;
	QQFriend administrator_ = QQFriend(0);
	string QQBot_nickname_;
	string cqhttp_addr_;
	unordered_set<QQFriend, QQObjectHash, QQObjectEqual> QQBot_friend_list_;
	unordered_set<QQGroup, QQObjectHash, QQObjectEqual> QQBot_group_list_;
	string access_token_;
	int SendGETRequest(const string& URL, string& recv_buffer);
	int SendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer);
};

#endif // !QQ_H
