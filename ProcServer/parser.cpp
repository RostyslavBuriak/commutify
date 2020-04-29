#include "parser.h"

htmlparser::htmlparser(std::string _text) :text(_text) {

}

std::string htmlparser::Find(std::string value) {
	std::string word;
	std::istringstream sstream(text); //divide text by wrods
	while (sstream >> word) {//while there are still words
		if (word == value) {//if we find a needed word
			while (sstream >> word) { //starts from word which we found
				if (word.find("value")!= std::string::npos) { //our main goal is to find "value" 
					return word.substr(word.find("value") + 6, std::string::npos); //if we find a "value" return value of this "value"
				}
			}
		}
	} 
	return "";
}