#ifndef QQMESSAGE_H
#define QQMESSAGE_H

#include <string>
#include "QQ.h"

class QQObject;
class QQFriend;
class QQGroupMember;
class QQGroup;
class QQBot;

// QQ发送的消息类
class QQMessage {
	friend QQBot;
public:
	QQMessage& MsgAppend(const string& to_append);
	QQMessage& operator<< (const string& to_append);
	void SetMsgID(int msg_id);
	int GetMsgID() const;
	string GetMsg() const;
	void SetCQMsg(bool s);
	bool IsCQMsg() const;
	bool CanSendToGroup() const;
	bool CanSendToPrivate() const;
	void AddAt(const QQGroup& group, const QQGroupMember& member);
	void AddEmoji(const int emoji_id);
	void AddShareLink(const string& URL, const string& title, const string& content, const string& image_URL);
	// 添加图片链接，使用URL需要连带传入协议(比如前面的http://)，添加图片文件则直接传入路径
	void AddImageURLForPrivate(const string& URL, const int& type = 0);		// type: 0普通图片，1闪照
	void AddImageFileForPrivate(const string& path, const int& type = 0);
	void AddImageURLForGroup(const string& URL, const int subType = 0);		// subType: 0正常图片 1表情包 2斗图...详见https://docs.go-cqhttp.org/cqcode/#%E5%9B%BE%E7%89%87
	void AddImageFileForGroup(const string& path, const int subType = 0);
	void AddReply(const int message_id);
	void MakePoke(const QQFriend& qfriend);
	void MakePoke(const QQGroup& group, const QQGroupMember& member);
	void MakeRecordURL(const string& URL);
	void MakeRecordFile(const string& path);
	void MakeVideoURL(const string& URL);
	void MakeVideoFile(const string& path);
protected:
	bool is_CQ_ = false;
	bool lock_single_CQcode = false;
	string message_;
	int message_id_ = 0;

	// 下面两个标签标志信息必须发到私聊/群，通过CanSendToGroup()、CanSendToPrivate()访问
	// 比如@某人功能只能发送为群消息
	bool must_to_group_ = false;
	bool must_to_private_ = false;
};

// QQ接受的消息类
class QQRawMessage {
public:
	bool is_group_msg = false;		// 是否为群消息
	unsigned int group_id = 0;		// 是群消息时为群号，否则为0
	int real_id_ = 0;				// 消息真实ID（有什么用？）
	int message_id_ = 0;			// 消息ID
	unsigned int sender_id_ = 0;	// 发送者的qq号
	string sender_nickname_;		// 发送者昵称
	int send_time_ = 0;				// 发送时间戳
	string message_;				// 消息内容
	string raw_message_;			// 原始消息内容
};

#endif // !QQMESSAGE_H
