#ifndef LOAD_PLUGIN_H
#define LOAD_PLUGIN_H

#include <dlfcn.h>
#include <string>
#include "base_plugin.h"
using namespace std;

#define Good_Plugin true;
#define Bad_Plugin false;

class ThisBot;

// 加载插件类：用于把插件加载到内存中，插件的使用和析构
class LoadedPlugin {
public:
	// 构造：传入插件位置和一个QQbot
	LoadedPlugin(const string& plugin_path, ThisBot* bot, const string& app_path);
	~LoadedPlugin();
	// 判断插件状态
	bool isGood();
	// 获取名称
	string getPluginName();
	// 获取版本
	string getPluginVersion();
	// 获取作者
	string getPluginAuthor();
	// 获取其他信息
	string getPluginOtherInfo();
	// 加载插件时执行
	void loading();
	// 插件主体，每个Tick执行一次：可以传入从go-cqhttp获取的json信息
	void pluginMain();
	void pluginMain(const string& msg);
	BasicPlugin* getBasicPlugin();
private:
	bool m_plugin_status = Good_Plugin;
	BasicPlugin* (*m_loadPlugin)(ThisBot*, const string&);
	void (*m_destroyPlugin)(BasicPlugin*);
	BasicPlugin* m_plugin = nullptr;
	void* m_handle = nullptr;
};

#endif // !LOADPLUGIN_H
