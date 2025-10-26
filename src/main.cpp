#include <fstream>

#include <cereal/archives/binary.hpp>

#include "App.hpp"
#include "GUI.hpp"
#include "Platform.hpp"
#include "Settings.hpp"

auto main(int, char **) -> i32 {
	try {
		bool isRunning = true;
		auto log = AddLogger("MAIN");

		Platform::RegisterWallpaperChangeHotkey();

		FreeImage_Initialise();

		Settings::Init();

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
				app->SetRandomWallpaperAsync();
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

		return 0;
	} catch (const std::exception &e) {
		spdlog::critical("Fehler: {}", e.what());
		return 1;
	} catch (...) {
		spdlog::critical("Unbekannter Fehler");
		return 1;
	}
}
