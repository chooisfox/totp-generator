#ifndef SPDLOG_WRAPPER_HPP
#define SPDLOG_WRAPPER_HPP

#include "global_names.hpp"

#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#define FMT_COLOR_BLACK	 "\033[30m"
#define FMT_COLOR_RED	 "\033[31m"
#define FMT_COLOR_GREEN	 "\033[32m"
#define FMT_COLOR_YELLOW "\033[33m"
#define FMT_COLOR_BLUE	 "\033[34m"
#define FMT_COLOR_PURPLE "\033[35m"
#define FMT_COLOR_CYAN	 "\033[36m"
#define FMT_COLOR_WHITE	 "\033[37m"
#define FMT_COLOR_RESET	 "\033[0m"

#define SPD_LOG_CLASS(level, class_name, message)                                                                                                  \
	spdlog::log(level, fmt::format("[{}{}{}] {}", FMT_COLOR_CYAN, class_name, FMT_COLOR_RESET, message))

#define SPD_DEBUG_CLASS(class_name, message) SPD_LOG_CLASS(spdlog::level::debug, class_name, message)

#define SPD_INFO_CLASS(class_name, message) SPD_LOG_CLASS(spdlog::level::info, class_name, message)

#define SPD_WARN_CLASS(class_name, message) SPD_LOG_CLASS(spdlog::level::warn, class_name, message)

#define SPD_ERROR_CLASS(class_name, message) SPD_LOG_CLASS(spdlog::level::err, class_name, message)

#endif // SPDLOG_WRAPPER_HPP
