

#pragma once

#include "../src/utility/Includes.hpp"

namespace Platform {
struct MonitorResolution {
	u32 width;
	u32 height;
};

enum class HotkeyReaction {
	NONE,
	NEXT_WALLPAPER,
	PREV_WALLPAPER,
	BLACKLIST_WALLPAPER
};

auto Init() -> void;
auto Shutdown() -> void;

auto OpenFileDialogue() -> fs::path;
auto RegisterWallpaperChangeHotkey() -> void;
auto UnregisterWallpaperChangeHotkey() -> void;
[[nodiscard]]
auto CheckForWallpaperChangeHotkey() -> HotkeyReaction;
auto ChangeWallpaper(const std::filesystem::path &wallpaperPath, u32 monitorNr = 0) -> bool;
auto GetMonitorCount() -> u32;
auto GetMonitorResolution(u32 monitorNr) -> MonitorResolution;
auto OpenPathInDefaultApp(const std::filesystem::path &wallpaperPath) -> void;

auto SetWallpaperPosition(i32 pos) -> void;
}; // namespace Platform

// namespace Platform