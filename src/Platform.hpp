

#pragma once

#include "utility/Includes.hpp"

namespace Platform {
enum class HotkeyReaction {
	NONE,
	NEXT_WALLPAPER,
	PREV_WALLPAPER,
	BLACKLIST_WALLPAPER
};

auto OpenFileDialogue() -> fs::path;
auto RegisterWallpaperChangeHotkey() -> void;
auto UnregisterWallpaperChangeHotkey() -> void;
[[nodiscard]]
auto CheckForWallpaperChangeHotkey() -> HotkeyReaction;
[[nodiscard]]
auto ChangeWallpaper(std::filesystem::path wallpaperPath) -> bool;
}; // namespace Platform