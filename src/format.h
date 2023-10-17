#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "all2str.h"
using namespace std;

class Format {
public:
	Format(const string& str) : m_str(str) {}
	Format(string&& str) : m_str(std::move(str)) {}
	Format(const char* str_arr) : m_str(std::move(string(str_arr))) {}
	~Format() {}

	template<class T>
	void arg(int index, const T& t) {
		m_arg_set.insert(std::move(std::pair<int, string>(index, all2str(t))));
	}

	string str() const;
protected:
	string m_str;
	std::unordered_map<int, string> m_arg_set;
};

#endif