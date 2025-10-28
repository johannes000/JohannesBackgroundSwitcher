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
	SETTING(std::string, LatoRegularFontFilePath, "./assets/font/Lato-Regular.ttf") \
	SETTING(f32, TTFFontSize, 20.f)                                                 \
                                                                                    \
	SETTING(i32, SettingAutoSaveIntervalInSec, 100)

enum class Setting {
#define SETTING(type, name, default) name,
	SETTINGS_DEFINITIONS
#undef SETTING
};

class Settings {
private:
	struct ShadowData {
#define SETTING(type, name, default) type name = default;
		SETTINGS_DEFINITIONS
#undef SETTING
	} ShadowSettings;

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

	static auto AutoSaveIfDirty() -> void;

	static auto SaveToFile(fs::path path = "./settings.json") -> void;
	static auto LoadFromFile(fs::path path = "./settings.json") -> void;

	static auto ChangeToDefault() -> void;

	template <Setting S>
	constexpr auto Get() -> auto &;

	template <class Archive>
	void serialize(Archive &archive) {
#define SETTING(type, name, default) archive(cereal::make_nvp(#name, m##name));
		SETTINGS_DEFINITIONS
#undef SETTING
	}

private:
	LogPtr log;
	bool mDirty = false;
	std::chrono::steady_clock::time_point mLastAutosave;

private:
	auto MarkDirty() -> void;
	auto CheckIfDirty() -> void;
	auto UpdateShadowData() -> void;

// Member vars
#define SETTING(type, name, default) type m##name = default;
	SETTINGS_DEFINITIONS
#undef SETTING
};

#define SETTING(type, name, default) constexpr Setting name = Setting::name;
SETTINGS_DEFINITIONS
#undef SETTING

	;

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
	} else if constexpr (S == Setting::LatoRegularFontFilePath) {
		return mLatoRegularFontFilePath;
	} else if constexpr (S == Setting::TTFFontSize) {
		return mTTFFontSize;
	} else {
		static_assert(!std::is_same_v<void, void>, "Unbekanntes Setting");
	}
}

inline auto S() -> Settings & {
	return Settings::GetInstance();
}

template <Setting S>
constexpr auto Setting() -> auto & {
	return ::S().template Get<S>();
}
