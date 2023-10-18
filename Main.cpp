//hjiang
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

std::pair<json, string_view> decode_next(std::string_view strv);

auto decode_string(string_view strv){
	// "5:hello" -> "hello"
	auto data=strv.data();
	size_t colon_index=strv.find(":");
	if (colon_index != std::string::npos) {
    		int64_t number = read_int64(data, data + colon_index);
    		string_view str_sv(data + colon_index + 1, number);
    		strv.remove_prefix(colon_index + 1 + number);
		return make_pair(json(str_sv), strv);
	} else {
		throw std::runtime_error("Invalid encoded value: "+ std::string(strv));
	}
}

auto decode_int(string_view strv){
	// "i42e" -> 42
	size_t index=strv.find("e");
	
	if (index != std::string::npos) {
	    auto data = strv.data();
	    strv.remove_prefix(index + 1);
	    return make_pair(json(read_int64(data + 1, data + index)), strv);
	} else {
	    throw std::runtime_error("Invalid encoded value: " + std::string(strv));
	}
}
	
auto decode_list(string_view strv){
	// l5:helloi52ee -> ["hello", 52]
	auto result=json::array();
	strv.remove_prefix(1);
	while (strv[0] != 'e'){
		auto[next, rest]=decode_next(strv);
		result.push_back(next);
		strv=rest;
	};
	strv.remove_prefix(1);
	return make_pair(result, strv);
}


auto decode_map(string_view strv){
	// d3:foo3:bar5:helloi52ee -> {"foo":"bar","hello":52}
	auto result= json::object();
	strv.remove_prefix(1);
	while(strv[0] != 'e'){
		auto[key, rest1]=decode_next(strv);
		auto[value, rest2]=decode_next(rest1);
		strv=rest2;
		result[key]=value;
	};
	strv.remove_prefix(1);
	return make_pair(result, strv);
}


std::pair<json, string_view> decode_next(std::string_view strv){
	if(strv.empty()){
		throw std::runtime_error("Invalid encoded value: " + std::string(strv));
	}
	if (std::isdigit(strv[0])) {
	    return decode_string(strv);
	} else if ('i' == strv[0]) {
	    return decode_int(strv);
	} else if ('l' == strv[0]) {
	    return decode_list(strv);
	} else if ('d' == strv[0]) {
	    return decode_map(strv);
	} else {
	    throw std::runtime_error("Unhandled encoded value: " + std::string(strv));
	}
}

json decode_bencoded_value(const std::string& encoded_value){
	auto [decoded_value, _]= decode_next(encoded_value);
	return decoded_value;
}

	
		
// function to read  metainfo file of torrent
std::string read_file(const std::string &path){

	/*./your_bittorrent.sh info sample.torrent 
	Tracker URL: http://bittorrent-test-tracker.codecrafters.io/announce
	Length: 92063
	*/
	std::ifstream inf(path);
	std::string contents((std::istreambuf_iterator<char>(inf)),
				std::istreambuf_iterator<char>());
	return contents;
};
	
	
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
        
    }else if(command == "info"){
	 if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }
	std::string torrent_file(argv[2]);
	json info = decode_bencoded_value(read_file(torrent_file));
	std::cout  << "Tracker URL: " << info["announce"].get<std::string>()<< std::endl;
    	std::cout << "Length: " << info["info"]["length"] << std::endl;  
 
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}

    return 0;
}
