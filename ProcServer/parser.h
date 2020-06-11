#pragma once

#include <iostream>
#include <sstream>
#include <string.h>


struct Student {
	std::string login, password;
	std::string name;
	std::string department, faculty, specialization, semester;
};

class htmlparser
{
public:
	explicit htmlparser(std::string);
	htmlparser(const htmlparser&) = delete;

	~htmlparser() = default;

	htmlparser& operator=(const htmlparser&) = delete;

	std::string ViewDataParse(std::string);

	void DaneOgolneParsePL(Student&); //Get polish version of student data. Call SetLanguage("pl") before
	void DaneOgolneParseEN(Student&); //Get english version of student data. Call SetLanguage("en") before
	void SetText(std::string _text) { text = _text; } //set text

	bool CheckResponse();

private:
	std::string text;
};

