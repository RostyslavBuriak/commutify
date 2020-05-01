#include "requests.h"


login::login(const std::string login, const std::string password): userlogin(login),userpassword(password) {
	params += 
		"ctl00%24ctl00%24ContentPlaceHolder%24MiddleContentPlaceHolder%24txtIdent=" + login +
		"&ctl00%24ctl00%24ContentPlaceHolder%24MiddleContentPlaceHolder%24txtHaslo=" + password +
		"&ctl00%24ctl00%24ContentPlaceHolder%24MiddleContentPlaceHolder%24butLoguj=Zaloguj&";
}


login::~login() {
	InternetCloseHandle(hsession);
	InternetCloseHandle(hconnection);
	InternetCloseHandle(hloginrequest);
}	


login::login(const login& obj): 
	cookieheader(obj.cookieheader), 
	viewstate (obj.viewstate),
	params(obj.params),
	userlogin(obj.userlogin),
	userpassword(obj.userpassword){

}


login& login::operator=(const login& obj) {
	cookieheader = obj.cookieheader;
	viewstate = obj.viewstate;
	userlogin = obj.userlogin;
	userpassword = obj.userpassword;
	params = obj.params;
	return *this;
}


bool login::SetupConnection() {
	if ((hsession = InternetOpen(L"Mozilla/5.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)", //Initializes an application's use of the WinINet functions
								INTERNET_OPEN_TYPE_PRECONFIG,
								NULL,
								NULL,
								0))== NULL) { //checks if it has successfully worked
		//error occured. throw an exception
	}

	if ((hconnection = InternetConnect( hsession,
										L"wu.wsiz.rzeszow.pl",
										INTERNET_DEFAULT_HTTPS_PORT, // THIS
										L"",
										L"",
										INTERNET_SERVICE_HTTP,
										0,
										0)) == NULL) { //checks if it has successfully worked
		//error occured. throw an exception
	}

	return 0; //everything is OK, return 0;
}



std::string login::GetCookieFromResponse(inhandle& hrequest) {
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


bool login::AddHeaders(inhandle& hrequest, std::string headers) {
	HttpAddRequestHeadersA(hrequest, headers.data(), (DWORD)headers.length(), HTTP_ADDREQ_FLAG_ADD);

	return 0;
}


std::string login::GetHtml(inhandle& hrequest) {
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
		buffer[dwBytesRead] = 0;
		html += buffer;

		if (dwBytesRead == 0)
			break;
	}
	return html;
}


std::string login::TextToUrlEncoded(const std::string& value) {
	std::ostringstream escaped;
	escaped << std::hex; //all integer values will be converted to hexadecimal values 

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		unsigned char c = (unsigned char)(*i);

		if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') { //check if char is an alphabetic or integer value or '_' '.' '~'
			escaped << c; //if it is put it into the stream
			continue; //continue with next iteration
		}

		escaped << std::uppercase << '%' << int(c) << std::nouppercase; //if char has other value it is encoded with '%' at 
																		//the beginning and hexadecimal value of integer value of this char 
																		//must be entered.
																		//Hexadecimal values must be entered only in uppercase
	}

	return escaped.str();
}


bool login::SendRequest(inhandle& request,std::string opt = "") {
	if (opt.empty()) {
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
		while (!HttpSendRequest(request, 0, 0, (LPVOID)opt.data(), opt.length())) {
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

	if (hrequest == NULL) {  //checks if it has successfully worked
		//error occured. throw an exception
	}

	return hrequest;
}


bool login::GetCookies() {
	inhandle hfirstrequest (OpenRequest()); //Create request
	AddHeaders(hfirstrequest, generalheaders); //Add headers to request
	SendRequest(hfirstrequest); //Send request
	SetCookies(GetCookieFromResponse(hfirstrequest));
	InternetCloseHandle(hfirstrequest);

	inhandle hsecondrequest (OpenRequest("GET","/wunet/Logowanie2.aspx"));
	BOOL decoding = 1;
	InternetSetOption(hsecondrequest, INTERNET_OPTION_HTTP_DECODING, &decoding, sizeof(decoding));
	AddHeaders(hsecondrequest, generalheaders);
	AddHeaders(hsecondrequest, "Referer: https://wu.wsiz.rzeszow.pl/ \r\n");
	AddHeaders(hsecondrequest, cookieheader);
	SendRequest(hsecondrequest);
	SetCookies(GetCookieFromResponse(hsecondrequest));
	
	std::string html = GetHtml(hsecondrequest);//get html for response from ../Logowanie2.aspx

	htmlparser parser(html); //this objects will be used for data parsing 
	viewstate += TextToUrlEncoded(parser.ViewDataParse("id=\"__VIEWSTATE\"")); //First parse data then encode it to urlencoded

	InternetCloseHandle(hsecondrequest);

	return 0;
}


bool login::Login() {
	hloginrequest = OpenRequest("POST", "/wunet/Logowanie2.aspx"); //create Login request

	BOOL decoding = 1;
	InternetSetOption(hloginrequest, INTERNET_OPTION_HTTP_DECODING, &decoding, sizeof(decoding)); //set decoding option to recieve inflated data

	AddHeaders(hloginrequest, generalheaders);
	AddHeaders(hloginrequest, "Referer: https://wu.wsiz.rzeszow.pl/wunet/Logowanie2.aspx\r\n");
	AddHeaders(hloginrequest, "Cache-control: no-cache\r\n");
	AddHeaders(hloginrequest, "Content-type: application/x-www-form-urlencoded\r\n");
	AddHeaders(hloginrequest, cookieheader);

	params += viewstate; //add viewstate to params

	SendRequest(hloginrequest, params); //Send login request
	std::string html = GetHtml(hloginrequest);//get html for response from ../Logowanie2.aspx
	SetCookies(GetCookieFromResponse(hloginrequest)); //Add response cookies to our cookies
	InternetCloseHandle(hloginrequest);


	return 0;
}


bool login::GetPersonData() {
	SetupConnection();
	GetCookies();
	Login();	
	return 0;
}
