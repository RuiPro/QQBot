#include <iostream>
#include "BasicPlugin.h"
using namespace std;

class MyPlugin : public BasicPlugin {
public:
	// 构造函数
	// 你应该在此做的：对插件的成员变量进行初始化
	// 你不该在此做的：对插件配置进行初始化，因为此时程序主体还不知道插件的信息，
	MyPlugin(QQBot* bot, const string& app_path) : BasicPlugin(bot, app_path){
		this->author_ = "xxx";
		this->name_ = "MyPlugin";
		this->version_ = "0.1.1.alpha";
		this->other_info_ = "xxx@xxx.xxx";
	}
	// 加载插件时执行
	// 你应该在此做的：对插件配置进行初始化，比如读取插件配置文件
	void Loading() {
		plugin_home_path_ += "plugins/" + name_ + "/";
		cout << "欢迎使用xxx的插件：MyPlugin" << endl;
		cout << "联系方式：" << this->other_info_ << endl;
	}
	// 插件主体，每个Tick执行一次
	void PluginMain() {
		// 无参版：每个Tick都会调用一次
	}
	void PluginMain(const string& msg) {
		// 有参版：只有消息队列有消息时，才会调用
		// 下面的逻辑是：当有人给Bot发送"呀咩咯"时，Bot进行回复
		json QQevent = json::parse(msg, NULL, false);
		if (QQevent["post_type"] == "message") {
			if (QQevent["message_type"] == "private") {
				unsigned int sender = QQevent["sender"]["user_id"];
				QQFriend f(sender);
				QQMessage msg;
				msg << "呀咩咯!";
				msg.AddImageAddr(bot_->GetQQHeaderImageURL(sender));
				bot_->SendPrivateMsg(f, msg);
			}
			else if (QQevent["message_type"] == "group" && QQevent["message"] == "呀咩咯") {
				unsigned int sender_group = QQevent["group_id"];
				QQGroup g(sender_group);
				unsigned int sender_member = QQevent["user_id"];
				QQGroupMember m(sender_member);
				QQMessage msg;
				msg.AddAt(g, m);
				msg << "呀咩咯!";
				bot_->SendGroupMsg(g, msg);
			}
		}
	}
};

// 下面是必须存在的两个函数，你可以把MyPlugin改成你自己继承的类名
// 一定要带extern "C"，不然g++会把函数添加奇奇怪怪的东西以实现重载，到时候加载插件就找不到了
// 比如loadPlugin()会变成_Z10loadPluginv()
// 可以使用nm 库文件 | grep loadPlugin看看
extern "C" MyPlugin * loadPlugin(QQBot* bot, const string& app_path) {
    return new MyPlugin(bot, app_path);
}
extern "C" void destroyPlugin(MyPlugin * plugin) {
    delete plugin;
}
