#include "requests.h"


login::login(Student* _student): student(_student) {
	params += 
		"ctl00%24ctl00%24ContentPlaceHolder%24MiddleContentPlaceHolder%24txtIdent=" + _student->login +
		"&ctl00%24ctl00%24ContentPlaceHolder%24MiddleContentPlaceHolder%24txtHaslo=" + _student->password +
		"&ctl00%24ctl00%24ContentPlaceHolder%24MiddleContentPlaceHolder%24butLoguj=Zaloguj&";
}


login::~login() {
	InternetCloseHandle(hsession);
	InternetCloseHandle(hconnection);
}	


login::login(const login& obj): 
	cookieheader(obj.cookieheader), 
	viewstate (obj.viewstate),
	params(obj.params),
	student(obj.student) {
}


login::login(login&& obj) noexcept:
	cookieheader(std::move(obj.cookieheader)),
	viewstate(std::move(obj.viewstate)),
	params(std::move(obj.params)),
	student(std::move(obj.student)),
	hsession(std::move(obj.hsession)),
	hconnection(std::move(hconnection)) {

	obj.cookieheader = obj.viewstate = obj.params = obj.student->login = obj.student->password = "";
	obj.student->faculty = obj.student->semester = obj.student->specialization = obj.student->department= "";
	obj.hsession = 0;
	obj.hconnection = 0;
	obj.cookieheader = "";
	obj.viewstate = "";
	obj.params = "";
}


login& login::operator=(const login& obj) {
	if (this != &obj) {
		cookieheader = obj.cookieheader;
		viewstate = obj.viewstate;
		student = obj.student;
		params = obj.params;
	}
	return *this;
}


login& login::operator=(login&& obj)noexcept {
	if (this != &obj) {
		cookieheader = std::move(obj.cookieheader);
		viewstate = std::move(obj.viewstate);
		student = std::move(obj.student);
		params = std::move(obj.params);
		hconnection = std::move(obj.hconnection);
		hsession = std::move(obj.hsession);
		obj.cookieheader = obj.viewstate = obj.params = obj.student->login = obj.student->password = "";
		obj.student->faculty = obj.student->semester = obj.student->specialization = obj.student->department = "";
		obj.hsession = 0;
		obj.hconnection = 0;
		obj.cookieheader = "";
		obj.viewstate = "";
		obj.params = "";
	}
	return *this;
}

void login::SetupConnection() {
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

}



std::string login::GetCookieFromResponse(inhandle& hrequest) {
	char buffer[4096]{}; //4096 is the maximum cookie size
	
	DWORD dwsize = 4096; //size of buffer

	HttpQueryInfoA(hrequest, HTTP_QUERY_SET_COOKIE, buffer, &dwsize, NULL);

	std::string cookie (buffer);

	return cookie.substr(0,cookie.find(";")); //cut string from the start until first ';' and then return it
}


void login::SetCookies(std::string cookie) {
	cookieheader += ";" + cookie;
}


void login::AddHeaders(inhandle& hrequest, std::string headers) {
	HttpAddRequestHeadersA(hrequest, headers.data(), (DWORD)headers.length(), HTTP_ADDREQ_FLAG_ADD);
}


std::string toUtf8(const std::wstring& str)
{
	std::string ret;
	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		ret.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len, NULL, NULL);
	}
	return ret;
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


void login::SendRequest(inhandle& request,std::string opt = "") {
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

}


bool login::CheckResponse(inhandle& hresponse) {
	char buffer[5]{}; //enaugh to handle result

	DWORD dwsize = 5; //size of buffer

	HttpQueryInfoA(hresponse, HTTP_QUERY_STATUS_CODE, buffer, &dwsize, NULL); //buffer will be "200" if OK or any other if not

	std::string result = buffer;

	if (result == "200") {
		return 0;
	}

	return 1;
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


void login::GetCookies() {
	inhandle hfirstrequest (OpenRequest()); //Create request
	AddHeaders(hfirstrequest, generalheaders); //Add headers to request
	SendRequest(hfirstrequest); //Send request
	SetCookies(GetCookieFromResponse(hfirstrequest));

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
}


void login::SetLanguage(std::string lan = "pl") {
	if(lan == "en"){
		inhandle changelanrequest(OpenRequest("GET", "/wunet/UstawJezyk.aspx?lang=en&adres=ref")); //create Login request

		AddHeaders(changelanrequest, generalheaders);
		AddHeaders(changelanrequest, "Referer: https://wu.wsiz.rzeszow.pl/wunet/pusta2.aspx\r\n");
		AddHeaders(changelanrequest, "Cache-control: no-cache\r\n");
		AddHeaders(changelanrequest, cookieheader);

		std::string lanparams = "adres=ref&lang=en";

		SendRequest(changelanrequest, params); //Send change language request

		InternetCloseHandle(changelanrequest);
	}
}


void login::Login() {
	inhandle hloginrequest(OpenRequest("POST", "/wunet/Logowanie2.aspx")); //create Login request

	BOOL decoding = 1;
	InternetSetOption(hloginrequest, INTERNET_OPTION_HTTP_DECODING, &decoding, sizeof(decoding)); //set decoding option to recieve inflated data

	AddHeaders(hloginrequest, generalheaders);
	AddHeaders(hloginrequest, "Referer: https://wu.wsiz.rzeszow.pl/wunet/Logowanie2.aspx\r\n");
	AddHeaders(hloginrequest, "Cache-control: no-cache\r\n");
	AddHeaders(hloginrequest, "Content-type: application/x-www-form-urlencoded\r\n");
	AddHeaders(hloginrequest, cookieheader);

	params += viewstate; //add viewstate to params

	SendRequest(hloginrequest, params); //Send login request
	SetCookies(GetCookieFromResponse(hloginrequest)); //Add response cookies to our cookies

	InternetCloseHandle(hloginrequest);
}


void login::GetStudentData() {
	inhandle hdaneogolne(OpenRequest("GET","/wunet/Wynik2.aspx")); //create GET request to get student data

	BOOL decoding = 1;
	InternetSetOption(hdaneogolne, INTERNET_OPTION_HTTP_DECODING, &decoding, sizeof(decoding)); //set decoding option to recieve inflated data

	AddHeaders(hdaneogolne, generalheaders);
	AddHeaders(hdaneogolne, "Referer: https://wu.wsiz.rzeszow.pl/wunet/pusta2.aspx\r\n");
	AddHeaders(hdaneogolne, "Cache-control: no-cache\r\n");
	AddHeaders(hdaneogolne, cookieheader);

	SendRequest(hdaneogolne);
	std::string html = GetHtml(hdaneogolne);//get html for response from ../Logowanie2.aspx

	htmlparser parser(html);
	parser.DaneOgolneParseEN(*student);

}


bool login::GetPersonData() {
	SetupConnection();
	GetCookies();
	Login();
	SetLanguage("en");
	GetStudentData();
	return 0;
}

