#include <windows.h>
#include "requests.h"
#pragma comment (lib, "Wininet.lib")

int main(int argc, char* argv[]) {

	login l;
	l.GetPersonData();
	return 0;
}