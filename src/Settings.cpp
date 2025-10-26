#include "Settings.hpp"

namespace {
Settings *Instance = nullptr;

} // namespace

Settings::Settings() {
}

auto Settings::GetInstance() -> Settings & {
	return *Instance;
}

auto Settings::Init() -> void {
	Instance->log = AddLogger("SET");

	if (Instance)
		Instance->log->critical("Instance dürfte vorm Init nicht valid sein");
	Instance = new Settings();
}

auto Settings::Shutdown() -> void {
	delete (Instance);
}

Settings::~Settings() {
}