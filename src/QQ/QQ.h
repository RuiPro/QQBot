#ifndef QQ_H
#define QQ_H

#define LIBQQ_VERSION "0.9.3"

#include <string>
#include <vector>
#include <utility>
#include <mutex>
#include "QQStructure.h"
#include "QQMessage.h"
#include "../json/json.hpp"
#include "../QQBotLog/QQBotLog.h"
using namespace std;
using json = nlohmann::json;

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
	string friend_remark_;			// 好友备注名

};

// QQ单向好友QQ Unidirectional Friend。单向好友的产生原因：加Bot为好友设置了不用确认即可添加。
class QQUFriend : public QQObject {
public:
	inline QQUFriend(unsigned int user_id) : QQObject(user_id) {}
	inline QQUFriend(unsigned int user_id, const string& user_nickname) : QQObject(user_id, user_nickname) {}
	inline QQUFriend(unsigned int user_id, const string& user_nickname, const string& source) : QQObject(user_id, user_nickname), source_(source) {}
	string source_;					// 单向好友来源
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
	long last_active_time_ = 0;
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
	inline QQBot(const string& cqhttp_addr, const string& access_token, bool use_cache) : cqhttp_addr_(cqhttp_addr), access_token_(access_token), use_cache_(use_cache) {};
	QQBot(const QQBot&) = delete;
	QQBot(QQBot&&) = delete;

	// 这些函数用以获取Bot中已设置的信息
	unsigned int GetBotID();										// 获取Bot的QQ号
	string GetBotNickname();										// 获取Bot的昵称
	bool GetBotHasAdmin();											// 判断是否拥有管理员
	QQFriend GetBotAdmin();											// 获取管理员
	int GetBotFriendNum();											// 获取好友数量
	int GetBotUFriendNum();											// 获取单向好友数量
	int GetBotGroupNum();											// 获取群数量
	bool GetBotHasQQFriend(unsigned int friend_id);					// 判断是否存在某个好友
	bool GetBotHasQQUFriend(unsigned int friend_id);				// 判断是否存在某个单向好友
	bool GetBotHasQQGroup(unsigned int group_id);					// 判断是否加了某个群
	const QQFriend GetBotFriend(unsigned int friend_id);			// 获取某个好友的拷贝
	const QQGroup GetBotGroup(unsigned int group_id);				// 获取某个群的拷贝
	const vector<unsigned int> GetBotGroupIDList();					// 获取群号列表，返回一个装有群号的容器
	const vector<unsigned int> GetBotFriendIDList();				// 获取好友列表，返回一个装有QQ号的容器
	const vector<unsigned int> GetBotUFriendIDList();				// 获取单向好友列表，返回一个装有QQ号的容器
	string GetBotAccessToken() const;								// 获取配置的AccessToken
	void PrintFriendList();											// 输出好友列表
	void PrintUFriendList();										// 输出单向好友列表
	void PrintGroupList();											// 输出群列表

	// 这些函数用以设置Bot的信息
	int SetBotAdmin(unsigned int admin_id);							// 设置管理员【功能待完善】
	int SetBotBasicInfo();											// 从go-cqhttp获取Bot的QQ号和QQ昵称。使用互斥锁同步。
	int SetBotFriendList();											// 从go-cqhttp获取Bot的QQ好友列表。使用互斥锁同步。
	int SetBotUFriendList();										// 从go-cqhttp获取Bot的QQ好友列表。使用互斥锁同步。
	int SetBotGroupList();											// 从go-cqhttp获取Bot的QQ群聊列表。使用互斥锁同步。
	int SetBotAllInfo();											// 以上4个API的封装
	// 从go-cqhttp获取传入群的群信息。Bot可以不在群内，此时群成员等信息为空或0。不使用互斥锁，如传入Bot的群，需注意Bot信息的线程安全
	int SetGroupInfo(QQGroup& group);
	// 从go-cqhttp获取(更新)传入Bot的某个群的成员列表，Bot必须在群内。使用互斥锁同步。
	int SetGroupMemberList(unsigned int group_id);
	// 从go-cqhttp获取(更新)Bot的某个群的某个群员的详细信息，Bot必须在群内。使用互斥锁同步。
	int SetGroupMemberInfo(unsigned int group_id, unsigned int member_id);
	// 从go-cqhttp更新Bot的某个群，Bot必须在群内。内部就是上面三个函数的封装。使用互斥锁同步。
	int ResetBotGroup(unsigned int group_id);

	// 这些函数用以向go-cqhttp请求实现Bot的某些主动动作或获得一些信息
	int SendPrivateMsg(const QQFriend& qfriend, QQMessage& msg);			// 发送私聊信息
	int SendGroupMsg(const QQGroup& group, QQMessage& msg);					// 发送群聊信息
	int DeleteFriend(const QQFriend& qfriend);								// 删除好友
	int DeleteUFriend(const QQUFriend& ufriend);							// 删除单向好友
	int DeleteGroup(const QQGroup& group, bool dissolve = false);			// 退出群聊与解散，只有当bot是群主时解散有效
	int WithdrawMsg(int message_id);										// 撤回一条消息
	bool CanSendImage();													// 判断QQ号是否可以发送图片？暂不知这个API的存在作用
	bool CanSendRecord();													// 判断QQ号是否可以发送录音？暂不知这个API的存在作用
	int PrintCqhttpVersion();												// 打印go-cqhttp的版本
	// 设置Bot的信息，包括昵称、公司、邮箱、学校、个人说明
	int SetQQRrofile(const string& nickname, const string& company, const string& email, const string& college, const string& personal_note);
	QQUser GetQQUserInfo(unsigned int QQid);								// 获取一个QQ用户的信息
	QQRawMessage GetMessageInfo(int message_id);							// 获取消息的详情信息
	// 处理加好友请求，flag要从上报中获取
	int SetAddFriendRequest(const string& flag, const bool approve, const string& remark);
	// 处理加群请求，flag和sub_type要从上报中获取
	int SetAddGroupRequest(const string& flag, const string& sub_type, const bool approve, const string& reason);
	// 提供一个设备名，查看可以设置哪些在线机型名称及是否需要会员。
	// 比如提供小米MixAlpha，返回的列表里可能为小米MixAlpha(黑色)/true、小米MixAlpha(白色)/false...true表示需要氪会员
	vector<pair<string, bool>> GetDeviceShowList(const string& device_name);
	// 设置在线机型名称，需要从上面的GetDeviceShowList中选择一个元素传入device_name_element，非会员就选带false的元素
	// 比如传入小米MixAlpha、小米MixAlpha(白色)
	int SetDeviceShowName(const string& device_name, const string& device_name_element);
	vector<QQClient> GetOnlineClients();									// 获取Bot当前在线设备列表
	int MarkMsgAsRead(int message_id);										// 标记消息为已读
	vector<QQForwardMsgNode> GetForwardMsg(const string& forward_id);		// 获取转发消息的内容
	int SendPrivateForwardMsg(const QQFriend& qfriend, QQMessage& msg);		// 发送私聊合并信息
	int SendGroupeForwardMsg(const QQGroup& group, QQMessage& msg);			// 发送群聊合并信息
	vector<QQRawMessage> GetGroupHistoryMsg(const QQGroup& group, unsigned int msg_seq = 0);	// 从提供的起始消息序号获取前19条历史消息
	int GetImageInfo(const string& file, int& size, string& filename, string& url);				// 获取图片信息，第一个是传入，后三个是传出
	QQImageOCR GetImageOCR(const string& image_id);							// 【未实现】获取图片OCR信息
	string GetRecordInfo(const string& file, const string& out_format);		// 获取语音信息，传入收到的语音文件名和需传出的格式(mp3、amr、wma、m4a、spx、ogg、wav、flac)，传出语音文件的绝对路径
	QQGroupHonor GetGroupHonorInfo(const QQGroup& group, const string& type = "all");	// 获取群荣耀详情
	QQGroupSystemMsg GetGroupSystemMsg(const QQGroup& group);				// 获取群系统消息，比如加群消息和邀请消息
	vector<QQEssenceMsg> GetGroupEssenceMsg(const QQGroup& group);			// 获取群精华消息
	int GetGroupAtAllChance(const QQGroup& group);							// 获取Bot剩余@全体成员的次数，-1为不可以@全部成员
	int SetGroupName(const QQGroup& group, const string& name);				// 更改群名
	int SetGroupHeader(const QQGroup& group, const string& URI, int type);	// 【未实现】更改群头像
	int SetGroupAdmin(const QQGroup& group, const QQGroupMember& member, bool to_set);	// 设置QQ管理员
	int SetGroupMemberNickname(const QQGroup& group, const QQGroupMember& member, const string& nickname);	// 设置QQ成员的群名片
	int SetGroupMemberTitle(const QQGroup& group, const QQGroupMember& member, const string& title, int time = -1);	// 设置QQ成员的专属头衔
	int GroupMuteMember(const QQGroup& group, const QQGroupMember& member, unsigned int time);	// 禁言成员，单位秒，0为取消禁言
	int GroupMuteAll(const QQGroup& group, bool mute);
	// 禁言匿名群员，anonymous_json_object和anonymous_flag要从上报中获得，需传入其中至少一个
	int GroupMuteAnonymous(const QQGroup& group, const string& anonymous_json_object, const string& anonymous_flag, unsigned int time);
	int SetEssenceMsg(int message_id);										// 设置精华消息
	int RemoveEssenceMsg(int message_id);									// 取消精华消息
	int ClockInGroup(const QQGroup& group);									// 群打卡
	int SendGroupNotice(const QQGroup& group, const string& content, const string& image_url);		// 发送群公告
	vector<QQGroupNotice> GetGroupNotice(const QQGroup& group);				// 获取群公告
	int GroupKickMember(const QQGroup& group, const QQGroupMember& member, bool allow_join_again = true);

	static string GetQQHeaderImageURL(unsigned int QQid);					// 使用QQ号获取QQ头像链接
private:
	mutex QQ_lock_;															// 互斥锁
	unsigned int QQBot_id_ = 0;												// Bot的QQ号
	QQFriend administrator_ = NULLFriend;									// Bot的管理员
	string QQBot_nickname_;													// Bot的昵称
	string cqhttp_addr_;													// go-cqhttp的地址
	bool use_cache_ = true;													// 是否使用go-cqhttp的缓存，使用缓存会更快，但数据缺少即时性
	vector<QQFriend> QQBot_friend_list_;									// QQ好友列表
	vector<QQFriend>::iterator FindQQFriend(unsigned int friend_id);		// 查找QQ好友
	vector<QQUFriend> QQBot_Ufriend_list_;									// QQ单向好友列表
	vector<QQUFriend>::iterator FindQQUFriend(unsigned int ufriend_id);		// 查找QQ单向好友
	vector<QQGroup> QQBot_group_list_;										// QQ群列表
	vector<QQGroup>::iterator FindQQGroup(unsigned int group_id);			// 查找QQ群
	// 虽然返回迭代器是一种不规范的行为，但这些返回迭代器的函数都是私有的且在使用时都上了互斥锁
	string access_token_;													// go-cqhttp的access token
	int SendGETRequest(const string& URL, string& recv_buffer);				// 向go-cqhttp发送一个HTTP GET请求。用于调用go-cqhttp无需参数的API
	int SendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer);			// 向go-cqhttp发送一个HTTP POST请求。用于调用go-cqhttp需参数的API
	static QQFriend NULLFriend;												// 空好友
	static QQGroup NULLGroup;												// 空群
};

#endif // !QQ_H
