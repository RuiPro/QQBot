#include "format.h"

string Format::str() const {
	size_t start = 0;
	size_t pos = 0;
	stringstream ss;
	while (true) {
		// 从start开始找"{"
		pos = m_str.find('{', start);
		// 找不到就把start后面的东西全都加到ss里
		if (pos == m_str.npos) {
			ss << m_str.substr(start);
			break;
		}
		// 找到了把start往后pos - start个字符加到ss里
		ss << m_str.substr(start, pos - start);
		// 如果连着两个"{"号，转义这两个"{"
		if (m_str[pos + 1] == '{') {
			ss << '{';
			start = pos + 2;
			continue;
		}
		// 从"{"的下一位位置开始找"}"
		start = pos + 1;
		pos = m_str.find('}', start);
		// 找不到就把"{"后面的东西全都加到ss里
		if (pos == m_str.npos) {
			ss << m_str.substr(start);
			break;
		}

		// item为{}中间的内容，substr的截取区间包括传入的第一个位置
		string item = m_str.substr(start, pos - start);
		int index = 0;
		try {
			if (item.empty()) throw 0;
			index = std::stol(item.c_str());
			auto iter = m_arg_set.find(index);
			if (iter == m_arg_set.end()) throw 0;
			ss << iter->second;
		}
		catch (...) {
			ss << '{' << item << '}';
			start = pos + 1;
			continue;
		}
		start = pos + 1;
	}
	return ss.str();
}