#ifndef JSONSTR_HPP
#define JSONSTR_HPP

#include <json.hpp>
#include <string>
#include <sstream>
using namespace std;
using json = nlohmann::json;
using json_value_type = nlohmann::json_abi_v3_11_2::detail::value_t;

template<class Source>
string all2str(const Source& sourse) {
	stringstream ss;
	ss << sourse;
	return ss.str();
}

template<>
string all2str<json>(const json& json_obj);

#endif // !JSONSTR_HPP