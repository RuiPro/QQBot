#include "format.h"

string Format::str() const {
	size_t start = 0;
	size_t pos = 0;
	stringstream ss;
	while (true) {
		// ��start��ʼ��"{"
		pos = m_str.find('{', start);
		// �Ҳ����Ͱ�start����Ķ���ȫ���ӵ�ss��
		if (pos == m_str.npos) {
			ss << m_str.substr(start);
			break;
		}
		// �ҵ��˰�start����pos - start���ַ��ӵ�ss��
		ss << m_str.substr(start, pos - start);
		// �����������"{"�ţ�ת��������"{"
		if (m_str[pos + 1] == '{') {
			ss << '{';
			start = pos + 2;
			continue;
		}
		// ��"{"����һλλ�ÿ�ʼ��"}"
		start = pos + 1;
		pos = m_str.find('}', start);
		// �Ҳ����Ͱ�"{"����Ķ���ȫ���ӵ�ss��
		if (pos == m_str.npos) {
			ss << m_str.substr(start);
			break;
		}

		// itemΪ{}�м�����ݣ�substr�Ľ�ȡ�����������ĵ�һ��λ��
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