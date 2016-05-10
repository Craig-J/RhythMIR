#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sstream>

namespace agn
{
	class Log
	{
	public:

		// FatalError
		// Use to log fatal errors to error stream.
		static void FatalError(std::string _message)
		{
			std::string string;
			if (timestamps_)
			{
				string.append(TimeStamp());
			}
			string.append(" fatal error > " + _message);

			if (log_to_console_)
				std::cout << string << std::endl;
			if (log_to_file_)
				ofstream_ << string << std::endl;
			stream_ << string << std::endl;

#ifdef _DEBUG
			abort();
#else
			exit(1);
#endif
		}

		// Error
		// Use to log non-fatal errors to error stream.
		static void Error(std::string _message)
		{
			if (error_vebosity_ != ERRORVERBOSITY::FATAL_ERRORS_ONLY)
			{
				std::string string;
				if (timestamps_)
				{
					string.append(TimeStamp());
				}
				string.append(" error > " + _message);

				if (log_to_console_)
					std::cout << string << std::endl;
				if (log_to_file_)
					ofstream_ << string << std::endl;
				stream_ << string << std::endl;
			}
		}

		// Warning
		// Use to log warnings to error stream.
		static void Warning(std::string _message)
		{
			if (error_vebosity_ == ERRORVERBOSITY::ALL)
			{
				std::string string;
				if (timestamps_)
				{
					string.append(TimeStamp());
				}
				string.append(" warning > " + _message);

				if (log_to_console_)
					std::cout << string << std::endl;
				if (log_to_file_)
					ofstream_ << string << std::endl;
				stream_ << string << std::endl;
			}
		}

		// Important
		// Use to log important information to cout.
		static void Important(std::string _message)
		{
			if (output_vebosity_ != OUTPUTVERBOSITY::NONE)
			{
				std::string string;
				if (timestamps_)
				{
					string.append(TimeStamp());
				}
				string.append(" important > " + _message);

				if (log_to_console_)
					std::cout << string << std::endl;
				if (log_to_file_)
					ofstream_ << string << std::endl;
				stream_ << string << std::endl;
			}
		}

		// Message
		// Use to log information to cout.
		static void Message(std::string _message)
		{
			if (output_vebosity_ == OUTPUTVERBOSITY::ALL)
			{
				std::string string;
				if (timestamps_)
				{
					string.append(TimeStamp());
				}
				string.append(_message);

				if (log_to_console_)
					std::cout << string << std::endl;
				if (log_to_file_)
					ofstream_ << string << std::endl;
				stream_ << string << std::endl;
			}
		}

		static std::stringstream& CaptureStreamOutput()
		{
			return stream_;
		}

		static void EnableConsole()
		{
			log_to_console_ = true;

			// Allocate a console for this app.
			AllocConsole();

			// Make the screen buffer big enough to let us scroll text.
			const WORD MAX_CONSOLE_LINES = 450;
			CONSOLE_SCREEN_BUFFER_INFO coninfo;
			GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
			coninfo.dwSize.Y = MAX_CONSOLE_LINES;
			SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

			// Redirect the C/C++ IO streams to the console.
			BindIOToConsole();
		}

	private:

		static enum class ERRORVERBOSITY { ALL, ERRORS_ONLY, FATAL_ERRORS_ONLY } error_vebosity_;
		static enum class OUTPUTVERBOSITY { ALL, IMPORTANT_ONLY, NONE } output_vebosity_;
		static bool timestamps_;
		static bool log_to_file_;
		static bool log_to_console_;
		static std::ofstream ofstream_; // File output stream.
		static std::stringstream stream_; // Extra output stream for consumption by external code.

		static void BindIOToConsole()
		{
			// Redirect the CRT standard input, output, and error handles to the console
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);

			// Clear the error state for each of the C++ standard stream objects. We need to do this, as
			// attempts to access the standard streams before they refer to a valid target will cause the
			// iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
			// to always occur during startup regardless of whether anything has been read from or written to
			// the console or not.
			std::wcout.clear();
			std::cout.clear();
			std::wcerr.clear();
			std::cerr.clear();
			std::wcin.clear();
			std::cin.clear();
		}

		static std::string TimeStamp()
		{
			std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			char time[100];
			std::strftime(time, sizeof(time), "[%x : %X] ", std::localtime(&now));
			return(time);
		}
	};
}
