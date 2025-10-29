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
	desktopWallpaper->SetPosition(DWPOS_FIT);

	if (monitorNr == 9) {
		return SystemParametersInfo(
			SPI_SETDESKWALLPAPER,
			0,
			(void *)absolutePath.string().c_str(),
			SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	} else {
		// TODO: Testen wie lange das hier dauert und evtl. die pointer speichern.
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
	UINT realCount = 0;
	if (desktopWallpaper) {
		auto hr = desktopWallpaper->GetMonitorDevicePathCount(&monitorCount);
		for (u32 i = 0; i < monitorCount; i++) {
			if (GetMonitorResolution(i).height > 0)
				realCount++;
		}

		if (FAILED(hr)) {
			log->error("Fehler");
			realCount = 1;
		}
	}

	return realCount == 0 ? 1 : realCount;
}

auto GetMonitorResolution(u32 monitorNr) -> MonitorResolution {
	MonitorResolution resolution = {};
	if (!desktopWallpaper)
		return resolution;

	wchar_t *monitorId = nullptr;
	HRESULT hr = desktopWallpaper->GetMonitorDevicePathAt(monitorNr, &monitorId);

	if (SUCCEEDED(hr)) {
		RECT rect{};

		hr = desktopWallpaper->GetMonitorRECT(monitorId, &rect);
		if (SUCCEEDED(hr)) {
			resolution.height = rect.bottom - rect.top;
			resolution.width = rect.right - rect.left;
		}
		CoTaskMemFree(monitorId);
	}
	return resolution;
}

auto SetWallpaperPosition(i32 pos) -> void{
	// Fit,
	// Fill,
	// Strech,
	// Tile,
	// Center,
	// Span
	DESKTOP_WALLPAPER_POSITION winPosition;
	switch (pos) {
		case 0: {
			winPosition = DWPOS_FIT;
		} break;
		case 1: {
			winPosition = DWPOS_FILL;
		} break;
		case 2: {
			winPosition = DWPOS_STRETCH;
		} break;
		case 3: {
			winPosition = DWPOS_TILE;
		} break;
		case 4: {
			winPosition = DWPOS_CENTER;
		} break;
		case 5: {
			winPosition = DWPOS_SPAN;
		} break;
		default: {
			winPosition = DWPOS_FIT;
		}
	}
	if (desktopWallpaper)
		desktopWallpaper->SetPosition(winPosition);
}

} // namespace Platform
