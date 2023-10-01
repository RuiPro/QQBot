#include "loger.h"
#include "../base_plugin.h"

string getSystemTime() {
	time_t now = time(nullptr);
	tm* local_time = localtime(&now);
	string now_time = "[";
	now_time.append(to_string(local_time->tm_year + 1900));
	now_time.append("-");
	if ((local_time->tm_mon + 1) < 10) {
		now_time.append("0");
	}
	now_time.append(to_string(local_time->tm_mon + 1));
	now_time.append("-");
	if (local_time->tm_mday < 10) {
		now_time.append("0");
	}
	now_time.append(to_string(local_time->tm_mday));
	now_time.append(" ");
	if (local_time->tm_hour < 10) {
		now_time.append("0");
	}
	now_time.append(to_string(local_time->tm_hour));
	now_time.append(":");
	if (local_time->tm_min < 10) {
		now_time.append("0");
	}
	now_time.append(to_string(local_time->tm_min));
	now_time.append(":");
	if (local_time->tm_sec < 10) {
		now_time.append("0");
	}
	now_time.append(to_string(local_time->tm_sec));
	return now_time;
}

Loger* Loger::m_log = new Loger;

ostream& Loger::error() const {
	string output_str = getSystemTime();
	output_str.append(m_error_tag);
	cout << output_str;
	return cout;
}
ostream& Loger::warn() const {
	string output_str = getSystemTime();
	output_str.append(m_warn_tag);
	cout << output_str;
	return cout;
}
ostream& Loger::info() const {
	string output_str = getSystemTime();
	output_str.append(m_info_tag);
	cout << output_str;
	return cout;
}
ostream& Loger::pluginError(BasicPlugin* plg) const {
	string output_str = getSystemTime();
	output_str.append(m_error_tag);
	output_str.append("[\033[36m");
	output_str.append(plg->PluginName());
	output_str.append("\033[0m] ");
	cout << output_str;
	return cout;
}
ostream& Loger::pluginWarn(BasicPlugin* plg) const {
	string output_str = getSystemTime();
	output_str.append(m_warn_tag);
	output_str.append("[\033[36m");
	output_str.append(plg->PluginName());
	output_str.append("\033[0m] ");
	cout << output_str;
	return cout;
}
ostream& Loger::pluginInfo(BasicPlugin* plg) const {
	string output_str = getSystemTime();
	output_str.append(m_info_tag);
	output_str.append("[\033[36m");
	output_str.append(plg->PluginName());
	output_str.append("\033[0m] ");
	cout << output_str;
	return cout;
}

Loger* Loger::getLoger() {
	return m_log;
}

string Loger::m_error_tag = " \033[31mError\033[0m]";
string Loger::m_warn_tag = " \033[33mWarn\033[0m] ";
string Loger::m_info_tag = " \033[34mInfo\033[0m] ";
