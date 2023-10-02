#include "loger.h"
#include "../base_plugin.h"

string Loger::sm_error_tag = " \033[31mError\033[0m]";
string Loger::sm_warn_tag = " \033[33mWarn\033[0m] ";
string Loger::sm_info_tag = " \033[34mInfo\033[0m] ";
string Loger::sm_debug_tag = " \033[43;37mDebug\033[0m]";

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

mutex Loger::sm_lock;

Loger::Loger() {}
Loger::~Loger() {
	unique_lock<mutex> locker(Loger::sm_lock);
	switch (m_stream_type) {
	case std_err:
		cerr << m_to_print << endl;
		break;
	case std_cout:
		cout << m_to_print << endl;
		break;
	}
}
Pstring& Loger::debug() {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_debug_tag);
	m_stream_type = ostreamType::std_err;
	return m_to_print;
}
Pstring& Loger::error() {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_error_tag);
	m_stream_type = ostreamType::std_err;
	return m_to_print;
}
Pstring& Loger::warn() {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_warn_tag);
	m_stream_type = ostreamType::std_cout;
	return m_to_print;
}
Pstring& Loger::info() {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_info_tag);
	m_stream_type = ostreamType::std_cout;
	return m_to_print;
}
Pstring& Loger::pluginDebug(BasicPlugin* plg) {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_debug_tag);
	m_to_print.append("[\033[36m");
	m_to_print.append(plg->PluginName());
	m_to_print.append("\033[0m] ");
	m_stream_type = ostreamType::std_err;
	return m_to_print;
}
Pstring& Loger::pluginError(BasicPlugin* plg) {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_error_tag);
	m_to_print.append("[\033[36m");
	m_to_print.append(plg->PluginName());
	m_to_print.append("\033[0m] ");
	m_stream_type = ostreamType::std_err;
	return m_to_print;
}
Pstring& Loger::pluginWarn(BasicPlugin* plg) {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_warn_tag);
	m_to_print.append("[\033[36m");
	m_to_print.append(plg->PluginName());
	m_to_print.append("\033[0m] ");
	m_stream_type = ostreamType::std_cout;
	return m_to_print;
}
Pstring& Loger::pluginInfo(BasicPlugin* plg) {
	m_to_print.append(getSystemTime());
	m_to_print.append(sm_info_tag);
	m_to_print.append("[\033[36m");
	m_to_print.append(plg->PluginName());
	m_to_print.append("\033[0m] ");
	m_stream_type = ostreamType::std_cout;
	return m_to_print;
}
