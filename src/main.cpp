#include <fstream>

#include "App.hpp"
#include <cereal/archives/binary.hpp>
#include <windows.h>

auto main(int, char **) -> i32 {
	try {

		App app;

		app.Init();
		if (std::ifstream is("data.knaub", std::ios::binary); is.good()) {
			cereal::BinaryInputArchive archive(is);
			archive(app);
		}

		app.Run();

		{
			std::ofstream os("data.knaub", std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
			archive(app);
		}

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
