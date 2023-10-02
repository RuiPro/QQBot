#ifndef LOGER_H
#define LOGER_H

#include <iostream>
#include <string>
#include <ctime>
#include <mutex>
#include <json.hpp>
using namespace std;
using json = nlohmann::json;
using json_value_type = nlohmann::json_abi_v3_11_2::detail::value_t;

class BasicPlugin;

string getSystemTime();

enum ostreamType{
	std_err,
	std_cout
};

class Pstring : public std::string {
public:
	Pstring& operator<< (const char* str_arr) {
		this->append(str_arr);
		return *this;
	}
	Pstring& operator<< (const std::string& str) {
		this->append(str);
		return *this;
	}
	Pstring& operator<< (int value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (short value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (long value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (long long value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (unsigned int value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (unsigned short value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (unsigned long value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (unsigned long long value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (float value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (double value) {
		this->append(to_string(value));
		return *this;
	}
	Pstring& operator<< (const char c) {
		this->operator+=(c);
		return *this;
	}
	Pstring& operator<< (const nlohmann::json& json_obj) {
		this->append(json_obj.dump());
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

	Pstring& error();
	Pstring& warn();
	Pstring& info();
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
};

#define loger Loger()

#endif // !OUTPUT_H