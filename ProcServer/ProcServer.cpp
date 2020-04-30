#include <windows.h>
#include "requests.h"
#pragma comment (lib, "Wininet.lib")

int main() {

	login l;
	l.GetPersonData();
	return 0;
}