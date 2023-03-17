#include "QQMessage.h"

void QQMessage::SetMsgID(int msg_id) {
	message_id_ = msg_id;
}
int QQMessage::GetMsgID() const {
	return message_id_;
}
QQMessage& QQMessage::MsgAppend(const string& to_append) {
	message_.append(to_append);
	return *this;
}
QQMessage& QQMessage::operator<< (const string& to_append) {
	message_.append(to_append);
	return *this;
}
string QQMessage::GetMsg() const {
	return message_;
}
void QQMessage::SetCQMsg(bool b) {
	is_CQ_ = b;
}
bool QQMessage::IsCQMsg() const {
	return is_CQ_;
}
bool QQMessage::IsGroupMsg() const {
	return is_group_msg;
}
void QQMessage::AddEmoji(const int emoji_id) {
	if (is_record_) return;
	is_CQ_ = true;
	string CQ = "[CQ:face,id=";
	CQ.append(to_string(emoji_id));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AddAt(const QQGroup& group, const QQGroupMember& member) {
	if (is_record_) return;
	is_group_msg = true;
	is_CQ_ = true;
	string CQ = "[CQ:at,qq=";
	CQ.append(to_string(member.id_));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AddImageAddr(const string& URL) {
	if (is_record_) return;
	is_CQ_ = true;
	string CQ = "[CQ:image,file=";
	CQ.append(URL);
	CQ.append(",type=show,subType=0,cache=1,id=40000]");
	message_.append(CQ);
}
void QQMessage::AddImageFile(const string& path) {
	if (is_record_) return;
	is_CQ_ = true;
	string CQ = "[CQ:image,file=file://";
	CQ.append(path);
	CQ.append(",type=show,subType=0,cache=1,id=40000]");
	message_.append(CQ);
}
void QQMessage::AddReply(const int message_id) {
	if (is_record_) return;
	is_CQ_ = true;
	string CQ = "[CQ:reply,id=";
	CQ.append(to_string(message_id));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeRecordAddr(const string& URL) {
	message_.clear();
	is_record_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=";
	CQ.append(URL);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeRecordFile(const string& path) {
	message_.clear();
	is_record_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
