#pragma once

#include "utility/Includes.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/string.hpp>

#define SETTINGS_DEFINITIONS                                \
	SETTING(bool, RenderFilenameInBackground, false)        \
	SETTING(i32, WallpaperIntervalInSeconds, 300 /*5 min*/) \
                                                            \
	SETTING(std::string, StatsPath, "./stats.json")         \
	SETTING(std::string, SettingsPath, "./settings.json")   \
	SETTING(std::string, DefaultSettingsPath, "")           \
	SETTING(std::string, TTFFilePath, "./assets/font/Lato-Regular.ttf")

enum class Setting {
#define SETTING(type, name, default) name,
	SETTINGS_DEFINITIONS
#undef SETTING
};

class Settings {
public:
private:
	Settings();
	~Settings();

	Settings(const Settings &) = delete;
	Settings &operator=(const Settings &) = delete;
	Settings(Settings &&) = delete;
	Settings &operator=(Settings &&) = delete;

public:
	static auto GetInstance() -> Settings &;
	static auto Init() -> void;
	static auto Shutdown() -> void;

	auto SaveToFile(fs::path path = "./settings.json") -> void;
	auto LoadFromFile(fs::path path = "./settings.json") -> void;

	template <Setting S>
	constexpr auto Get() const -> const auto &;

	template <Setting S>
	constexpr auto Set(const auto &value) -> void;

	template <class Archive>
	void serialize(Archive &archive) {
#define SETTING(type, name, default) archive(cereal::make_nvp(#name, m##name));
		SETTINGS_DEFINITIONS
#undef SETTING
	}

private:
	LogPtr log;

// Member vars
#define SETTING(type, name, default) type m##name = default;
	SETTINGS_DEFINITIONS
#undef SETTING
};

template <Setting S>
constexpr auto Settings::Get() const -> const auto & {
	if constexpr (S == Setting::RenderFilenameInBackground) {
		return mRenderFilenameInBackground;
	} else if constexpr (S == Setting::WallpaperIntervalInSeconds) {
		return mWallpaperIntervalInSeconds;
	} else if constexpr (S == Setting::StatsPath) {
		return mStatsPath;
	} else if constexpr (S == Setting::SettingsPath) {
		return mSettingsPath;
	} else if constexpr (S == Setting::DefaultSettingsPath) {
		return mDefaultSettingsPath;
	} else if constexpr (S == Setting::TTFFilePath) {
		return mTTFFilePath;
	} else {
		static_assert(!std::is_same_v<void, void>, "Unbekanntes Setting");
	}
}

template <Setting S>
constexpr auto Settings::Set(const auto &value) -> void {
	bool changed = false;

	// Explizite if-else Kette (zuverlässiger als Makro)
	if constexpr (S == Setting::RenderFilenameInBackground) {
		changed = true;
		mRenderFilenameInBackground = value;
	} else if constexpr (S == Setting::WallpaperIntervalInSeconds) {
		changed = true;
		mWallpaperIntervalInSeconds = value;
	} else if constexpr (S == Setting::StatsPath) {
		changed = true;
		mStatsPath = value;
	} else if constexpr (S == Setting::SettingsPath) {
		changed = true;
		mSettingsPath = value;
	} else if constexpr (S == Setting::DefaultSettingsPath) {
		changed = true;
		mDefaultSettingsPath = value;
	} else if constexpr (S == Setting::TTFFilePath) {
		changed = true;
		mTTFFilePath = value;
	} else {
		static_assert(!std::is_same_v<void, void>, "Unbekanntes Setting");
	}

	if (changed) {
		SaveToFile();
	}
}

inline auto S() -> Settings & {
	return Settings::GetInstance();
}

template <Setting S>
constexpr auto GetSetting() -> const auto & {
	return ::S().Get<S>();
}

template <Setting S>
constexpr void SetSetting(const auto &value) {
	::S().Set<S>(value);
}