#include <Windows.h>
// NOTE: Nur ab Win 8
#include "Platform.hpp"
#include <shobjidl.h>

namespace {
IDesktopWallpaper *desktopWallpaper = nullptr;

} // namespace
namespace Platform {
LogPtr log = GetLogger("WIN");

auto Init() -> void {
	HRESULT coInit = CoInitialize(NULL);
	if (FAILED(coInit) && coInit != RPC_E_CHANGED_MODE) {
		std::cerr << "COM Initialisierung fehlgeschlagen!" << std::endl;
	}

	HRESULT hr = CoCreateInstance(
		__uuidof(DesktopWallpaper),
		NULL,
		CLSCTX_ALL,
		__uuidof(IDesktopWallpaper),
		reinterpret_cast<void **>(&desktopWallpaper));

	if (!SUCCEEDED(hr)) {
		log->error("Fehler beim DesktopWallpaper COM Init: HRESULT = 0x{:08X}", hr);
	}
}

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

// auto ChangeWallpaperAlt(std::filesystem::path wallpaperPath) -> bool {
// 	fs::path absolutePath = fs::absolute(wallpaperPath);

// 	return SystemParametersInfo(
// 		SPI_SETDESKWALLPAPER,
// 		0,
// 		(void *)absolutePath.string().c_str(),
// 		SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
// }

auto ChangeWallpaper(std::filesystem::path wallpaperPath, u32 monitorNr) -> bool {
	fs::path absolutePath = fs::absolute(wallpaperPath);

	if (monitorNr == 9) {
		return SystemParametersInfo(
			SPI_SETDESKWALLPAPER,
			0,
			(void *)absolutePath.string().c_str(),
			SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	} else {
		wchar_t *monitorID = nullptr;
		auto hr = desktopWallpaper->GetMonitorDevicePathAt(monitorNr, &monitorID);
		if (SUCCEEDED(hr)) {
			hr = desktopWallpaper->SetWallpaper(monitorID, absolutePath.c_str());
			CoTaskMemFree(monitorID);
			return (SUCCEEDED(hr));
		}
	}
	return false;
}

auto GetMonitorCount() -> u32 {
	UINT monitorCount = 1;
	if (desktopWallpaper) {
		auto hr = desktopWallpaper->GetMonitorDevicePathCount(&monitorCount);
		if (FAILED(hr)) {
			log->error("Fehler");
			monitorCount = 1;
		}
	}

	return monitorCount;
}

} // namespace Platform
