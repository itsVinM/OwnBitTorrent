#include <cctype>
#include <charconv>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;
using std::string_view;


int64_t read_int64(const char *begin, const char *end){
	int64_t number;
	auto [_, ec]=std::from_chars(begin, end, number);
	if (ec !=std::errc()){
	  throw std::runtime_error("Invalid number");
	}
	return number;
}

std::pair<json, string_view> decode_next(std::string_view sv);

auto decode_string(string_view sv){
	// "5:hello" -> "hello"
	auto data=sv.data();
	size_t colon_index=sv.find(":");
	if (colon_index != std::string::npos) {
    		int64_t number = read_int64(data, data + colon_index);
    		string_view str_sv(data + colon_index + 1, number);
    		sv.remove_prefix(colon_index + 1 + number);
		return make_pair(json(str_sv), sv);
	} else {
		throw std::runtime_error("Invalid encoded value: "+ std::string(sv));
	}
}

auto decode_int(string_view sv){
	// "i42e" -> 42
	size_t index=sv.find("e");
	
	if (index != std::string::npos) {
	    auto data = sv.data();
	    sv.remove_prefix(index + 1);
	    return make_pair(json(read_int64(data + 1, data + index)), sv);
	} else {
	    throw std::runtime_error("Invalid encoded value: " + std::string(sv));
	}
}
	
auto decode_list(string_view sv){
	// l5:helloi52ee -> ["hello", 52]
	auto result=json::array();
	sv.remove_prefix(1);
	while (sv[0] != 'e'){
		auto[next, rest]=decode_next(sv);
		result.push_back(next);
		sv=rest;
	};
	sv.remove_prefix(1);
	return make_pair(result, sv);
}


auto decode_map(string_view sv){
	// d3:foo3:bar5:helloi52ee -> {"foo":"bar","hello":52}
	auto result= json::object();
	sv.remove_prefix(1);
	while(sv[0] != 'e'){
		auto[key, rest1]=decode_next(sv);
		auto[value, rest2]=decode_next(rest1);
		sv=rest2;
		result[key]=value;
	};
	sv.remove_prefix(1);
	return make_pair(result, sv);
}


std::pair<json, string_view> decode_next(std::string_view sv){
	if(sv.empty()){
		throw std::runtime_error("Invalid encoded value: " + std::string(sv));
	}
	if (std::isdigit(sv[0])) {
	    return decode_string(sv);
	} else if ('i' == sv[0]) {
	    return decode_int(sv);
	} else if ('l' == sv[0]) {
	    return decode_list(sv);
	} else if ('d' == sv[0]) {
	    return decode_map(sv);
	} else {
	    throw std::runtime_error("Unhandled encoded value: " + std::string(sv));
	}
}

json decode_bencoded_value(const std::string& encoded_value){
	auto [decoded_value, _]= decode_next(encoded_value);
	return decoded_value;
}

	
		
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
        return 1;
    }

    string_view command = argv[1];

    if (command == "decode") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }

        std::string encoded_value = argv[2];
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
 
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
