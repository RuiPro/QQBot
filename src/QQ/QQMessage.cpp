#include "QQMessage.h"

void QQMessage::SetMsgID(int msg_id) {
	message_id_ = msg_id;
}
void QQMessage::SetForwardMsgID(const string& forward_id) {
	forward_id_ = forward_id;
}
int QQMessage::GetMsgID() const {
	return message_id_;
}
string QQMessage::GetForwardMsgID() const {
	return forward_id_;
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
bool QQMessage::IsForwardMsg() const {
	return is_forward_msg;
}
bool QQMessage::CanSendToGroup() const {
	return !must_to_private_;
}
bool QQMessage::CanSendToPrivate() const {
	return !must_to_group_;
}
void QQMessage::Reset() {
	is_CQ_ = false;
	lock_single_struct = false;
	is_forward_msg = false;
	message_.clear();
	message_id_ = 0;
	forward_id_.clear();
	must_to_group_ = false;
	must_to_private_ = false;
}
void QQMessage::AddEmoji(const int emoji_id) {
	if (lock_single_struct) return;
	is_CQ_ = true;
	string CQ = "[CQ:face,id=";
	CQ.append(to_string(emoji_id));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AddAt(const QQGroup& group, const QQGroupMember& member) {
	if (lock_single_struct) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:at,qq=";
	CQ.append(to_string(member.id_));
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::AtAll(const QQGroup& group) {
	if (lock_single_struct) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	is_CQ_ = true;
	string CQ = "[CQ:at,qq=all]";
	message_.append(CQ);
}
void QQMessage::AddImageURLForPrivate(const string& URL, const int& type) {
	if (lock_single_struct) return;
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
	if (lock_single_struct) return;
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
	if (lock_single_struct) return;
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
	if (lock_single_struct) return;
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
	if (lock_single_struct) return;
	is_CQ_ = true;
	string CQ = "[CQ:reply,id=";
	CQ.append(to_string(message_id));
	CQ.append("]");
	message_.append(CQ);
}

void QQMessage::MakeShareLink(const string& URL, const string& title, const string& content = "", const string& image_URL = "") {
	if (lock_single_struct) return;
	Reset();
	lock_single_struct = true;
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
void QQMessage::MakePoke(const QQFriend& qfriend) {
	if (lock_single_struct) return;
	Reset();
	must_to_private_ = true;
	lock_single_struct = true;
	is_CQ_ = true;
	string CQ = "[CQ:poke,qq=" + to_string(qfriend.id_) + "]";
	message_.append(CQ);
}
void QQMessage::MakePoke(const QQGroup& group, const QQGroupMember& member) {
	if (lock_single_struct) return;
	if (!group.GetGroupHasMember(member.id_)) return;
	Reset();
	must_to_group_ = true;
	lock_single_struct = true;
	is_CQ_ = true;
	string CQ = "[CQ:poke,qq=" + to_string(member.id_) + "]";
	message_.append(CQ);
}
void QQMessage::MakeRecordURL(const string& URL) {
	if (lock_single_struct) return;
	Reset();
	lock_single_struct = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=";
	CQ.append(URL);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeRecordFile(const string& path) {
	if (lock_single_struct) return;
	Reset();
	lock_single_struct = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeVideoURL(const string& path) {
	if (lock_single_struct) return;
	Reset();
	lock_single_struct = true;
	is_CQ_ = true;
	string CQ = "[CQ:video,file=";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeVideoFile(const string& path) {
	if (lock_single_struct) return;
	Reset();
	lock_single_struct = true;
	is_CQ_ = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	message_.append(CQ);
}
void QQMessage::MakeForwardMsg() {
	if (lock_single_struct) return;
	Reset();
	lock_single_struct = true;
	is_forward_msg = true;
	message_ = "[]";
}
void QQMessage::AddForwardMsg(int message_id) {
	if (!is_forward_msg) return;
	json forward_msg_list = json::parse(message_);
	json forward_msg_node;
	forward_msg_node["type"] = "node";
	json data;
	data["id"] = message_id;
	forward_msg_node["data"] = data;
	forward_msg_list.push_back(forward_msg_node);
	message_ = forward_msg_list.dump();
}
void QQMessage::AddForwardMsg(const string& role_name, unsigned int role_id, const QQMessage& content) {
	if (!is_forward_msg) return;
	if (content.is_forward_msg) return;
	json forward_msg_list = json::parse(message_);
	json forward_msg_node;
	forward_msg_node["type"] = "node";
	json data;
	data["name"] = role_name;
	data["uin"] = to_string(role_id);
	data["content"] = content.GetMsg();
	forward_msg_node["data"] = data;
	forward_msg_list.push_back(forward_msg_node);
	message_ = forward_msg_list.dump();
}
