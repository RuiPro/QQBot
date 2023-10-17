#include "all2str.h"

template<>
string all2str<json>(const json& json_obj) {
	auto type = json_obj.type();
	switch (type) {
	case json_value_type::null:
		return string();
	case json_value_type::string:
		return json_obj.get<string>();
	case json_value_type::boolean:
		return json_obj.get<bool>() ? "1" : "0";
	case json_value_type::number_integer:
		return to_string(json_obj.get<long>());
	case json_value_type::number_unsigned:
		return to_string(json_obj.get<unsigned long>());
	case json_value_type::number_float:
		return to_string(json_obj.get<float>());
	default:
		return json_obj.dump();
	}
}