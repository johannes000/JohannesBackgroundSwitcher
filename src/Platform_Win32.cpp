#include <Windows.h>
#include <gdiplus.h>

#include "Platform.hpp"

namespace Platform {
auto log = AddLogger("WIN");

auto OpenFileDialogue() -> fs::path {
	return "C:/Wallpaper/Test";
}

auto RegisterWallpaperChangeHotkey() -> void {
	// STRG + ALT + N
	if (RegisterHotKey(NULL, GLOBAL_WINDOWS_SWITCH_HOTKEY_ID, MOD_CONTROL | MOD_ALT, 0x4E)) {
		log->trace("Windows Global Hotkey Registriert");
	}
}
auto CheckForWallpaperChangeHotkey() -> HotkeyReaction {
	HotkeyReaction reaction = HotkeyReaction::NONE;

	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_HOTKEY &&
			msg.wParam == GLOBAL_WINDOWS_SWITCH_HOTKEY_ID) {
			reaction = HotkeyReaction::NEXT_WALLPAPER;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return reaction;
}

auto UnregisterWallpaperChangeHotkey() -> void {
	UnregisterHotKey(NULL, GLOBAL_WINDOWS_SWITCH_HOTKEY_ID);
}

auto ChangeWallpaper(std::filesystem::path wallpaperPath) -> bool {
	fs::path absolutePath = fs::absolute(wallpaperPath);
	return SystemParametersInfo(
		SPI_SETDESKWALLPAPER,
		0,
		(void *)absolutePath.string().c_str(),
		SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}
} // namespace Platform