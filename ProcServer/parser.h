#pragma once

#include <iostream>
#include <sstream>
#include <string.h>

class htmlparser
{
public:
	explicit htmlparser(std::string);

	std::string Find(std::string);
private:
	std::string text;
};

