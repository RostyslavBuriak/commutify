#pragma once
#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "parser.h"

class login
{
public:
	bool GetPersonData();

	~login();

private:
	HINTERNET hsession = 0,hconnection = 0,hloginrequest = 0, hfirstrequest = 0, hsecondrequest = 0;

	std::string cookieheader = "Cookie: default_login_type = 0";

	std::string generalheaders = "Accept: text/html, application/xhtml+xml, image/jxr, */*\r\n"
								 "Accept-Encoding: gzip, deflate\r\n"
								 "Accept-Language: en-US\r\n"
								 "Connection: Keep-Alive\r\n"
								 "Host: wu.wsiz.rzeszow.pl\r\n"
								 "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko\r\n";

	std::string viewstate = "__VIEWSTATE: ";
	std::string viewstategenerator = "__VIEWSTATEGENERATOR: ";

	bool GetCookies();
	bool AddHeaders(HINTERNET&, std::string);
	bool SetupConnection();
	bool SendRequest(HINTERNET&,char *,unsigned int);
	bool SetCookies(std::string);
	bool Login();

	HINTERNET OpenRequest(std::string, std::string);

	std::string GetCookieFromResponse(HINTERNET&);
	std::string  GetHtml(HINTERNET);
	std::string InflateData(std::string);
};

