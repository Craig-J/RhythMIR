#include <Agnostic/logger.h>

namespace agn
{
	// Static member initialization
	Log::ERRORVERBOSITY Log::error_vebosity_ = Log::ERRORVERBOSITY::ALL;
	Log::OUTPUTVERBOSITY Log::output_vebosity_ = Log::OUTPUTVERBOSITY::ALL;
	bool Log::timestamps_ = true;
	bool Log::log_to_file_ = true;
	bool Log::log_to_console_ = false;
	std::ofstream Log::ofstream_("output_log.txt");
	std::stringstream Log::stream_;
}