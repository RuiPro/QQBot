#ifndef SQL_HPP
#define SQL_HPP

class SQL {
public:
	inline SQL() {
		m_arg_set = new string[50];
	}
	inline explicit SQL(const string& str) : m_sql_str(str) {
		m_arg_set = new string[50];
	}
	inline explicit SQL(const char* char_arr) : m_sql_str(char_arr) {
		m_arg_set = new string[50];
	}
	inline explicit SQL(string&& str) : m_sql_str(std::move(str)) {
		m_arg_set = new string[50];
	}
	~SQL() {
		delete[] m_arg_set;
	}
	SQL& operator=(const string& str) {
		m_sql_str = str;
		return *this;
	}
	SQL& operator=(const char* char_arr) {
		m_sql_str = char_arr;
		return *this;
	}
	string getStr() const;
	// ��������е�ռλ��%index->target
	// transfer���Ƿ�ʹ��ת�壬��SQL����е�'ת���''
	template<class T>
	void args(int index, const T& t, bool transfer = true) {
		if (index <= 0 || index > 50) return;
		string target = all2str(t);
		if (transfer) {
			for (auto& element : target) {
				if (element == '\'') {
					m_arg_set[index - 1].push_back('\'');
				}
				m_arg_set[index - 1].push_back(element);
			}
		}
		else {
			m_arg_set[index - 1] = std::move(target);
		}
	}
private:
	string m_sql_str;
	string* m_arg_set;
};

#endif // !SQL_HPP