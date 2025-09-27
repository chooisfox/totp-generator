#include "application.hpp"

int main(const int argc, const char** argv)
{
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	auto main_program = APP::Application(argc, argv);

	return main_program.run();
}
