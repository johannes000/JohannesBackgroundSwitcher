#include "App.hpp"

auto main(int, char **) -> i32 {
	try {
		// Anwendung initialisieren und starten
		App app;
		app.Init();
		app.Run();
		app.Shutdown();

		return EXIT_SUCCESS;
	} catch (const std::exception &e) {
		spdlog::critical("Fehler: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		spdlog::critical("Unbekannter Fehler");

		return EXIT_FAILURE;
	}
}
