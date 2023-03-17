#ifndef QQMESSAGE_H
#define QQMESSAGE_H

#include <string>
#include "QQ.h"
using namespace std;

class QQObject;
class QQFriend;
class QQGroupMember;
class QQGroup;
class QQBot;

// QQ消息类，就是有气泡框的消息。拍一拍不算在内-->QQBot::Poke
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
	bool IsGroupMsg() const;
	void AddAt(const QQGroup& group, const QQGroupMember& member);
	void AddEmoji(const int emoji_id);
	void AddImageAddr(const string& URL);		// 添加图片链接，需要连带传入协议(前面的http://)
	void AddImageFile(const string& path);		// 添加图片文件，直接传入路径
	void AddReply(const int message_id);
	void MakeRecordAddr(const string& URL);
	void MakeRecordFile(const string& path);
protected:
	bool is_CQ_ = false;
	bool is_record_ = false;
	string message_;
	int message_id_ = 0;
	bool is_group_msg = false;
};

#endif // !QQMESSAGE_H
