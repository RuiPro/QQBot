#ifndef LOGER_H
#define LOGER_H

#include <iostream>
#include <string>
#include <ctime>
#include <mutex>
#include <sstream>
#include "../Gadget/all2str.h"
using namespace std;

class BasicPlugin;

string getSystemTime();

enum ostreamType{
	std_err,
	std_cout
};

class Pstring : public std::string {
public:
	template <typename T>
	Pstring& operator<< (const T& t) {
		this->append(all2str(t));
		return *this;
	}
};

class Loger {
public:
	Loger();
	~Loger();
	Loger(const Loger&) = delete;
	Loger(Loger&&) = delete;
	Loger& operator=(const Loger& obj) = delete;

	Pstring& debug();
	Pstring& error();
	Pstring& warn();
	Pstring& info();
	Pstring& pluginDebug(BasicPlugin* plg);
	Pstring& pluginError(BasicPlugin* plg);
	Pstring& pluginWarn(BasicPlugin* plg);
	Pstring& pluginInfo(BasicPlugin* plg);

protected:
	ostreamType m_stream_type;
	Pstring m_to_print;
	static mutex sm_lock;
	static string sm_error_tag;
	static string sm_warn_tag;
	static string sm_info_tag;
	static string sm_debug_tag;
};

#define loger Loger()

#endif // !OUTPUT_H