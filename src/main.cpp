#include <fstream>

#include <cereal/archives/binary.hpp>

#include "App.hpp"
#include "GUI.hpp"
#include "Platform.hpp"
#include "Settings.hpp"

auto main(int /* arg */, char ** /* args */) -> i32 {
	try {
		bool isRunning = true;
		auto log = GetLogger("MAIN");

		Platform::Init();
		Platform::RegisterWallpaperChangeHotkey();

		FreeImage_Initialise();

		Settings::Init();
		Setting<Setting::WallpaperPositionStyle>() = WallpaperPosition::Fit;
		Platform::SetWallpaperPosition((i32)Setting<WallpaperPositionStyle>());

		auto app = std::make_unique<App>();
		auto gui = std::make_unique<GUI>();

		std::ifstream is("data.json");
		if (is.good()) {
			cereal::JSONInputArchive archive(is);
			archive(*app);
		}
		app->Init(gui.get());
		gui->Init(app.get());

		while (isRunning) {
			// Windows muss die Hotkeys zuerst checken sonst frisst SDL die Events.
			if (Platform::CheckForWallpaperChangeHotkey() == Platform::HotkeyReaction::NEXT_WALLPAPER) {
				app->SetRandomWallpaperForAllMonitors();
			}

			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL3_ProcessEvent(&event);
				if (event.type == SDL_EVENT_QUIT) {
					isRunning = false;
				}
				if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(gui->GetWindow())) {
					isRunning = false;
				}
			}

			app->HandleEvents();
			app->Update();

			static u64 lastSlowUpdate = 0;
			u64 currentTime = SDL_GetTicks();
			if (currentTime - lastSlowUpdate >= 1000) {
				app->SlowUpdate();
				Settings::AutoSaveIfDirty();

				lastSlowUpdate = currentTime;
			}
			u32 flags = SDL_GetWindowFlags(gui->GetWindow());
			bool hasFocus = (flags & SDL_WINDOW_INPUT_FOCUS) != 0u;
			bool isMinimized = (flags & SDL_WINDOW_MINIMIZED);

			if (isMinimized) {
				SDL_Delay(200);
			} else if (hasFocus) {
				gui->Render();
				SDL_Delay(5);
			} else {
				gui->Render();
				// 30 FPS
				SDL_Delay(32);
			}
		}

		{
			// Settings serialisieren
			std::ofstream os("data.json");
			cereal::JSONOutputArchive archive(os);
			archive(*app);
		}

		gui->Shutdown();
		app->Shutdown();

		FreeImage_DeInitialise();

		Platform::UnregisterWallpaperChangeHotkey();
		Settings::Shutdown();

		return 0;
	} catch (const std::exception &e) {
		spdlog::critical("Fehler: {}", e.what());
		return 1;
	} catch (...) {
		spdlog::critical("Unbekannter Fehler");
		return 1;
	}
}
