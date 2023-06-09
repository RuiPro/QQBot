#ifndef QQMESSAGE_H
#define QQMESSAGE_H

#include "QQ.h"
using namespace std;

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
	void SetForwardMsgID(const string& forward_id);
	int GetMsgID() const;
	string GetForwardMsgID() const;
	string GetMsg() const;
	void SetCQMsg(bool s);
	bool IsCQMsg() const;
	bool IsForwardMsg() const;
	bool CanSendToGroup() const;
	bool CanSendToPrivate() const;
	void Reset();

	void AddAt(const QQGroup& group, const QQGroupMember& member);
	void AtAll(const QQGroup& group);
	void AddEmoji(const int emoji_id);
	// 添加图片链接，使用URL需要连带传入协议(比如前面的http://)，添加图片文件则直接传入路径
	void AddImageURLForPrivate(const string& URL, const int& type = 0);		// type: 0普通图片，1闪照
	void AddImageFileForPrivate(const string& path, const int& type = 0);
	void AddImageURLForGroup(const string& URL, const int subType = 0);		// subType: 0正常图片 1表情包 2斗图...详见https://docs.go-cqhttp.org/cqcode/#%E5%9B%BE%E7%89%87
	void AddImageFileForGroup(const string& path, const int subType = 0);
	void AddReply(const int message_id);

	void MakeShareLink(const string& URL, const string& title, const string& content, const string& image_URL);
	void MakePoke(const QQFriend& qfriend);
	void MakePoke(const QQGroup& group, const QQGroupMember& member);
	void MakeRecordURL(const string& URL);
	void MakeRecordFile(const string& path);
	void MakeVideoURL(const string& URL);
	void MakeVideoFile(const string& path);
	void MakeForwardMsg();
	void AddForwardMsg(int message_id);				// 添加对现有消息的转发
	void AddForwardMsg(const string& role_name, unsigned int role_id, const QQMessage& content);	// 添加自定义消息的转发
protected:
	bool is_CQ_ = false;				// 是否为CQ码
	bool lock_single_struct = false;	// 是否只能为固定结构消息，固定结构消息比如语音和拍一拍，这些消息无法添加其他内容
	bool is_forward_msg = false;		// 是否为转发消息
	string message_;
	int message_id_ = 0;
	string forward_id_;					// 转发消息的ID，如果不是转发消息则为空

	// 下面两个标签标志信息必须发到私聊/群，通过CanSendToGroup()、CanSendToPrivate()访问
	// 比如@某人功能只能发送为群消息
	bool must_to_group_ = false;
	bool must_to_private_ = false;
};

#endif // !QQMESSAGE_H
