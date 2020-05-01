#pragma once
#include <Windows.h>
#include <wininet.h>
#include <string.h>
#include <iostream>

#pragma comment (lib, "Wininet.lib")

class inhandle
{
public:
	inhandle() = default;
	explicit inhandle(HINTERNET) noexcept;
	inhandle(const inhandle&) = delete;
	inhandle(inhandle&&);

	~inhandle();

	inhandle& operator=(const inhandle&) = delete;
	inhandle& operator=(inhandle&&);
	inhandle& operator=(HINTERNET);

	operator HINTERNET() const noexcept;
	operator bool() const noexcept;

private:
	HINTERNET handle = NULL;
};



