#include "QQ_message.h"

void QQMessage::setMsgID(int msg_id) {
	m_message_id = msg_id;
}
void QQMessage::setForwardMsgID(const string& forward_id) {
	m_forward_id = forward_id;
}
int QQMessage::getMsgID() const {
	return m_message_id;
}
string QQMessage::getForwardMsgID() const {
	return m_forward_id;
}
QQMessage& QQMessage::msgAppend(const string& to_append) {
	for (auto& element : to_append) {
		switch (element) {
		case '&':
			m_message.append("&amp;");
			break;
		case '[':
			m_message.append("&#91;");
			break;
		case ']':
			m_message.append("&#93;");
			break;
		case ',':
			m_message.append("&#44;");
			break;
		default:
			m_message.push_back(element);
		}
	}
	return *this;
}
QQMessage& QQMessage::operator<< (const string& to_append) {
	return msgAppend(to_append);
}
string QQMessage::getMsg() const {
	return m_message;
}
void QQMessage::setCQMsg(bool b) {
	m_isCQMsg = b;
}
bool QQMessage::isCQMsg() const {
	return m_isCQMsg;
}
bool QQMessage::isForwardMsg() const {
	return m_is_forwardMsg;
}
bool QQMessage::canSendToGroup() const {
	return !must_to_private_;
}
bool QQMessage::canSendToPrivate() const {
	return !must_to_group_;
}
void QQMessage::reset() {
	m_isCQMsg = false;
	m_lock_single = false;
	m_is_forwardMsg = false;
	m_message.clear();
	m_message_id = 0;
	m_forward_id.clear();
	must_to_group_ = false;
	must_to_private_ = false;
}
void QQMessage::addEmoji(const int emoji_id) {
	if (m_lock_single) return;
	m_isCQMsg = true;
	string CQ = "[CQ:face,id=";
	CQ.append(to_string(emoji_id));
	CQ.append("]");
	m_message.append(CQ);
}
void QQMessage::addAt(unsigned int member_id) {
	if (m_lock_single) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	m_isCQMsg = true;
	string CQ = "[CQ:at,qq=";
	CQ.append(to_string(member_id));
	CQ.append("]");
	m_message.append(CQ);
}
void QQMessage::atAll() {
	if (m_lock_single) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	m_isCQMsg = true;
	string CQ = "[CQ:at,qq=all]";
	m_message.append(CQ);
}
void QQMessage::addImageURLForPrivate(const string& URL, const int& type) {
	if (m_lock_single) return;
	if (must_to_group_) return;
	if(type < 0 || type > 1) return;
	must_to_private_ = true;
	m_isCQMsg = true;
	string CQ = "[CQ:image,file=";
	CQ.append(URL);
	if (type == 1) {
		CQ.append(",type=flash");
	}
	CQ.append(",cache=1]");
	m_message.append(CQ);
}
void QQMessage::addImageFileForPrivate(const string& path, const int& type) {
	if (m_lock_single) return;
	if (must_to_group_) return;
	if (type < 0 || type > 1) return;
	must_to_private_ = true;
	m_isCQMsg = true;
	string CQ = "[CQ:image,file=file://";
	CQ.append(path);
	if (type == 1) {
		CQ.append(",type=flash");
	}
	CQ.append(",cache=1]");
	m_message.append(CQ);
}
void QQMessage::addImageURLForGroup(const string& URL, const int subType) {
	if (m_lock_single) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	m_isCQMsg = true;
	string CQ = "[CQ:image,file=";
	CQ.append(URL);
	CQ.append(",subType=");
	CQ.append(to_string(subType));
	CQ.append(",cache=1]");
	m_message.append(CQ);
}
void QQMessage::addImageFileForGroup(const string& path, const int subType) {
	if (m_lock_single) return;
	if (must_to_private_) return;
	must_to_group_ = true;
	m_isCQMsg = true;
	string CQ = "[CQ:image,file=file://";
	CQ.append(path);
	CQ.append(",subType=");
	CQ.append(to_string(subType));
	CQ.append(",cache=1]");
	m_message.append(CQ);
}
void QQMessage::addReply(const int message_id) {
	if (m_lock_single) return;
	m_isCQMsg = true;
	string CQ = "[CQ:reply,id=";
	CQ.append(to_string(message_id));
	CQ.append("]");
	m_message.append(CQ);
}

void QQMessage::makeShareLink(const string& URL, const string& title, const string& content = "", const string& image_URL = "") {
	if (m_lock_single) return;
	reset();
	m_lock_single = true;
	m_isCQMsg = true;
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
	m_message.append(CQ);
}
void QQMessage::makePoke(unsigned int id, bool is_private_poke) {
	if (is_private_poke) {
		if (m_lock_single) return;
		reset();
		must_to_private_ = true;
		m_lock_single = true;
		m_isCQMsg = true;
		string CQ = "[CQ:poke,qq=" + to_string(id) + "]";
		m_message.append(CQ);
	}
	else {
		if (m_lock_single) return;
		reset();
		must_to_group_ = true;
		m_lock_single = true;
		m_isCQMsg = true;
		string CQ = "[CQ:poke,qq=" + to_string(id) + "]";
		m_message.append(CQ);
	}
}
void QQMessage::makeRecordURL(const string& URL) {
	if (m_lock_single) return;
	reset();
	m_lock_single = true;
	m_isCQMsg = true;
	string CQ = "[CQ:record,file=";
	CQ.append(URL);
	CQ.append("]");
	m_message.append(CQ);
}
void QQMessage::makeRecordFile(const string& path) {
	if (m_lock_single) return;
	reset();
	m_lock_single = true;
	m_isCQMsg = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	m_message.append(CQ);
}
void QQMessage::makeVideoURL(const string& path) {
	if (m_lock_single) return;
	reset();
	m_lock_single = true;
	m_isCQMsg = true;
	string CQ = "[CQ:video,file=";
	CQ.append(path);
	CQ.append("]");
	m_message.append(CQ);
}
void QQMessage::makeVideoFile(const string& path) {
	if (m_lock_single) return;
	reset();
	m_lock_single = true;
	m_isCQMsg = true;
	string CQ = "[CQ:record,file=file://";
	CQ.append(path);
	CQ.append("]");
	m_message.append(CQ);
}
void QQMessage::makeForwardMsg() {
	if (m_lock_single) return;
	reset();
	m_lock_single = true;
	m_is_forwardMsg = true;
	m_message = "[]";
}
void QQMessage::addForwardMsg(int message_id) {
	if (!m_is_forwardMsg) return;
	json forward_msg_list = json::parse(m_message);
	json forward_msg_node;
	forward_msg_node["type"] = "node";
	json data;
	data["id"] = message_id;
	forward_msg_node["data"] = data;
	forward_msg_list.push_back(forward_msg_node);
	m_message = forward_msg_list.dump();
}
void QQMessage::addForwardMsg(const string& role_name, unsigned int role_id, const QQMessage& content) {
	if (!m_is_forwardMsg) return;
	if (content.m_is_forwardMsg) return;
	json forward_msg_list = json::parse(m_message);
	json forward_msg_node;
	forward_msg_node["type"] = "node";
	json data;
	data["name"] = role_name;
	data["uin"] = to_string(role_id);
	data["content"] = content.getMsg();
	forward_msg_node["data"] = data;
	forward_msg_list.push_back(forward_msg_node);
	m_message = forward_msg_list.dump();
}
