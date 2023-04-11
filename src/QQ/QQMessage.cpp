#include "QQMessage.h"
#include "QQMessage.h"
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
bool QQMessage::CanSendToGroup() const {
	return !must_to_private_;
}
bool QQMessage::CanSendToPrivate() const {
	return !must_to_group_;
}
void QQMessage::AddEmoji(const int emoji_id) {
	if (lock_single_CQcode) return;
	is_CQ_ = true;
	string CQ = "[CQ:face,id=";
	CQ.append(to_string(emoji_id));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AddShareLink(const string& URL, const string& title, const string& content = "", const string& image_URL = "") {
	if (lock_single_CQcode) return;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
	string CQ = "CQ:share,url=";
	CQ.append(URL);
	CQ.append(",title=");
	CQ.append(title);
	if (!content.empty()) {
		CQ.append(",content=");
		CQ.append(content);
	}
	if (!image_URL.empty()) {
		CQ.append(",image=");
		CQ.append(image_URL);
	}
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AddAt(const QQGroup& group, const QQGroupMember& member) {
	if (lock_single_CQcode) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:at,qq=";
	CQ.append(to_string(member.id_));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AddImageURLForPrivate(const string& URL, const int& type) {
	if (lock_single_CQcode) return;
	if (must_to_group_) return;
	if(type < 0 || type > 1) return;
	must_to_private_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:image,file=";
	CQ.append(URL);
	if (type == 1) {
		CQ.append(",type=flash");
	}
	CQ.append(",cache=1]");
	message_.append(CQ);
}
void QQMessage::AddImageFileForPrivate(const string& path, const int& type) {
	if (lock_single_CQcode) return;
	if (must_to_group_) return;
	if (type < 0 || type > 1) return;
	must_to_private_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:image,file=file://";
	CQ.append(path);
	if (type == 1) {
		CQ.append(",type=flash");
	}
	CQ.append(",cache=1]");
	message_.append(CQ);
}
void QQMessage::AddImageURLForGroup(const string& URL, const int subType) {
	if (lock_single_CQcode) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:image,file=";
	CQ.append(URL);
	CQ.append(",subType=");
	CQ.append(to_string(subType));
	CQ.append(",cache=1]");
	message_.append(CQ);
}
void QQMessage::AddImageFileForGroup(const string& path, const int subType) {
	if (lock_single_CQcode) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:image,file=file://";
	CQ.append(path);
	CQ.append(",subType=");
	CQ.append(to_string(subType));
	CQ.append(",cache=1]");
	message_.append(CQ);
}
void QQMessage::AddReply(const int message_id) {
	if (lock_single_CQcode) return;
	is_CQ_ = true;
	string CQ = "[CQ:reply,id=";
	CQ.append(to_string(message_id));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakePoke(const QQFriend& qfriend) {
	if (lock_single_CQcode) return;
	if (must_to_group_) return;
	must_to_private_ = true;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
	string CQ = "[CQ:poke,qq=" + to_string(qfriend.id_) + "]";
	message_.append(CQ);
}
void QQMessage::MakePoke(const QQGroup& group, const QQGroupMember& member) {
	if (lock_single_CQcode) return;
	if (must_to_private_) return;
	if (group.group_member_list_.find(member) == group.group_member_list_.end()) return;
	must_to_group_ = true;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
}
void QQMessage::MakeRecordURL(const string& URL) {
	if (lock_single_CQcode) return;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=";
	CQ.append(URL);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeRecordFile(const string& path) {
	if (lock_single_CQcode) return;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeVideoURL(const string& path) {
	if (lock_single_CQcode) return;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
	string CQ = "[CQ:video,file=";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeVideoFile(const string& path) {
	if (lock_single_CQcode) return;
	message_.clear();
	lock_single_CQcode = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
