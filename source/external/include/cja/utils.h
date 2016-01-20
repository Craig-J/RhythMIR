#pragma once
// Contains various utilities - mostly for debugging

// MACROS
#define DeleteNull(x) if(x != nullptr) { delete x; x = nullptr; }
#define DeleteArrayNull(x) if(x != nullptr) { delete[] x; x = nullptr; }
#define ReleaseNull(x) if(x != nullptr) { x->Release(); x = nullptr; }

// FUNCTIONS

#include <io.h>
#include <iostream>

static void die(const char *message)
{
	fprintf(stderr, "ERROR: %s \n", message);

#ifdef _DEBUG
	// Debug build -- drop the program into the debugger.
	abort();
#else
	exit(1);
#endif
}

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fcntl.h>
//#include <cstdio>
//#include <cstdlib>

// In a Win32 application, open a console window and redirect the
// standard input, output and error streams to it.
static void handleToConsole(DWORD handle, FILE *stream, const char *mode)
{
	long lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	*stream = *_fdopen(hConHandle, "w");
	setvbuf(stream, NULL, _IONBF, 0);
}

static void openConsoleWindow()
{
	// Allocate a console for this app.
	AllocConsole();

	// Make the screen buffer big enough to let us scroll text.
	const WORD MAX_CONSOLE_LINES = 450;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// Redirect the C IO streams (stdout etc.) to the console.
	handleToConsole(STD_INPUT_HANDLE, stdin, "r");
	handleToConsole(STD_OUTPUT_HANDLE, stdout, "w");
	handleToConsole(STD_ERROR_HANDLE, stderr, "w");

	// Redirect the C++ IO streams (cout etc.) to the console.
	std::ios::sync_with_stdio();
}