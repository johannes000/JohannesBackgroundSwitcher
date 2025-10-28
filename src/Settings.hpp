#pragma once

#include "utility/Includes.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/string.hpp>

#define SETTINGS_DEFINITIONS                                                        \
	SETTING(bool, GUIRenderFilenameInBackground, true)                              \
	SETTING(bool, GUIRenderFilenameInBackgroundOutline, true)                       \
	SETTING(i32, WallpaperIntervalInSeconds, 300 /*5 min*/)                         \
	SETTING(bool, SeperateWallpapersForEachMonitor, true)                           \
                                                                                    \
	SETTING(i32, GUIWindowWidth, 700)                                               \
	SETTING(i32, GUIWindowHeight, 720)                                              \
                                                                                    \
	SETTING(bool, GUIShowDemoWindow, false)                                         \
	SETTING(bool, GUIShowAnotherWindow, false)                                      \
                                                                                    \
	SETTING(std::string, StatsPath, "./stats.json")                                 \
	SETTING(std::string, SettingsPath, "./settings.json")                           \
	SETTING(std::string, DefaultSettingsPath, "./settings.json")                    \
	SETTING(std::string, LatoRegularFontFilePath, "./assets/font/Lato-Regular.ttf") \
	SETTING(f32, TTFFontSize, 20.f)

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
	constexpr auto Get() -> auto &;

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
constexpr auto Settings::Get() -> auto & {
	if constexpr (S == Setting::GUIRenderFilenameInBackground) {
		return mGUIRenderFilenameInBackground;
	} else if constexpr (S == Setting::GUIRenderFilenameInBackgroundOutline) {
		return mGUIRenderFilenameInBackgroundOutline;
	} else if constexpr (S == Setting::WallpaperIntervalInSeconds) {
		return mWallpaperIntervalInSeconds;
	} else if constexpr (S == Setting::StatsPath) {
		return mStatsPath;
	} else if constexpr (S == Setting::SeperateWallpapersForEachMonitor) {
		return mSeperateWallpapersForEachMonitor;
	} else if constexpr (S == Setting::GUIWindowWidth) {
		return mGUIWindowWidth;
	} else if constexpr (S == Setting::GUIWindowHeight) {
		return mGUIWindowHeight;
	} else if constexpr (S == Setting::GUIShowAnotherWindow) {
		return mGUIShowAnotherWindow;
	} else if constexpr (S == Setting::GUIShowDemoWindow) {
		return mGUIShowDemoWindow;
	} else if constexpr (S == Setting::SettingsPath) {
		return mSettingsPath;
	} else if constexpr (S == Setting::DefaultSettingsPath) {
		return mDefaultSettingsPath;
	} else if constexpr (S == Setting::LatoRegularFontFilePath) {
		return mLatoRegularFontFilePath;
	} else if constexpr (S == Setting::TTFFontSize) {
		return mTTFFontSize;
	} else {
		static_assert(!std::is_same_v<void, void>, "Unbekanntes Setting");
	}
}

template <Setting S>
constexpr auto Settings::Set(const auto &value) -> void {
	bool changed = false;

	if constexpr (S == Setting::GUIRenderFilenameInBackground) {
		changed = true;
		mGUIRenderFilenameInBackground = value;
	} else if constexpr (S == Setting::GUIRenderFilenameInBackgroundOutline) {
		changed = true;
		mGUIRenderFilenameInBackgroundOutline = value;
	} else if constexpr (S == Setting::WallpaperIntervalInSeconds) {
		changed = true;
		mWallpaperIntervalInSeconds = value;
	} else if constexpr (S == Setting::StatsPath) {
		changed = true;
		mStatsPath = value;
	} else if constexpr (S == Setting::SettingsPath) {
		changed = true;
		mSettingsPath = value;
	} else if constexpr (S == Setting::GUIWindowWidth) {
		changed = true;
		mGUIWindowWidth = value;
	} else if constexpr (S == Setting::GUIWindowHeight) {
		changed = true;
		mGUIWindowHeight = value;
	} else if constexpr (S == Setting::GUIShowAnotherWindow) {
		changed = true;
		mGUIShowAnotherWindow = value;
	} else if constexpr (S == Setting::GUIShowDemoWindow) {
		changed = true;
		mGUIShowDemoWindow = value;
	} else if constexpr (S == Setting::DefaultSettingsPath) {
		// Nicht änderbar
	} else if constexpr (S == Setting::LatoRegularFontFilePath) {
		changed = true;
		mLatoRegularFontFilePath = value;
	} else if constexpr (S == Setting::TTFFontSize) {
		changed = true;
		mTTFFontSize = value;
	} else if constexpr (S == Setting::SeperateWallpapersForEachMonitor) {
		changed = true;
		mSeperateWallpapersForEachMonitor = value;
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
constexpr auto GetSetting() -> auto & {
	return ::S().Get<S>();
}

template <Setting S>
constexpr void SetSetting(const auto &value) {
	::S().Set<S>(value);
}