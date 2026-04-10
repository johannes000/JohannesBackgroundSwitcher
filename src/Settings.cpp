#include "Settings.hpp"
#include <fstream>

namespace {
Settings *Instance = nullptr;

} // namespace

Settings::Settings() {
}

auto Settings::GetInstance() -> Settings & {
	if (Instance == nullptr) {
		Instance->log->critical("Settings::Init() nicht vergessen.");
	}
	return *Instance;
}

auto Settings::Init() -> void {
	if (Instance != nullptr) {
		Instance->log->critical("Instance dürfte vorm Init nicht valid sein");
	}
	Instance = new Settings();

	Instance->log = GetLogger("SET");

	Settings::LoadFromFile();

	Instance->mLastAutosave = std::chrono::steady_clock::now();
}

auto Settings::Shutdown() -> void {
	SaveToFile();
	if (Instance != nullptr) {
		delete Instance;
		Instance = nullptr;
	}
}

auto Settings::MarkDirty() -> void {
	mDirty = true;
	mLastAutosave = std::chrono::steady_clock::now();
}

auto Settings::CheckIfDirty() -> void {
	if (mDirty) {
		return;
	}

#define SETTING(type, name, default)      \
	if (m##name != ShadowSettings.name) { \
		mDirty = true;                    \
		return;                           \
	}

	SETTINGS_DEFINITIONS
#undef SETTING
}

auto Settings::AutoSaveIfDirty() -> void {
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - Instance->mLastAutosave);

	if (elapsed.count() >= Instance->mSettingAutoSaveIntervalInSec) {
		Instance->mLastAutosave = now;
		Instance->CheckIfDirty();

		if (Instance->mDirty) {
			Instance->UpdateShadowData();
			Settings::SaveToFile();
		}
	}
}

auto Settings::UpdateShadowData() -> void {
#define SETTING(type, name, default) ShadowSettings.name = m##name;
	SETTINGS_DEFINITIONS
#undef SETTING
}

Settings::~Settings() {
}

auto Settings::SaveToFile(const fs::path &path) -> void {
	try {
		auto absolutePath = fs::absolute(path);
		auto parentPath = absolutePath.parent_path();
		if (!parentPath.empty() && !fs::exists(parentPath)) {
			fs::create_directories(parentPath);
			Instance->log->info("Verzeichniss erstellt {}", parentPath.string());
		}

		std::ofstream file(absolutePath);
		if (!file.is_open()) {
			Instance->log->error("Konnte die Datei nicht öffnen {}", absolutePath.string());
			return;
		}
		cereal::JSONOutputArchive archive(file);
		archive(cereal::make_nvp("Settings", *Instance));
		Instance->mDirty = false;
		Instance->log->info("Save Settings in {}", absolutePath.string());

	} catch (const std::exception &e) {
		Instance->log->error("Felher beim Speichern {}", e.what());
	}
};

auto Settings::LoadFromFile(const fs::path &path) -> void {
	try {
		auto absolutePath = fs::absolute(path);
		if (!fs::exists(absolutePath)) {
			Instance->log->warn("Settings-Datei existiert nicht: {}, verwende Default Werte", absolutePath.string());

			Instance->UpdateShadowData();

			Settings::SaveToFile();
			return;
		}

		std::ifstream file(absolutePath);
		if (!file.is_open()) {
			Instance->log->error("Konnte die Datei nicht öffnen {}", absolutePath.string());
			return;
		}

		cereal::JSONInputArchive archive(file);
		archive(cereal::make_nvp("Settings", *Instance));

		Instance->UpdateShadowData();

		Instance->mDirty = false;
		Instance->log->info("Settings geladen aus {}", absolutePath.string());

	} catch (const std::exception &e) {
		Instance->log->error("Felher beim Laden {}", e.what());
		Instance->log->warn("Verwende Default Werte");

		Settings::SaveToFile();
	}
};

auto Settings::ChangeToDefault() -> void {
#define SETTING(type, name, default) Instance->m##name = default;
	SETTINGS_DEFINITIONS
#undef SETTING
	Instance->UpdateShadowData();
}