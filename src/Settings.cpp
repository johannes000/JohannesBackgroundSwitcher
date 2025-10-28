#include "Settings.hpp"

namespace {
Settings *Instance = nullptr;
} // namespace

Settings::Settings() {
}

auto Settings::GetInstance() -> Settings & {
	if (!Instance)
		Instance->log->critical("Settings::Init() nicht vergessen.");
	return *Instance;
}

auto Settings::Init() -> void {
	if (Instance) {
		Instance->log->critical("Instance dürfte vorm Init nicht valid sein");
	}
	Instance = new Settings();

	Instance->log = GetLogger("SET");
}

auto Settings::Shutdown() -> void {
	if (Instance) {
		delete Instance;
		Instance = nullptr;
	}
}

Settings::~Settings() {
}

auto Settings::SaveToFile(fs::path path) -> void {
	log->info("Save Settings in {}", path.string());
};

auto Settings::LoadFromFile(fs::path path) -> void {
	log->info("Lade Settings aus {}", path.string());
};