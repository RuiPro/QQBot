#ifndef LOGER_H
#define LOGER_H

#include <iostream>
#include <string>
#include <ctime>
using namespace std;

class BasicPlugin;

string getSystemTime();

class Loger {
public:
	Loger(const Loger&) = delete;
	Loger(Loger&&) = delete;
	Loger& operator=(const Loger& obj) = delete;

	ostream& error() const;
	ostream& warn() const;
	ostream& info() const;
	ostream& pluginError(BasicPlugin* plg) const;
	ostream& pluginWarn(BasicPlugin* plg) const;
	ostream& pluginInfo(BasicPlugin* plg) const;
	static Loger* getLoger();

protected:
	Loger() = default;
	static string m_error_tag;
	static string m_warn_tag;
	static string m_info_tag;
	static Loger* m_log;
};

#define loger (*Loger::getLoger())

#endif // !OUTPUT_H