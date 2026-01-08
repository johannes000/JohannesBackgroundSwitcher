#pragma once

#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

typedef std::shared_ptr<spdlog::logger> LogPtr;

inline LogPtr GetLogger(const std::string &name, spdlog::level::level_enum level = spdlog::level::info) {
	auto existingLogger = spdlog::get(name);
	if (existingLogger) {
		return existingLogger;
	}
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm = *std::localtime(&now_time_t);

	std::ostringstream timestamp_stream;
	timestamp_stream << std::put_time(&now_tm, "%y%m%d%H%M");
	std::string timestamp = timestamp_stream.str();

	std::string filename = std::format("logs/Wallpaper_{}.log", timestamp);
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);

	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(level);

	std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
	auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::trace);

	spdlog::register_logger(logger);
	return logger;
}
