#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <iostream>
#include "requests.h"
//#include <string.h>
#pragma comment (lib, "Wininet.lib")

int main(int argc, char* argv[]) {

	//HINTERNET hSession = InternetOpen(
	//	L"Mozilla/5.0",
	//	INTERNET_OPEN_TYPE_PRECONFIG,
	//	NULL,
	//	NULL,
	//	0);

	//HINTERNET hConnect = InternetConnect(
	//	hSession,
	//	L"wu.wsiz.rzeszow.pl",
	//	INTERNET_DEFAULT_HTTPS_PORT, // THIS
	//	L"",
	//	L"",
	//	INTERNET_SERVICE_HTTP,
	//	0,
	//	0);

	//HINTERNET hHttpFile = HttpOpenRequest(
	//	hConnect,
	//	L"GET",
	//	L"/",
	//	NULL,
	//	NULL,
	//	NULL,
	//	INTERNET_FLAG_SECURE, // THIS
	//	0);

	//while (!HttpSendRequest(hHttpFile, NULL, 0, 0, 0)) {
	//	printf("HttpSendRequest error : (%lu)\n", GetLastError());

	//	InternetErrorDlg(
	//		GetDesktopWindow(),
	//		hHttpFile,
	//		ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED,
	//		FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
	//		FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
	//		FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
	//		NULL);
	//}
	//char buf[9999]{};
	//DWORD dwsize = 9999;
	//DWORD dwFileSize;
	//dwFileSize = BUFSIZ;
	//HttpQueryInfoA(hHttpFile, HTTP_QUERY_RAW_HEADERS_CRLF, buf, &dwsize, NULL);

	//for (int i = 0; i < dwsize; ++i) {
	//	std::cout << buf[i];
	//}

	//char buffer[513]{};
	//std::cout << GetLastError() << std::endl;
	//while (true) {
	//	DWORD dwBytesRead;
	//	BOOL bRead;

	//	bRead = InternetReadFile(
	//		hHttpFile,
	//		buffer,
	//		dwFileSize + 1,
	//		&dwBytesRead);

	//	if (dwBytesRead == 0) break;

	//	if (!bRead) {
	//		printf("InternetReadFile error : <%lu>\n", GetLastError());
	//	}
	//	else {
	//		std::cout << std::endl << dwBytesRead << std::endl;
	//		std::cout << buffer;
	//		
	//	}
	//}

	//InternetCloseHandle(hHttpFile);
	//InternetCloseHandle(hConnect);
	//InternetCloseHandle(hSession);

	login l;
	l.GetPersonData();
	return 0;
}