#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

class QQBot;

// 插件抽象基类
class BasicPlugin {
public:
	BasicPlugin(QQBot* bot, const string& app_path) {
		bot_ = bot;
		plugin_home_path_ = app_path;
	}
	// 获取插件名称
	virtual string PluginName() {
		return name_;
	}
	// 获取版本
	virtual string Version() {
		return version_;
	}
	// 获取作者
	virtual string Author() {
		return author_;
	}
	// 获取详情
	virtual string OtherInfo() {
		return other_info_;
	}
	// 加载插件时执行
	virtual void Loading() = 0;
	// 插件主体，每个Tick执行一次：可以传入从go-cqhttp获取的json信息
	virtual void PluginMain() = 0;
	virtual void PluginMain(const string& msg) = 0;
private:
	string name_;           // 插件名称，一个标准的插件名称应该由大小写字母、数字和_组成，如QQWeather
	// 使用!@#$%^&*{}:">*这些字符会导致无法创建插件目录
	string version_;        // 插件版本，你应该遵循版本命名规范：主版本号.次版本号.修订版本号.日期版本号_希腊字母版本号
	// 比如正式版第一版的第2个小版本，第53次构建，构建日期是1970年1月1日的快照版命名为1.2.53.19700101.RC
	string author_;         // 插件作者
	string other_info_;     // 插件信息，你可以用来放你的Email或项目地址
	QQBot* bot_ = nullptr;  // 使用QQbot的API
	string plugin_home_path_;// 插件相关文件的存放目录，为(%程序目录%/plugins/%插件名称)
};

#endif