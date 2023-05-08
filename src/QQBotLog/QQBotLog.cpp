#include "QQBotLog.h"
#include "../BasicPlugin.h"

string GetSystemTime() {
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

QQBotLog* QQBotLog::log_ = new QQBotLog;

ostream& QQBotLog::Error() const {
	string output_str = GetSystemTime();
	output_str.append(error_tag_);
	cout << output_str;
	return cout;
}
ostream& QQBotLog::Warn() const {
	string output_str = GetSystemTime();
	output_str.append(warn_tag_);
	cout << output_str;
	return cout;
}
ostream& QQBotLog::Info() const {
	string output_str = GetSystemTime();
	output_str.append(info_tag_);
	cout << output_str;
	return cout;
}
ostream& QQBotLog::PluginError(BasicPlugin* plg) const {
	string output_str = GetSystemTime();
	output_str.append(error_tag_);
	output_str.append("[\033[36m");
	output_str.append(plg->PluginName());
	output_str.append("\033[0m] ");
	cout << output_str;
	return cout;
}
ostream& QQBotLog::PluginWarn(BasicPlugin* plg) const {
	string output_str = GetSystemTime();
	output_str.append(warn_tag_);
	output_str.append("[\033[36m");
	output_str.append(plg->PluginName());
	output_str.append("\033[0m] ");
	cout << output_str;
	return cout;
}
ostream& QQBotLog::PluginInfo(BasicPlugin* plg) const {
	string output_str = GetSystemTime();
	output_str.append(info_tag_);
	output_str.append("[\033[36m");
	output_str.append(plg->PluginName());
	output_str.append("\033[0m] ");
	cout << output_str;
	return cout;
}

const QQBotLog& QQBotLog::GetLogObject() {
	return *log_;
}

string QQBotLog::error_tag_ = " \033[31mError\033[0m]";
string QQBotLog::warn_tag_ = " \033[33mWarn\033[0m] ";
string QQBotLog::info_tag_ = " \033[34mInfo\033[0m] ";
