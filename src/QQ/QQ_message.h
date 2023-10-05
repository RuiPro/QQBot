#ifndef QQMESSAGE_H
#define QQMESSAGE_H

#include <string>
using namespace std;

// QQ发送的消息类
// 线程不安全
class QQMessage {
public:
	QQMessage& msgAppend(const string& to_append);
	QQMessage& operator<< (const string& to_append);
	// 消息属性设置&获取
	void setMsgID(int msg_id);						// 设置消息ID
	void setForwardMsgID(const string& forward_id);	// 设置转发消息ID
	int getMsgID() const;							// 获取消息ID
	string getForwardMsgID() const;					// 获取转发消息ID
	string getMsg() const;							// 获取包装好的消息字符串
	void setCQMsg(bool s);							// 设置是否包含CQ码
	bool isCQMsg() const;							// 判断是否含CQ码
	bool isForwardMsg() const;						// 判断是否为转发消息
	bool canSendToGroup() const;					// 判断是否可以发送到群
	bool canSendToPrivate() const;					// 判断是否可以发到私聊
	void reset();									// 重置属性为默认

	void addAt(unsigned int member_id);				// 添加@xxx 发送到群聊。请保证成员id在群内，否则行为未定义
	void atAll();									// @所有人
	void addEmoji(int emoji_id);					// 添加emoji
	// 添加图片链接，使用URL需要连带传入协议(比如前面的http://)，添加图片文件则直接传入路径
	void addImageURLForPrivate(const string& URL, const int& type = 0);		// type: 0普通图片，1闪照
	void addImageFileForPrivate(const string& path, const int& type = 0);
	void addImageURLForGroup(const string& URL, const int subType = 0);		// subType: 0正常图片 1表情包 2斗图...详见https://docs.go-cqhttp.org/cqcode/#%E5%9B%BE%E7%89%87
	void addImageFileForGroup(const string& path, const int subType = 0);
	void addReply(const int message_id);
	void addForwardMsg(int message_id);				// 添加对现有消息的转发
	void addForwardMsg(const string& role_name, unsigned int role_id, const QQMessage& content);	// 添加自定义消息的转发

	void makeShareLink(const string& URL, const string& title, const string& content, const string& image_URL);
	void makePoke(unsigned int id, bool is_private_poke);	// 拍一拍好友或群成员，请保证成员id在群内，否则行为未定义	
	void makeRecordURL(const string& URL);			// 发送音频
	void makeRecordFile(const string& path);
	void makeVideoURL(const string& URL);			// 发送视频
	void makeVideoFile(const string& path);
	void makeForwardMsg();							// 转发消息
protected:
	bool m_isCQMsg = false;				// 是否为CQ码
	bool m_lock_single = false;			// 是否只能为固定结构消息，固定结构消息比如语音和拍一拍，这些消息无法添加其他内容
	bool m_is_forwardMsg = false;		// 是否为转发消息
	string m_message;
	int m_message_id = 0;
	string m_forward_id;				// 转发消息的ID，如果不是转发消息则为空

	// 下面两个标签标志信息必须发到私聊/群，通过canSendToGroup()、canSendToPrivate()访问
	// 比如@某人功能只能发送为群消息
	bool must_to_group_ = false;
	bool must_to_private_ = false;
};

#endif // !QQMESSAGE_H