#include "inhandle.h"


inhandle::inhandle(HINTERNET _handle)noexcept :handle(_handle) {
}


inhandle::inhandle(inhandle&& _handle) : handle(_handle) {
	_handle = NULL;
}


inhandle::~inhandle() {
	InternetCloseHandle(handle);
}


inhandle& inhandle::operator=(inhandle&& _handle) {
	InternetCloseHandle(handle);
	handle = _handle;
	_handle = NULL;
	return *this;
}


inhandle& inhandle::operator=(HINTERNET _handle) {
	InternetCloseHandle(handle);
	handle = _handle;
	return *this;
}


inhandle::operator HINTERNET()const noexcept {
	return handle;
}


inhandle::operator bool()const noexcept {
	if (handle == NULL)
		return 0;

	return 1;
}

