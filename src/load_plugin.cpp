#include "load_plugin.h"
#include "Loger/loger.h"

LoadedPlugin::LoadedPlugin(const string& plugin_path, const string& app_path) {
	m_handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
	if (m_handle == nullptr) {
		loger.error() << "Open plugin failed: " << dlerror();
		m_plugin_status = Bad_Plugin;
		return;
	}
	*(void**)(&m_loadPlugin) = dlsym(m_handle, "loadPlugin");
	if (m_loadPlugin == nullptr) {
		loger.error() << "Get load plugin function failed: " << dlerror();
		dlclose(m_handle);
		m_plugin_status = Bad_Plugin;
		return;
	}
	*(void**)(&m_destroyPlugin) = dlsym(m_handle, "destroyPlugin");
	if (m_destroyPlugin == nullptr) {
		loger.error() << "Get destroy plugin function failed: " << dlerror();
		dlclose(m_handle);
		m_plugin_status = Bad_Plugin;
		return;
	}
	m_plugin = m_loadPlugin(app_path);
}
LoadedPlugin::~LoadedPlugin() {
	if (m_plugin != nullptr) m_destroyPlugin(m_plugin);
	if (m_handle != nullptr) dlclose(m_handle);
}
// 判断插件状态
bool LoadedPlugin::isGood() {
	return m_plugin_status;
}
// 获取名称
string LoadedPlugin::getPluginName() {
	return m_plugin->getPluginName();
}
// 获取版本
string LoadedPlugin::getPluginVersion() {
	return m_plugin->getVersion();
}
// 获取作者
string LoadedPlugin::getPluginAuthor() {
	return m_plugin->getAuthor();
}
// 获取其他信息
string LoadedPlugin::getPluginOtherInfo() {
	return m_plugin->getOtherInfo();
}
// 加载插件时执行
void LoadedPlugin::loading() {
	m_plugin->loading();
}
// 插件主体，每个Tick执行一次：可以传入从go-cqhttp获取的json信息
void LoadedPlugin::pluginMain() {
	m_plugin->pluginMain();
}
void LoadedPlugin::pluginMain(const string& msg) {
	m_plugin->pluginMain(msg);
}
BasicPlugin* LoadedPlugin::getBasicPlugin() {
	return m_plugin;
}