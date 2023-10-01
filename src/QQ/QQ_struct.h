#ifndef QQSTRUCTURE_H
#define QQSTRUCTURE_H

#include <string>
#include <vector>
using namespace std;

// QQ接受的消息类
struct QQRawMessage {
	bool m_is_group_msg = false;		// 是否为群消息
	unsigned int m_group_id = 0;		// 是群消息时为群号，否则为0
	int m_real_id = 0;				// 消息真实ID（有什么用？）
	int m_message_id = 0;			// 消息ID
	unsigned int m_sender_id = 0;	// 发送者的qq号
	string m_sender_nickname;		// 发送者昵称
	int m_send_time = 0;				// 发送时间戳
	string m_message;				// 消息内容
	string m_raw_message;			// 原始消息内容
	string m_forward_id;				// 转发消息的ID，如果不是转发消息则为空
};

// QQ接受的转发的子消息类
struct QQForwardMsgNode {
	unsigned int m_sender_id = 0;	// 发送者的qq号
	string m_sender_nickname;		// 发送者昵称
	int m_send_time = 0;				// 发送时间戳
	string m_message;				// 消息内容
};

// QQ在线设备类
struct QQOnlineClient {
	int m_app_id;			// 客户端ID
	string m_device_name;	// 设备名称
	string m_device_kind;	// 设备类型
};

// QQ图片OCR的文字元素
struct ImageOCRElement {
	string m_text;		// 文字
	int m_confidence;		// 置信度
	pair<int, int> m_coordinates;	// 坐标？但是go-cqhttp的文档里没写坐标的详细数据结构，因此先用pair<int,int>存着
};

// QQ图片OCR
struct QQImageOCR {
public:
	string m_language;		// 语言
	vector< ImageOCRElement> m_imageOCR;
};

// QQ用户在某个群的荣耀
struct QQUserBaseHonor {
	unsigned int m_user_id;	// 群号
	string m_nickname;		// 昵称
	string m_header_url;		// 头像URL
};
// 今日龙王
struct QQDailyDragonKing : public QQUserBaseHonor {
	unsigned int m_day_count;// 蝉联天数
};
// 其他群荣耀，比如群聊之火、快乐之源
struct QQOtherHonor : public QQUserBaseHonor {
	string m_description;	// 荣誉描述
};
// 某个QQ群的群荣耀，比如龙王
struct QQGroupHonor {
public:
	unsigned int m_group_id;	// 群号
	QQDailyDragonKing m_dragon_king;				// 今日龙王
	vector<QQOtherHonor> m_history_dragon_king;		// 历史龙王
	vector<QQOtherHonor> m_chat_flame;				// 群聊之火
	vector<QQOtherHonor> m_chat_large_flame;		// 群聊炽焰
	vector<QQOtherHonor> m_bamboo_shoot;			// 冒尖小春笋
	vector<QQOtherHonor> m_happy_sorce;				// 快乐之源
};

// QQ群邀请请求和加群请求
struct QQGroupInvitedRequest {
	unsigned int m_request_id;		// 请求id
	unsigned int m_invitor_id;		// 邀请者id
	string m_invitor_nickname;		// 邀请者昵称
	unsigned int m_group_id;		// 群号
	unsigned int m_group_name;		// 群名
	bool m_has_checked;				// 是否已被处理
	unsigned int m_operator_id;		// 处理者，未处理为0
};
struct QQGroupJoinRequest {
	unsigned int m_request_id;		// 请求id
	unsigned int m_applicant_id;	// 申请人
	string m_applicant_nickname;	// 申请人id
	string m_applicant_message;		// 验证消息
	unsigned int m_group_id;		// 群号
	unsigned int m_group_name;		// 群名
	bool m_has_checked;				// 是否已被处理
	unsigned int m_operator_id;		// 处理者，未处理为0
};
// QQ系统消息
struct QQGroupSystemMsg {
	vector<QQGroupInvitedRequest> m_invitedList;
	vector<QQGroupJoinRequest> m_joinList;
};

// QQ精华消息
struct QQEssenceMsg {
	unsigned int m_sender_id;		// 精华消息发送者id
	string m_sender_nickname;		// 精华消息发送者昵称
	unsigned int m_send_time;		// 精华消息发送时间
	unsigned int m_operator_id;		// 设精操作者id
	string m_operator_nickname;		// 设精操作者昵称
	unsigned m_operator_settime;	// 设精时间
	int m_message_id;				// 精华消息的消息id
};

// QQ公告
struct QQGroupNotice {
	unsigned int m_sender_id;		// 公告发布者
	unsigned int m_send_time;		// 公告发布时间
	string m_content;				// 公告文字内容
	string m_image_id;				// 公告图片id
};

#endif // !QQSTRUCTURE_H
