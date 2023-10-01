#ifndef LOAD_PLUGIN_H
#define LOAD_PLUGIN_H

#include <dlfcn.h>
#include "base_plugin.h"

#define Good_Plugin true;
#define Bad_Plugin false;

class ThisBot;

// 加载插件类：用于把插件加载到内存中，插件的使用和析构
class LoadedPlugin {
public:
	// 构造：传入插件位置和一个QQbot
	LoadedPlugin(const string& plugin_path, ThisBot* bot, const string& app_path) {
		m_handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
		if (m_handle == nullptr) {
			loger.error() << "Open plugin failed: " << dlerror() << endl;
			m_plugin_status = Bad_Plugin;
			return;
		}
		*(void**)(&m_loadPlugin) = dlsym(m_handle, "loadPlugin");
		if (m_loadPlugin == nullptr) {
			loger.error() << "Get load plugin function failed: " << dlerror() << endl;
			dlclose(m_handle);
			m_plugin_status = Bad_Plugin;
			return;
		}
		*(void**)(&m_destroyPlugin) = dlsym(m_handle, "destroyPlugin");
		if (m_destroyPlugin == nullptr) {
			loger.error() << "Get destroy plugin function failed: " << dlerror() << endl;
			dlclose(m_handle);
			m_plugin_status = Bad_Plugin;
			return;
		}
		m_plugin = m_loadPlugin(bot, app_path);
	}
	~LoadedPlugin() {
		if (m_plugin != nullptr) m_destroyPlugin(m_plugin);
		if (m_handle != nullptr) dlclose(m_handle);
	}
	// 判断插件状态
	bool isGood() {
		return m_plugin_status;
	}
	// 获取名称
	string GetName() {
		return m_plugin->PluginName();
	}
	// 获取版本
	string GetVersion() {
		return m_plugin->Version();
	}
	// 获取作者
	string GetAuthor() {
		return m_plugin->Author();
	}
	// 获取其他信息
	string GetOtherInfo() {
		return m_plugin->OtherInfo();
	}
	// 加载插件时执行
	void Loading() {
		m_plugin->Loading();
	}
	// 插件主体，每个Tick执行一次：可以传入从go-cqhttp获取的json信息
	void PluginMain() {
		m_plugin->PluginMain();
	}
	void PluginMain(const string& msg) {
		m_plugin->PluginMain(msg);
	}
	BasicPlugin* GetBasicPlugin() {
		return m_plugin;
	}
private:
	bool m_plugin_status = Good_Plugin;
	BasicPlugin* (*m_loadPlugin)(ThisBot*, const string&);
	void (*m_destroyPlugin)(BasicPlugin*);
	BasicPlugin* m_plugin = nullptr;
	void* m_handle = nullptr;
};

#endif // !LOADPLUGIN_H
