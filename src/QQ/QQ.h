#ifndef QQ_H
#define QQ_H

#define LIBQQ_VERSION "1.0.0"

#include <string>
#include <vector>
#include <utility>
#include <mutex>
#include "QQ_struct.h"
#include "QQ_message.h"
using namespace std;

class SQLiteClient;
class QQMessage;
class QQRawMessage;

// 抽象QQ基类
class QQObject {
public:
	inline QQObject(unsigned int user_id) : 
		m_id(user_id) {}

	unsigned int m_id = 0;
	string m_name;
	virtual bool isNull() {
		if (m_id == 0) return true;
		return false;
	}
};

// QQ用户
class QQUser : public QQObject {
public:
	inline QQUser(unsigned int user_id) : QQObject(user_id) {}
	
	char m_gender;					// 性别，0=unknow，1=男，2=女
	unsigned short m_age;			// 年龄
	string m_qid;					// QQuid	
	unsigned short m_level;			// 等级
	unsigned int m_login_days;		// 达人日期？
};

// QQ好友
class QQFriend : public QQObject {
public:
	inline QQFriend(unsigned int user_id) : 
		QQObject(user_id) {}

	string m_friend_remark;			// 好友备注名
};

// QQ单向好友QQ Unidirectional Friend。单向好友的产生原因：加Bot为好友设置了不用确认即可添加。
class QQUFriend : public QQObject {
public:
	inline QQUFriend(unsigned int user_id) : 
		QQObject(user_id) {}

	string m_source;	// 单向好友来源
};

enum QQGroupRole {
	member,
	administrator,
	owner
};

// QQ群成员
class QQGroupMember : public QQObject {
public:
	inline QQGroupMember(unsigned int user_id) : 
		QQObject(user_id) {}

	unsigned int m_group_id = 0;				// 在哪个群
	int m_user_age = 0;							// 用户年龄
	string m_user_area;							// 用户所在地
	int m_user_gender;							// 用户性别，0未知，1男，2女
	string m_group_nickname;					// 群昵称
	bool m_group_nickname_changeable = false;	// 昵称是否可修改
	unsigned int m_group_join_time = 0;			// 加入群的时间戳
	unsigned int m_group_last_active_time = 0;	// 最近活跃的时间戳
	string m_group_level;						// 群等级，此字段还是旧版等级(冒泡0~活跃6)，不是新版的lv 0~100
	QQGroupRole m_group_role;					// 在群里的角色
	unsigned int m_group_mute_time = 0;			// 被禁言的时间，0为未被禁言
	string m_group_title;						// 专属群头衔
	unsigned int m_group_title_expire_time = 0;	// 群头衔到期时间，0为永不到期
	bool m_group_is_unfriendly = false;			// 是否为不良记录成员
};

// QQ群
class QQGroup : public QQObject {
public:
	inline QQGroup(unsigned int group_id) : 
		QQObject(group_id) {}

	short m_member_count = 0;				// 群人数
	short m_max_member_count = 0;			// 群最大可达人数
	string m_group_remark;					// 群备注
	unsigned int m_group_create_time = 0;	// 群创建时间
	unsigned int m_group_level = 0;			// 群等级
};

void createBot(const string&, const string&, bool);

// 用于绑定一个go-cqhttp登录的QQ号，单例
class ThisBot {
	friend void createBot(const string&, const string&, bool);
public:
	~ThisBot();
	ThisBot(const ThisBot&) = delete;
	ThisBot(ThisBot&&) = delete;
	ThisBot& operator=(const ThisBot& obj) = delete;

	// 【get】这些函数用以获取Bot的信息
	unsigned int getThisBotID() const;									// 获取Bot的QQ号
	string getThisBotNickname() const;									// 获取Bot的昵称
	bool getThisBotHasAdmin() const;									// 判断是否拥有管理员
	QQFriend getThisBotAdmin() const;									// 获取管理员
	int getThisBotFriendNum() const;									// 获取好友数量
	int getThisBotUFriendNum() const;									// 获取单向好友数量
	int getThisBotGroupNum() const;										// 获取群数量
	int getGroupMemberNum(unsigned int group_id) const;					// 获取某个群的成员数量
	bool getThisBotHasFriend(unsigned int friend_id) const;				// 判断是否存在某个好友
	bool getThisBotHasUFriend(unsigned int ufriend_id) const;			// 判断是否存在某个单向好友
	bool getThisBotHasGroup(unsigned int group_id) const;				// 判断是否加了某个群
	bool getGroupHasMember(unsigned int group_id, unsigned int member_id) const;		// 判断某个群是否存在某个群员
	QQFriend getThisBotFriend(unsigned int friend_id) const;			// 获取某个好友的信息
	QQUFriend getThisBotUFriend(unsigned int friend_id) const;			// 获取某个好友的信息
	QQGroup getThisBotGroup(unsigned int group_id) const;				// 获取某个群的信息
	QQGroupMember getGroupMember(unsigned int group_id, unsigned int member_id) const;	// 获取某个群的某个成员的信息
	vector<unsigned int> getThisBotFriendIDList() const;				// 获取好友列表，返回一个装有QQ号的容器
	vector<unsigned int> getThisBotUFriendIDList() const;				// 获取单向好友列表，返回一个装有QQ号的容器
	vector<unsigned int> getThisBotGroupIDList() const;					// 获取群号列表，返回一个装有群号的容器
	vector<unsigned int> getGroupMemberIDList(unsigned int group_id) const;				// 获取某个群的成员列表，返回一个装有QQ号的容器
	void printFriendList() const;										// 输出好友列表
	void printUFriendList() const;										// 输出单向好友列表
	void printGroupList() const;										// 输出群列表

	// 【set】这些函数用以手动设置Bot的某些信息
	int setThisBotAdmin(unsigned int admin_id);							// 设置管理员【功能待完善】
	void setCqhttpAddr(const string& addr);
	void setUseCqhttpCache(bool flag);
	void setCqhttpAccessToken(const string& token);

	// 【fetch】这些函数用以从go-cqhttp获取(更新)Bot的信息并保存或更新到Bot的成员变量或数据库内
	int fetchThisBotBasicInfo();						// QQ号和QQ昵称
	int fetchThisBotFriendList();						// QQ好友列表
	int fetchThisBotUFriendList();						// QQ好友列表
	int fetchThisBotGroupList();						// QQ群聊列表，不会获取群成员列表
	// 获取群信息，Bot必须在群内，否则行为未定义
	// 某个群的成员列表
	int fetchThisBotGroupMemberList(unsigned int group_id);
	// 某个群的某个群员的详细信息，传入成员在m_group_member_list的下标
	int fetchThisBotGroupMemberInfo(unsigned int group_id, unsigned int member_id);

	// 【query】这些函数用以从go-cqhttp获取其他信息
	QQGroup queryGroupInfo(unsigned int group_id);		// 从go-cqhttp获取QQ群基础信息，不带群成员列表。Bot可以不在群内
	QQUser queryUserInfo(unsigned int user_id);			// 获取一个QQ用户的信息
	QQRawMessage queryMessageInfo(int message_id);		// 获取消息的详情信息
	string queryCqhttpVersion();						// 打印go-cqhttp的版本
	bool queryCanSendImage();							// 判断QQ号是否可以发送图片？暂不知这个API的存在作用
	bool queryCanSendRecord();							// 判断QQ号是否可以发送录音？暂不知这个API的存在作用
	// 提供一个设备名，查看可以设置哪些在线机型名称及是否需要会员。
	// 比如提供小米MixAlpha，返回的列表里可能为小米MixAlpha(黑色)/true、小米MixAlpha(白色)/false...true表示需要氪会员
	vector<pair<string, bool>> queryDeviceShowList(const string& device_name);
	vector<QQOnlineClient> queryOnlineClients();									// 获取Bot当前在线设备列表
	vector<QQForwardMsgNode> queryForwardMsgContent(const string& forward_id);		// 获取转发消息的内容
	vector<QQRawMessage> queryGroupHistoryMsg(unsigned int group_id, unsigned int msg_seq = 0);	// 从提供的起始消息序号获取前19条历史消息
	int queryImageInfo(const string& file, int& size, string& filename, string& url);				// 获取图片信息，第一个是传入，后三个是传出
	QQImageOCR queryImageOCR(const string& image_id);							// 【未实现】获取图片OCR信息
	string queryRecordInfo(const string& file, const string& out_format);		// 获取语音信息，传入收到的语音文件名和需传出的格式(mp3、amr、wma、m4a、spx、ogg、wav、flac)，传出语音文件的绝对路径
	QQGroupHonor queryGroupHonorInfo(unsigned int group_id, const string& type = "all");	// 获取群荣耀详情
	QQGroupSystemMsg queryGroupSystemMsg(unsigned int group_id);				// 获取群系统消息，比如加群消息和邀请消息
	vector<QQEssenceMsg> queryGroupEssenceMsg(unsigned int group_id);			// 获取群精华消息
	int queryGroupatAllChance(unsigned int group_id);							// 获取Bot剩余@全体成员的次数，-1为不可以@全部成员
	vector<QQGroupNotice> queryGroupNotice(unsigned int group_id);				// 获取群公告

	// 【apply】这些函数用以向go-cqhttp请求实现Bot的某些主动动作
	int applySendPrivateMsg(unsigned int friend_id, QQMessage& msg);			// 发送私聊信息
	int applySendGroupMsg(unsigned int group_id, QQMessage& msg);				// 发送群聊信息
	int applySendPrivateForwardMsg(unsigned int friend_id, QQMessage& msg);	// 发送私聊合并信息
	int applySendGroupeForwardMsg(unsigned int group_id, QQMessage& msg);		// 发送群聊合并信息
	int applyRemoveFriend(unsigned int friend_id);								// 删除好友
	int applyRemoveUFriend(unsigned int ufriend_id);							// 删除单向好友
	int applyRemoveGroup(unsigned int group_id, bool dissolve = false);			// 退出群聊与解散，只有当bot是群主时解散有效
	int applyWithdrawMsg(int message_id);										// 撤回一条消息
	// 设置Bot的信息，包括昵称、公司、邮箱、学校、个人说明
	int applySetThisBotProfile(const string& nickname, const string& company, const string& email, const string& college, const string& personal_note);
	// 处理加好友请求，flag要从上报中获取
	int applyAddFriendRequest(const string& flag, const bool approve, const string& remark);
	// 处理加群请求，flag和sub_type要从上报中获取
	int applyAddGroupRequest(const string& flag, const string& sub_type, const bool approve, const string& reason);
	// 设置在线机型名称，需要从上面的GetDeviceShowList中选择一个元素传入device_name_element，非会员就选带false的元素
	// 比如传入小米MixAlpha、小米MixAlpha(白色)
	int applySetDeviceShowName(const string& device_name, const string& device_name_element);
	int applyMarkMsgAsRead(int message_id);										// 标记消息为已读
	int applySetGroupName(unsigned int group_id, const string& name);				// 更改群名
	int applySetGroupHeader(unsigned int group_id, const string& URI, int type);	// 【未实现】更改群头像
	int applySetGroupAdmin(unsigned int group_id, unsigned int member_id, bool to_set);	// 设置QQ管理员
	int applySetGroupMemberNickname(unsigned int group_id, unsigned int member_id, const string& nickname);	// 设置QQ成员的群名片
	int applySetGroupMemberTitle(unsigned int group_id, unsigned int member_id, const string& title, int time = -1);	// 设置QQ成员的专属头衔
	int applyMuteGroupMember(unsigned int group_id, unsigned int member_id, unsigned int time);	// 禁言成员，单位秒，0为取消禁言
	int applyMuteGroupAll(unsigned int group_id, bool mute);
	// 禁言匿名群员，anonymous_json_object和anonymous_flag要从上报中获得，需传入其中至少一个
	int applyMuteGroupAnonymous(unsigned int group_id, const string& anonymous_json_object, const string& anonymous_flag, unsigned int time);
	int applySetGroupEssenceMsg(int message_id);																// 设置精华消息
	int applyRemoveGroupEssenceMsg(int message_id);																// 取消精华消息
	int applyClockInGroup(unsigned int group_id);																// 群打卡
	int applySendGroupNotice(unsigned int group_id, const string& content, const string& image_url);				// 发送群公告
	int applyKickGroupMember(unsigned int group_id, unsigned int member_id, bool allow_join_again = true);	// 踢出成员
	
	static ThisBot* getThisBotObj();
	static string getQQHeaderImageURL(unsigned int user_id);		// 使用QQ号获取QQ头像链接
private:
	ThisBot(const string& cqhttp_addr, const string& cqhttp_access_token, bool m_cqhttp_use_cache);
	static ThisBot* sm_bot;

	SQLiteClient* sqlite_c = nullptr;		// 内存数据库
	unsigned int m_id = 0;					// Bot的QQ号
	unsigned int m_administrator_id = 0;	// Bot的管理员
	string m_nickname;						// Bot的昵称
	string m_cqhttp_addr;					// go-cqhttp的地址
	string m_cqhttp_access_token;			// go-cqhttp的access token
	bool m_cqhttp_use_cache;				// 是否使用go-cqhttp的缓存，使用缓存会更快，但数据缺少即时性

	// 向go-cqhttp发送一个HTTP GET请求。用于调用go-cqhttp无需参数的API
	int sendGETRequest(const string& URL, string& recv_buffer);
	// 向go-cqhttp发送一个HTTP POST请求。用于调用go-cqhttp需参数的API
	int sendPOSTRequest(const string& URL, const string& send_buffer, string& recv_buffer);
};

#define QQBot (*ThisBot::getThisBotObj())
#define QQBotPtr ThisBot::getThisBotObj()

#endif // !QQ_H