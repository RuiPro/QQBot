#ifndef LOADPLUGIN_H
#define LOADPLUGIN_H

#include <dlfcn.h>
#include "BasicPlugin.h"

#define Good_Plugin true;
#define Bad_Plugin false;

class QQBot;

// 加载插件类：用于把插件加载到内存中，插件的使用和析构
class LoadedPlugin {
public:
	// 构造：传入插件位置和一个QQbot
	LoadedPlugin(const string& plugin_path, QQBot* bot, const string& app_path) {
		handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
		if (handle == nullptr) {
			Qlog.Error() << "Open plugin failed: " << dlerror() << std::endl;
			plugin_status_ = Bad_Plugin;
			return;
		}
		*(void**)(&loadPlugin) = dlsym(handle, "loadPlugin");
		if (loadPlugin == nullptr) {
			Qlog.Error() << "Get load plugin function failed: " << dlerror() << std::endl;
			dlclose(handle);
			plugin_status_ = Bad_Plugin;
			return;
		}
		*(void**)(&destroyPlugin) = dlsym(handle, "destroyPlugin");
		if (destroyPlugin == nullptr) {
			Qlog.Error() << "Get destroy plugin function failed: " << dlerror() << std::endl;
			dlclose(handle);
			plugin_status_ = Bad_Plugin;
			return;
		}
		plugin = loadPlugin(bot, app_path);
	}
	~LoadedPlugin() {
		if (plugin != nullptr) destroyPlugin(plugin);
		if (handle != nullptr) dlclose(handle);
	}
	// 判断插件状态
	bool isGood() {
		return plugin_status_;
	}
	// 获取名称
	string GetName() {
		return plugin->PluginName();
	}
	// 获取版本
	string GetVersion() {
		return plugin->Version();
	}
	// 获取作者
	string GetAuthor() {
		return plugin->Author();
	}
	// 获取其他信息
	string GetOtherInfo() {
		return plugin->OtherInfo();
	}
	// 加载插件时执行
	void Loading() {
		plugin->Loading();
	}
	// 插件主体，每个Tick执行一次：可以传入从go-cqhttp获取的json信息
	void PluginMain() {
		plugin->PluginMain();
	}
	void PluginMain(const string& msg) {
		plugin->PluginMain(msg);
	}
	BasicPlugin* GetBasicPlugin() {
		return plugin;
	}
private:
	bool plugin_status_ = Good_Plugin;
	BasicPlugin* (*loadPlugin)(QQBot*, const string&);
	void (*destroyPlugin)(BasicPlugin*);
	BasicPlugin* plugin = nullptr;
	void* handle = nullptr;
};

#endif // !LOADPLUGIN_H
