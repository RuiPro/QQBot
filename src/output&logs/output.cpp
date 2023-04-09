#include "output.h"

string GetSystemTime() {
	std::time_t now = std::time(nullptr);
	std::tm* local_time = std::localtime(&now);
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
	now_time.append("]");
	return now_time;
}

Console_Error* Console_Error::cError_ = new Console_Error;
Console_Info* Console_Info::cInfo_ = new Console_Info;
Console_Warn* Console_Warn::cWarn_ = new Console_Warn;

Plugin_Console_Error* Plugin_Console_Error::pError_ = new Plugin_Console_Error;
Plugin_Console_Info* Plugin_Console_Info::pInfo_ = new Plugin_Console_Info;
Plugin_Console_Warn* Plugin_Console_Warn::pWarn_ = new Plugin_Console_Warn;

Console_Error& Error() {
	return *Console_Error::cError_;
}
Console_Info& Info() {
	return *Console_Info::cInfo_;
}
Console_Warn& Warn() {
	return *Console_Warn::cWarn_;
}

Plugin_Console_Error& PError() {
	return *Plugin_Console_Error::pError_;
}
Plugin_Console_Info& PInfo() {
	return *Plugin_Console_Info::pInfo_;
}
Plugin_Console_Warn& PWarn() {
	return *Plugin_Console_Warn::pWarn_;
}