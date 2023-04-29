#ifndef QQBOTLOG_H
#define QQBOTLOG_H

#include <iostream>
using std::cout;
#include <string>
using std::string;
#include <ctime>

class QQBot;
#include "../BasicPlugin.h"

string GetSystemTime();

class QQBotLog {
public:
	QQBotLog(const QQBotLog&) = delete;
	QQBotLog(QQBotLog&&) = delete;
	QQBotLog& operator=(const QQBotLog& obj) = delete;

	ostream& Error() const;
	ostream& Warn() const;
	ostream& Info() const;
	ostream& PluginError(BasicPlugin* plg) const;
	ostream& PluginWarn(BasicPlugin* plg) const;
	ostream& PluginInfo(BasicPlugin* plg) const;
	static const QQBotLog& GetLogObject();

protected:
	QQBotLog() = default;
	static string error_tag_;
	static string warn_tag_;
	static string info_tag_;
	static QQBotLog* log_;
};

#define Qlog QQBotLog::GetLogObject()

#endif // !OUTPUT_H