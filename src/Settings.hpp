#pragma once

#include "utility/Includes.hpp"

enum class Setting {
	RenderFilename,
	WallpaperInterval,
	BackgroundOnClose,
	BrowserCustomArguments,
};

class Settings {
private:
	Settings();
	~Settings();

public:
	Settings(const Settings &) = delete;
	Settings &operator=(const Settings &) = delete;
	Settings(Settings &&) = delete;
	Settings &operator=(Settings &&) = delete;

	static auto GetInstance() -> Settings &;
	static auto Init() -> void;
	static auto Shutdown() -> void;

	template <typename T>
	auto Get(Setting setting) const -> T;

	template <typename T>
	auto Set(Setting setting, T value) -> void;

private:
	LogPtr log;
};
