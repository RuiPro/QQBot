#ifndef QQSTRUCTURE_H
#define QQSTRUCTURE_H

#include <string>
#include <vector>
using namespace std;

// QQ接受的消息类
struct QQRawMessage {
	bool is_group_msg = false;		// 是否为群消息
	unsigned int group_id = 0;		// 是群消息时为群号，否则为0
	int real_id_ = 0;				// 消息真实ID（有什么用？）
	int message_id_ = 0;			// 消息ID
	unsigned int sender_id_ = 0;	// 发送者的qq号
	string sender_nickname_;		// 发送者昵称
	int send_time_ = 0;				// 发送时间戳
	string message_;				// 消息内容
	string raw_message_;			// 原始消息内容
	string forward_id_;				// 转发消息的ID，如果不是转发消息则为空
};

// QQ接受的转发的子消息类
struct QQForwardMsgNode {
	unsigned int sender_id_ = 0;	// 发送者的qq号
	string sender_nickname_;		// 发送者昵称
	int send_time_ = 0;				// 发送时间戳
	string message_;				// 消息内容
};

// QQ在线设备类
struct QQClient {
	QQClient(int app_id, string device_name, string device_kind)
		:app_id_(app_id), device_name_(device_name), device_kind_(device_kind_) {}
	int app_id_;			// 客户端ID
	string device_name_;	// 设备名称
	string device_kind_;	// 设备类型
};

// QQ图片OCR的文字元素
struct ImageOCRElement {
	string text_;		// 文字
	int confidence_;		// 置信度
	pair<int, int> coordinates_;	// 坐标？但是go-cqhttp的文档里没写坐标的详细数据结构，因此先用pair<int,int>存着
};

// QQ图片OCR
struct QQImageOCR {
public:
	string language_;		// 语言
	vector< ImageOCRElement> ImageOCR_;
};

// QQ用户在某个群的荣耀
struct QQUserBaseHonor {
	unsigned int user_id_;	// 群号
	string nickname_;		// 昵称
	string header_url_;		// 头像URL
};
// 今日龙王
struct QQDailyDragonKing : public QQUserBaseHonor {
	unsigned int day_count_;// 蝉联天数
};
// 其他群荣耀，比如群聊之火、快乐之源
struct QQOtherHonor : public QQUserBaseHonor {
	string description_;	// 荣誉描述
};
// 某个QQ群的群荣耀，比如龙王
struct QQGroupHonor {
public:
	unsigned int group_id_;	// 群号
	QQDailyDragonKing dragon_king_;				// 今日龙王
	vector<QQOtherHonor> history_dragon_king_;	// 历史龙王
	vector<QQOtherHonor> chat_flame_;			// 群聊之火
	vector<QQOtherHonor> chat_large_flame_;		// 群聊炽焰
	vector<QQOtherHonor> bamboo_shoot_;			// 冒尖小春笋
	vector<QQOtherHonor> happy_sorce_;			// 快乐之源
};

// QQ群邀请请求和加群请求
struct QQGroupInvitedRequest {
	unsigned int request_id_;		// 请求id
	unsigned int invitor_id_;		// 邀请者id
	string invitor_nickname_;		// 邀请者昵称
	unsigned int group_id_;			// 群号
	unsigned int group_name_;		// 群名
	bool has_checked_;				// 是否已被处理
	unsigned int operator_id_;		// 处理者，未处理为0
};
struct QQGroupJoinRequest {
	unsigned int request_id_;		// 请求id
	unsigned int applicant_id_;		// 申请人
	string applicant_nickname_;		// 申请人id
	string applicant_message_;		// 验证消息
	unsigned int group_id_;			// 群号
	unsigned int group_name_;		// 群名
	bool has_checked_;				// 是否已被处理
	unsigned int operator_id_;		// 处理者，未处理为0
};
// QQ系统消息
struct QQGroupSystemMsg {
	vector<QQGroupInvitedRequest> InvitedList;
	vector<QQGroupJoinRequest> JoinList;
};

// QQ精华消息
struct QQEssenceMsg {
	unsigned int sender_id_;		// 精华消息发送者id
	string sender_nickname_;		// 精华消息发送者昵称
	unsigned int send_time_;		// 精华消息发送时间
	unsigned int operator_id_;		// 设精操作者id
	string operator_nickname_;		// 设精操作者昵称
	unsigned operator_settime_;		// 设精时间
	int message_id_;				// 精华消息的消息id
};

// QQ公告
struct QQGroupNotice {
	unsigned int sender_id_;		// 公告发布者
	unsigned int send_time_;		// 公告发布时间
	string content_;				// 公告文字内容
	string image_id_;				// 公告图片id
};

#endif // !QQSTRUCTURE_H
