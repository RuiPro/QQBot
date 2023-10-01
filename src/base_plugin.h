#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

class ThisBot;

// 插件抽象基类
class BasicPlugin {
public:
	BasicPlugin(ThisBot* bot, const string& app_path) {
		m_plugin_home_path = app_path;
	}
	// 获取插件名称
	virtual string PluginName() {
		return m_name;
	}
	// 获取版本
	virtual string Version() {
		return m_version;
	}
	// 获取作者
	virtual string Author() {
		return m_author;
	}
	// 获取详情
	virtual string OtherInfo() {
		return m_other_info;
	}
	// 加载插件时执行
	virtual void Loading() = 0;
	// 插件主体，每个Tick执行一次：可以传入从go-cqhttp获取的json信息
	virtual void PluginMain() = 0;
	virtual void PluginMain(const string& msg) = 0;
private:
	string m_name;           // 插件名称，一个标准的插件名称应该由大小写字母、数字和_组成，如QQWeather
	// 使用!@#$%^&*{}:">*这些字符会导致无法创建插件目录
	string m_version;        // 插件版本，应该遵循版本命名规范：主版本号.次版本号.修订版本号.日期版本号_希腊字母版本号
	string m_author;         // 插件作者
	string m_other_info;     // 插件信息，可以用来放你的Email或项目地址
	string m_plugin_home_path;// 插件相关文件的存放目录，为(%程序目录%/plugins/%插件名称)
};

#endif