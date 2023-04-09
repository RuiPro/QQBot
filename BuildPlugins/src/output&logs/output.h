#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>
#include <mutex>
#include <string>
#include <ctime>
using namespace std;

string GetSystemTime();

class Console_Error {
	friend Console_Error& Error();
public:
	Console_Error(const Console_Error&) = delete;
	Console_Error(Console_Error&&) = delete;
	Console_Error& operator=(const Console_Error& obj) = delete;
	template<typename T>
	ostream& operator<<(T t) {
		string output_str = GetSystemTime();
		output_str.append(" \033[31m[Error]\033[0m\t");
		cout << output_str;
		cout << t;
		return cout;
	}
protected:
	Console_Error() = default;
	static Console_Error* cError_;
};

class Console_Info {
	friend Console_Info& Info();
public:
	Console_Info(const Console_Info&) = delete;
	Console_Info(Console_Info&&) = delete;
	Console_Info& operator=(const Console_Info& obj) = delete;
	template<typename T>
	ostream& operator<<(T t) {
		string output_str = GetSystemTime();
		output_str.append(" \033[34m[Info]\033[0m\t");
		cout << output_str;
		cout << t;
		return cout;
	}
protected:
	Console_Info() = default;
	static Console_Info* cInfo_;
};

class Console_Warn {
	friend Console_Warn& Warn();
public:
	Console_Warn(const Console_Warn&) = delete;
	Console_Warn(Console_Warn&&) = delete;
	Console_Warn& operator=(const Console_Warn& obj) = delete;
	template<typename T>
	ostream& operator<<(T t) {
		string output_str = GetSystemTime();
		output_str.append(" \033[33m[Warn]\033[0m\t");
		cout << output_str;
		cout << t;
		return cout;
	}
protected:
	Console_Warn() = default;
	static Console_Warn* cWarn_;
};

class Plugin_Console_Error {
	friend Plugin_Console_Error& PError();
public:
	Plugin_Console_Error(const Plugin_Console_Error&) = delete;
	Plugin_Console_Error(Plugin_Console_Error&&) = delete;
	Plugin_Console_Error& operator=(const Plugin_Console_Error& obj) = delete;
	template<typename T>
	ostream& operator<<(T t) {
		string output_str = GetSystemTime();
		output_str.append(" \033[31m[Error]\033[0m\033[32m[P]\033[0m\t");
		cout << output_str;
		cout << t;
		return cout;
	}
protected:
	Plugin_Console_Error() = default;
	static Plugin_Console_Error* pError_;
};

class Plugin_Console_Info {
	friend Plugin_Console_Info& PInfo();
public:
	Plugin_Console_Info(const Plugin_Console_Info&) = delete;
	Plugin_Console_Info(Plugin_Console_Info&&) = delete;
	Plugin_Console_Info& operator=(const Plugin_Console_Info& obj) = delete;
	template<typename T>
	ostream& operator<<(T t) {
		string output_str = GetSystemTime();
		output_str.append(" \033[34m[Info]\033[0m \033[32m[P]\033[0m\t");
		cout << output_str;
		cout << t;
		return cout;
	}
protected:
	Plugin_Console_Info() = default;
	static Plugin_Console_Info* pInfo_;
};

class Plugin_Console_Warn {
	friend Plugin_Console_Warn& PWarn();
public:
	Plugin_Console_Warn(const Plugin_Console_Warn&) = delete;
	Plugin_Console_Warn(Plugin_Console_Warn&&) = delete;
	Plugin_Console_Warn& operator=(const Plugin_Console_Warn& obj) = delete;
	template<typename T>
	ostream& operator<<(T t) {
		string output_str = GetSystemTime();
		output_str.append(" \033[33m[Warn]\033[0m \033[32m[P]\033[0m\t");
		cout << output_str;
		cout << t;
		return cout;
	}
protected:
	Plugin_Console_Warn() = default;
	static Plugin_Console_Warn* pWarn_;
};

Console_Error& Error();
Console_Info& Info();
Console_Warn& Warn();
Plugin_Console_Error& PError();
Plugin_Console_Info& PInfo();
Plugin_Console_Warn& PWarn();

#endif // !OUTPUT_H