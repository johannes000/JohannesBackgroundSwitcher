#include "App.hpp"
#include <windows.h>

auto main(int, char **) -> i32 {
	try {
		App app;
		app.Init();
		app.Deserialize();
		app.Run();
		app.Serialize();
		app.Shutdown();

		return 0;
	} catch (const std::exception &e) {
		spdlog::critical("Fehler: {}", e.what());
		return 1;
	} catch (...) {
		spdlog::critical("Unbekannter Fehler");
		return 1;
	}
}
