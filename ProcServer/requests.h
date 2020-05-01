#pragma once
#include <memory>
#include <stdio.h>
#include "parser.h"
#include "inhandle.h"

class login
{
public:
	bool GetPersonData();

	login(const std::string, const std::string);
	login(const login& obj);

	~login();

	login& operator=(const login&);
private:
	inhandle hsession,hconnection,hloginrequest;

	std::string userlogin, userpassword, params;

	std::string cookieheader = "Cookie: default_login_type = 0";

	const std::string generalheaders = "Accept: text/html, application/xhtml+xml, image/jxr, */*\r\n"
								 "Accept-Encoding: gzip, deflate\r\n"
								 "Accept-Language: en-US\r\n"
								 "Connection: Keep-Alive\r\n"
								 "Host: wu.wsiz.rzeszow.pl\r\n"
								 "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n";


	std::string viewstate = "__VIEWSTATE=";

	bool GetCookies();
	bool AddHeaders(inhandle&, std::string);
	bool SetupConnection();
	bool SendRequest(inhandle&,std::string);
	bool SetCookies(std::string);
	bool Login();

	HINTERNET OpenRequest(std::string, std::string);

	std::string GetCookieFromResponse(inhandle&);
	std::string  GetHtml(inhandle&);
	std::string TextToUrlEncoded(const std::string&);
};

