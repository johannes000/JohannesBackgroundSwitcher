#include <fstream>

#include "App.hpp"
#include "GUI.hpp"
#include <cereal/archives/binary.hpp>
#include <windows.h>

auto main(int, char **) -> i32 {
	try {
		bool isRunning = true;

		FreeImage_Initialise();

		auto app = std::make_unique<App>();
		app->Init();
		if (std::ifstream is("data.knaub", std::ios::binary); is.good()) {
			cereal::BinaryInputArchive archive(is);
			archive(*app);
		}

		auto gui = std::make_unique<GUI>();
		gui->Init(app.get());

		while (isRunning) {
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
			gui->Render();
		}

		{
			std::ofstream os("data.knaub", std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
			archive(*app);
		}

		gui->Shutdown();
		app->Shutdown();

		FreeImage_DeInitialise();

		return 0;
	} catch (const std::exception &e) {
		spdlog::critical("Fehler: {}", e.what());
		return 1;
	} catch (...) {
		spdlog::critical("Unbekannter Fehler");
		return 1;
	}
}
