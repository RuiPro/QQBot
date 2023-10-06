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
// �жϲ��״̬
bool LoadedPlugin::isGood() {
	return m_plugin_status;
}
// ��ȡ����
string LoadedPlugin::getPluginName() {
	return m_plugin->getPluginName();
}
// ��ȡ�汾
string LoadedPlugin::getPluginVersion() {
	return m_plugin->getVersion();
}
// ��ȡ����
string LoadedPlugin::getPluginAuthor() {
	return m_plugin->getAuthor();
}
// ��ȡ������Ϣ
string LoadedPlugin::getPluginOtherInfo() {
	return m_plugin->getOtherInfo();
}
// ���ز��ʱִ��
void LoadedPlugin::loading() {
	m_plugin->loading();
}
// ������壬ÿ��Tickִ��һ�Σ����Դ����go-cqhttp��ȡ��json��Ϣ
void LoadedPlugin::pluginMain() {
	m_plugin->pluginMain();
}
void LoadedPlugin::pluginMain(const string& msg) {
	m_plugin->pluginMain(msg);
}
BasicPlugin* LoadedPlugin::getBasicPlugin() {
	return m_plugin;
}