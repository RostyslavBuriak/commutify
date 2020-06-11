#pragma once
#include <memory>
#include <stdio.h>
#include "parser.h"
#include "inhandle.h"


class login
{
public:
	bool GetPersonData();

	login(Student*);
	login(const login& obj);
	login(login&&) noexcept;

	~login();

	login& operator=(const login&);
	login& operator=(login&&) noexcept;
private:
	Student* student;

	inhandle hsession,hconnection;

	std::string params;

	std::string cookieheader = "Cookie: default_login_type = 0";

	const std::string generalheaders = "Accept: text/html, application/xhtml+xml, image/jxr, */*\r\n"
								 "Accept-Encoding: gzip, deflate\r\n"
								 "Accept-Language: en-US\r\n"
								 "Connection: Keep-Alive\r\n"
								 "Host: wu.wsiz.rzeszow.pl\r\n"
								 "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n";


	std::string viewstate = "__VIEWSTATE=";

	void GetCookies();
	void AddHeaders(inhandle&, std::string);
	void SetupConnection();
	void SendRequest(inhandle&,std::string);
	void SetCookies(std::string);
	void SetLanguage(std::string);
	bool Login();
	void GetStudentData();
	void GetStudentSubjectsData();

	HINTERNET OpenRequest(std::string, std::string);

	std::string GetCookieFromResponse(inhandle&);
	std::string GetHtml(inhandle&);
	std::string TextToUrlEncoded(const std::string&);

	bool CheckResponse(inhandle&);
};

