#include "requests.h"


login::~login() {
	InternetCloseHandle(hsession);
	InternetCloseHandle(hconnection);
	InternetCloseHandle(hloginrequest);
}	


bool login::SetupConnection() {
	hsession = InternetOpen(
		L"Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	hconnection = InternetConnect(
		hsession,
		L"wu.wsiz.rzeszow.pl",
		INTERNET_DEFAULT_HTTPS_PORT, // THIS
		L"",
		L"",
		INTERNET_SERVICE_HTTP,
		0,
		0);

	return 0;
}



std::string login::GetCookieFromResponse(HINTERNET& hrequest) {
	char buffer[4096]{}; //4096 is the maximum cookie size
	
	DWORD dwsize = 4096; //size of buffer

	HttpQueryInfoA(hrequest, HTTP_QUERY_SET_COOKIE, buffer, &dwsize, NULL);

	std::string cookie (buffer);

	return cookie.substr(0,cookie.find(";")); //cut string from the start until first ';' and then return it
}


bool login::SetCookies(std::string cookie) {

		cookieheader += ";" + cookie;

	return 0;
}


bool login::AddHeaders(HINTERNET& hrequest, std::string headers) {
	HttpAddRequestHeadersA(hrequest, headers.data(), headers.length(), HTTP_ADDREQ_FLAG_ADD);

	return 0;
}


std::string  login::GetHtml(HINTERNET hrequest) {
	std::string html;
	const size_t sz = 512;
	char buffer[sz + 1];
	while (true) {
		DWORD dwBytesRead;
		BOOL bRead;


		bRead = InternetReadFile(
			hrequest,
			buffer,
			sz,
			&dwBytesRead);

		if (!bRead)
		{
			printf("InternetReadFile error : <%lu>\n", GetLastError());
			break;
		}
		std::cout << buffer;
		buffer[dwBytesRead] = 0;
		html += buffer;

		if (dwBytesRead == 0)
			break;
	}
	return html;
}


bool login::SendRequest(HINTERNET& request,char * opt = nullptr,unsigned int optsize = 0) {
	if (opt == nullptr) {
		while (!HttpSendRequest(request, 0, 0, 0, 0)) {
			printf("HttpSendRequest error : (%lu)\n", GetLastError());
			InternetErrorDlg(
				GetDesktopWindow(),
				request,
				ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED,
				FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
				FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
				FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
				NULL);
		}
	}
	else{
		while (!HttpSendRequest(request, 0, 0, opt, optsize)) {
			printf("HttpSendRequest error : (%lu)\n", GetLastError());
			InternetErrorDlg(
				GetDesktopWindow(),
				request,
				ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED,
				FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
				FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
				FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
				NULL);
		}

	}


	return 0;
}


HINTERNET login::OpenRequest(std::string verb = "GET", std::string destination = "/") {
	std::wstring wverb, wdestination;
	wverb = std::wstring(verb.begin(), verb.end());
	wdestination = std::wstring(destination.begin(), destination.end()); //convert string to wstring

	HINTERNET hrequest = HttpOpenRequest(
		hconnection,
		wverb.c_str(),
		wdestination.c_str(),
		NULL,
		NULL,
		NULL,
		INTERNET_FLAG_SECURE,
		0);

	return hrequest;
}


bool login::GetCookies() {
	hfirstrequest = OpenRequest(); //Create request
	AddHeaders(hfirstrequest, generalheaders); //Add headers to request
	SendRequest(hfirstrequest); //Send request
	SetCookies(GetCookieFromResponse(hfirstrequest));
	InternetCloseHandle(hfirstrequest);

	hsecondrequest = OpenRequest("GET","/wunet/Logowanie2.aspx");
	BOOL decoding = 1;
	InternetSetOption(hsecondrequest, INTERNET_OPTION_HTTP_DECODING, &decoding, sizeof(decoding));
	AddHeaders(hsecondrequest, generalheaders);
	AddHeaders(hsecondrequest, "Referer: https://wu.wsiz.rzeszow.pl/ \r\n");
	AddHeaders(hsecondrequest, cookieheader);
	SendRequest(hsecondrequest);
	SetCookies(GetCookieFromResponse(hsecondrequest));
	char buf[9999]{};
	DWORD dwsize = 9999;
	DWORD dwFileSize;
	dwFileSize = BUFSIZ;
	HttpQueryInfoA(hsecondrequest, HTTP_QUERY_RAW_HEADERS_CRLF, buf, &dwsize, NULL);
	std::cout << buf << std::endl;
	

	std::string html = GetHtml(hsecondrequest);//get html for response from ../Logowanie2.aspx
	htmlparser parser(html); //this objects will be used for data parsing 
	viewstate += parser.Find("id=\"__VIEWSTATE\"");
	viewstategenerator += parser.Find("id=\"__VIEWSTATEGENERATOR\"");
	InternetCloseHandle(hsecondrequest);

	return 0;
}


bool login::Login() {
	hloginrequest = OpenRequest("POST", "/wunet/Logowanie2.aspx");
	AddHeaders(hloginrequest, generalheaders);
	AddHeaders(hloginrequest, "Referer: https://wu.wsiz.rzeszow.pl/wunet/Logowanie2.aspx \r\n");
	AddHeaders(hloginrequest, "Cache-control: no-cache\r\n");
	AddHeaders(hloginrequest, "Content type: application/x-www-form-urlencoded\r\n");
	AddHeaders(hloginrequest, "Content-Length: 229\r\n");
	AddHeaders(hloginrequest, cookieheader);
	char buff[] = "ctl00$ctl00$ContentPlaceHolder$MiddleContentPlaceHolder$txtIdent: \"w58926\"\r\nctl00$ctl00$ContentPlaceHolder$MiddleContentPlaceHolder$txtHaslo: \"bazuk2006AZ\"\r\nctl00$ctl00$ContentPlaceHolder$MiddleContentPlaceHolder$butLoguj: \"Zaloguj\"";
	SendRequest(hloginrequest,buff,233);

	

	SetCookies(GetCookieFromResponse(hloginrequest));

	return 0;
}


bool login::GetPersonData() {
	SetupConnection();
	GetCookies();
	//Login();
	return 0;
}
