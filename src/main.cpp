#include <fstream>

#include <cereal/archives/binary.hpp>

#include "App.hpp"
#include "GUI.hpp"
#include "Platform.hpp"
#include "Settings.hpp"

auto main(int, char **) -> i32 {
	try {
		bool isRunning = true;
		auto log = GetLogger("MAIN");

		Platform::Init();
		Platform::RegisterWallpaperChangeHotkey();
		auto r1 = Platform::GetMonitorResolution(0);
		auto r2 = Platform::GetMonitorResolution(1);

		log->info("Res {}x{} {}x{} ", r1.width, r1.height, r2.width, r2.height);
		log->info("mon count {}", Platform::GetMonitorCount());

		FreeImage_Initialise();

		Settings::Init();
		Setting<Setting::WallpaperPositionStyle>() = WallpaperPosition::Fit;
		Platform::SetWallpaperPosition((i32)Setting<WallpaperPositionStyle>());

		auto app = std::make_unique<App>();
		auto gui = std::make_unique<GUI>();

		app->Init(gui.get());
		if (std::ifstream is("data.knaub", std::ios::binary); is.good()) {
			cereal::BinaryInputArchive archive(is);
			archive(*app);
		}

		gui->Init(app.get());

		while (isRunning) {

			// Windows muss die Hotkeys zuerst checken sonst frisst SDL die Events.
			if (Platform::CheckForWallpaperChangeHotkey() == Platform::HotkeyReaction::NEXT_WALLPAPER) {
				app->SetRandomWallpaper();
			}

			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				ImGui_ImplSDL3_ProcessEvent(&event);
				if (event.type == SDL_EVENT_QUIT)
					isRunning = false;
				if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(gui->GetWindow()))
					isRunning = false;
			}

			app->HandleEvents();
			app->Update();
			if (!(SDL_GetWindowFlags(gui->GetWindow()) & SDL_WINDOW_MINIMIZED)) {
				gui->Render();
			} else {
				SDL_Delay(10);
			}
			Settings::AutoSaveIfDirty();
		}

		{
			std::ofstream os("data.knaub", std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
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
