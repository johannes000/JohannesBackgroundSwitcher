

#pragma once

#include "../src/utility/Includes.hpp"

namespace Platform {
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
[[nodiscard]]
auto ChangeWallpaper(std::filesystem::path wallpaperPath, u32 monitorNr) -> bool;
auto GetMonitorCount() -> u32;
}; // namespace Platform

// namespace Platform