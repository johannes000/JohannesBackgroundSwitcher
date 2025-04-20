#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

typedef std::shared_ptr<spdlog::logger> LogPtr;

inline LogPtr AddLogger(const std::string &name, spdlog::level::level_enum level = spdlog::level::info) {

	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm now_tm = *std::localtime(&now_time_t);

	std::ostringstream timestamp_stream;
	timestamp_stream << std::put_time(&now_tm, "%y%m%d%H%M");
	std::string timestamp = timestamp_stream.str();

	std::string filename = std::format("logs/{}.log", timestamp);

	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
	std::vector<spdlog::sink_ptr> sinks = {console_sink, file_sink};
	auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

	logger->set_level(level);

	spdlog::register_logger(logger);
	logger->info("{} Logger Added", name);
	return logger;
}
