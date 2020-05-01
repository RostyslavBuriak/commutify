#pragma once

#include <iostream>
#include <sstream>
#include <string.h>

class htmlparser
{
public:
	explicit htmlparser(std::string);
	htmlparser(const htmlparser&) = delete;

	~htmlparser() = default;

	htmlparser& operator=(const htmlparser&) = delete;

	std::string ViewDataParse(std::string);
	std::string SrudentDataParse(std::string);
private:
	std::string text;
};

