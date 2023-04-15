#ifndef QQ_H
#define QQ_H

#define LIBQQ_VERSION "0.9.3"

#include <string>
#include <vector>
#include <iostream>
#include <mutex>
#include "QQMessage.h"
#include "../json/json.hpp"
#include "../QQBotLog/QQBotLog.h"
using json = nlohmann::json;
using namespace std;

class QQMessage;
class QQRawMessage;

// 抽象QQ类
class QQObject {
public:
	inline QQObject(unsigned int user_id) : id_(user_id) {}
	inline QQObject(unsigned int user_id, const string& user_nickname) : id_(user_id), name_(user_nickname) {}
	unsigned int id_ = 0;
	string name_;
	bool isNull() {
		if (id_ == 0) return true;
		return false;
	}
};

// QQ用户
class QQUser : public QQObject {
public:
	inline QQUser(unsigned int user_id) : QQObject(user_id) {}
	inline QQUser(unsigned int user_id, const string& user_nickname) : QQObject(user_id, user_nickname) {}
	inline QQUser(unsigned int user_id, const string& user_nickname, const string& friend_remark) : QQObject(user_id, user_nickname) {}
	char gender_;					// 性别，0=unknow，1=男，2=女
	unsigned short age_;			// 年龄
	string qid_;					// QQuid	
	unsigned short level_;			// 等级
	unsigned int login_days_;		// 达人日期？
};

// QQ好友
class QQFriend : public QQObject {
public:
	inline QQFriend(unsigned int user_id) : QQObject(user_id) {}
	inline QQFriend(unsigned int user_id, const string& user_nickname) : QQObject(user_id, user_nickname) {}
	inline QQFriend(unsigned int user_id, const string& user_nickname, const string& friend_remark) : QQObject(user_id, user_nickname), friend_remark_(friend_remark) {}
	string friend_remark_;
};

// QQ群成员
class QQGroupMember : public QQObject {
public:
	inline QQGroupMember(unsigned int user_id) : QQObject(user_id) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname) : QQObject(user_id, user_nickname) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname, const string& group_nickname) : QQObject(user_id, user_nickname), group_nickname_(group_nickname) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname, const string& group_nickname, unsigned int in_group_id) :
		QQObject(user_id, user_nickname), group_nickname_(group_nickname), in_group_id_(in_group_id) {}
	inline QQGroupMember(unsigned int user_id, const string& user_nickname, const string& group_nickname, unsigned int in_group_id, const string& group_title) :
		QQObject(user_id, user_nickname), group_nickname_(group_nickname), in_group_id_(in_group_id), group_title_(group_title) {}
	string group_nickname_;				// 群名片
	unsigned int in_group_id_;			// 在哪个群
	string group_title_;				// 群头衔
	char role_;							// 角色，member为0，admin为1，owner为2

	// 这些是必须通过获取详情才能获取的信息
	int age_ = 0;
	string area_;
	int join_time_ = 0;
	int last_active_time_ = 0;
	string level_;
	bool unfriendly_ = false;
	long title_expire_time_ = 0;
	bool card_changeable_ = false;
	long shut_up_timestamp_ = 0;
};

// QQ群
class QQGroup : public QQObject {
public:
	inline QQGroup(unsigned int group_id) : QQObject(group_id) {}
	inline QQGroup(unsigned int group_id, const string& group_name) : QQObject(group_id, group_name) {}
	inline QQGroup(unsigned int group_id, const string& group_name, short member_count, short max_member_count) :
		QQObject(group_id, group_name), member_count_(member_count), max_member_count_(max_member_count) {}
	vector<QQGroupMember>::iterator FindMember(unsigned int QQid);			// 找群友
	bool GetGroupHasMember(unsigned int QQid) const;						// 判断是否拥有群友
	short member_count_;													// 群人数
	short max_member_count_;												// 群最大可达人数
	vector<QQGroupMember> group_member_list_;								// 群成员列表
};

// 用于绑定一个go-cqhttp登录的QQ号
class QQBot {
public:
	inline QQBot(const string& cqhttp_addr) : cqhttp_addr_(cqhttp_addr) {};
	inline QQBot(const string& cqhttp_addr, const string& access_token) : cqhttp_addr_(cqhttp_addr), access_token_(access_token) {};
	QQBot(const QQBot&) = delete;
	QQBot(QQBot&&) = delete;
	
	// 这些函数用以获取Bot中已设置的信息
	unsigned int GetBotID();										// 获取Bot的QQ号
	string GetBotNickname();										// 获取Bot的昵称
	bool GetBotHasAdmin();											// 判断是否拥有管理员
	QQFriend GetBotAdmin();											// 获取管理员
	int GetBotFriendNum();											// 获取好友数量
	int GetBotGroupNum();											// 获取群数量
	bool GetBotHasQQFriend(unsigned int friend_id);					// 判断是否存在某个好友
	bool GetBotHasQQGroup(unsigned int group_id);					// 判断是否加了某个群
	const QQFriend& GetBotFriendInfo(unsigned int friend_id);		// 获取某个好友的信息
	const QQGroup& GetBotGroupInfo(unsigned int group_id);			// 获取某个群的信息
	const vector<unsigned int> GetBotGroupIDList();					// 获取群号列表，返回一个装有群号的容器
	const vector<unsigned int> GetBotFriendIDList();				// 获取好友列表，返回一个装有QQ号的容器
	string GetBotAccessToken() const;								// 获取配置的AccessToken

	// 这些函数用以设置Bot的信息，有些是手动设置，有些是从go-cqhttp获取
	int SetBotAdmin(unsigned int admin_id);							// 设置管理员【功能待完善】
	int SetBotBasicInfo();											// 从go-cqhttp获取Bot的QQ号和QQ昵称。使用互斥锁同步。
	int SetBotFriendList();											// 从go-cqhttp获取Bot的QQ好友列表。使用互斥锁同步。
	int SetBotGroupList();											// 从go-cqhttp获取Bot的QQ群聊列表。使用互斥锁同步。
	int SetBotAllInfo();											// 以上三个API的封装
	// 从go-cqhttp获取传入群的群信息。Bot可以不在群内，此时群成员等信息为空或0。不使用互斥锁，如传入Bot的群，需注意Bot信息的线程安全
	int SetGroupInfo(QQGroup& group);
	// 从go-cqhttp获取(更新)传入Bot的某个群的成员列表，Bot必须在群内。使用互斥锁同步。
	int SetGroupMemberList(unsigned int group_id);
	// 从go-cqhttp获取(更新)Bot的某个群的某个群员的详细信息，Bot必须在群内。使用互斥锁同步。
	int SetGroupMemberInfo(unsigned int group_id, unsigned int member_id);
	// 从go-cqhttp更新Bot的某个群，Bot必须在群内。内部就是上面三个函数的封装。使用互斥锁同步。
	int ResetBotGroup(unsigned int group_id);

	// 这些函数为功能性函数
	int SendPrivateMsg(const QQFriend& qfriend, QQMessage& msg);	// 发送私聊信息
	int SendGroupMsg(const QQGroup& group, QQMessage& msg);			// 发送群聊信息
	void PrintFriendList();											// 输出好友列表
	void PrintGroupList();											// 输出群列表
	int DeleteFriend(const QQFriend& qfriend);						// 删除好友
	int DeleteGroup(const QQGroup& group);							// 退出群聊
	int WithdrawMsg(int message_id);								// 撤回一条消息
	bool CanSendImage();											// 判断QQ号是否可以发送图片？
	bool CanSendRecord();											// 判断QQ号是否可以发送录音？
	int PrintCqhttpVersion();										// 打印go-cqhttp的版本
	// 设置Bot的信息，比如昵称
	int SetQQRrofile(const string& nickname, const string& company, const string& email, const string& college, const string& personal_note);
	QQUser GetQQUserInfo(unsigned int QQid);						// 获取QQ用户信息
	QQRawMessage GetMessageInfo(int message_id);					// 获取消息详情
	// 处理加好友请求，flag要从上报中获取
	int SetAddFriendRequest(const string& flag, const bool approve, const string& remark);
	// 处理加群请求，flag和sub_type要从上报中获取
	int SetAddGroupRequest(const string& flag, const string& sub_type, const bool approve, const string& reason);

	static string GetQQHeaderImageURL(unsigned int QQid);			// 使用QQ号获取QQ头像链接
private:
	mutex QQ_lock_;
	unsigned int QQBot_id_ = 0;
	QQFriend administrator_ = QQFriend(0);
	string QQBot_nickname_;
	string cqhttp_addr_;
	bool use_cache = true;
	vector<QQFriend> QQBot_friend_list_;
	vector<QQFriend>::iterator FindQQFriend(unsigned int friend_id);
	vector<QQGroup> QQBot_group_list_;
	vector<QQGroup>::iterator FindQQGroup(unsigned int group_id);
	string access_token_;
	int SendGETRequest(const string& URL, string& recv_buffer);
	int SendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer);
	static QQFriend NULLFriend;
	static QQGroup NULLGroup;
};

#endif // !QQ_H
