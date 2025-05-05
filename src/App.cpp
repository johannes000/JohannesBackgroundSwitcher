// TODO: Platform Stuff
#include <Windows.h>
#include "App.hpp"
#include "utility/Serializer.hpp"
#include "nlohmann/json.hpp"

namespace {
i32 maxSelectionCount = 10;

constexpr f64 PI = 3.14159265358979323846;

constexpr const char *statsPath = "./stats.json";
constexpr const char *settingsPath = "./settings.json";
constexpr const char *defaultSettingsPath = "./default_settings.json";
} // namespace

auto GetWeight(i32 count) -> f64 {
	f64 norm = static_cast<f64>(count % maxSelectionCount) / maxSelectionCount;
	f64 cos = std::cos(2.f * PI * norm);
	return 0.1f + 0.9f * (cos + 1.f) / 2.f;
}

auto App::HandleEvents() -> void {
	// Windows-only Events
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_HOTKEY &&
			msg.wParam == GLOBAL_WINDOWS_SWITCH_HOTKEY_ID) {
			SetWallpaper(GetRandomWallpaper());
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// SDL Events
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		if (event.type == SDL_EVENT_QUIT)
			mRunning = false;
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(mGUI.GetWindow()))
			mRunning = false;
	}
}

auto App::HandleCounts(const fs::path &path) -> void {
	++mPathUseCount[path];
}

auto App::Serialize() -> void {
	SerializeSettings();
	SerializeStats();
}

auto App::SerializeStats() -> void {
	using json = nlohmann::json;
	json data;

	data["Wallpaper Folders"] = json::array();
	for (const auto &[path, selected] : mWallpaperFolders) {
		data["Wallpaper Folders"].push_back(
			{{"path", path},
			 {"selected", selected}});
	}

	data["Path Count"] = json::object();
	for (const auto &[path, count] : mPathUseCount) {
		data["Path Count"][path.string()] = count;
	}

	// data["Wallpaper Blacklist"] = json::array();
	// for (const auto &path : mWallpaperBlacklist) {
	// 	data["Wallpaper Blacklist"].push_back(path.string());
	// }

	data["Folder Blacklist"] = json::array();
	for (const auto &path : mFolderBlacklist) {
		data["Folder Blacklist"].push_back(path.string());
	}

	std::ofstream file(statsPath);
	file << data.dump(4);
}

auto App::SerializeSettings() -> void {
	using json = nlohmann::json;

	json data;
	data["Interval"] = mWallpaperInterval.count();

	std::ofstream file(settingsPath);
	file << data.dump(4);
}

auto App::Deserialize() -> void {
	DeserializeStats();
	DeserializeSettings();
}

auto App::DeserializeStats() -> void {
	if (!fs::exists(fs::path(statsPath)))
		return;

	try {
		std::ifstream file(statsPath);

		using json = nlohmann::json;
		json data = json::parse(file);

		if (data.contains("Wallpaper Folders")) {
			for (const auto &folder : data["Wallpaper Folders"]) {
				fs::path path = folder["path"].get<std::string>();
				bool selected = folder.value("selected", true);
				AddWallpaperFolder(path, selected);
			}
		}

		if (data.contains("Path Count")) {
			for (const auto &[path, count] : data["Path Count"].items()) {
				mPathUseCount[fs::path(path)] = count.get<i32>();
			}
		}

		if (data.contains("Wallpaper Blacklist")) {
			for (const auto &pathStr : data["Wallpaper Blacklist"]) {
				mWallpaperBlacklist.emplace(fs::path(pathStr.get<std::string>()));
			}
		}

		if (data.contains("Folder Blacklist")) {
			for (const auto &pathStr : data["Folder Blacklist"]) {
				mFolderBlacklist.emplace(fs::path(pathStr.get<std::string>()));
			}
		}
	} catch (const std::exception &e) {
		log->warn("Fehler beim Laden der Settings: {}", e.what());
		return;
	}
}

auto App::DeserializeSettings() -> void {
	using json = nlohmann::json;

	std::string path = settingsPath;
	if (!fs::exists(path))
		path = defaultSettingsPath;
	if (!fs::exists(path)) {
		log->warn("Konnte keine Settings Laden. Viel Glück!");
		return;
	}

	try {

		std::ifstream file(path);
		json data = json::parse(file);

		if (data.contains("Interval")) {
			mWallpaperInterval = std::chrono::minutes(data["Interval"].get<i32>());
		}
	} catch (const std::exception &e) {
		log->warn("Fehler beim Laden der Settings: {}", e.what());
		return;
	}
	log->trace("Settings erfolgreich geladen");
}

auto App::SelectWeightedEntry(const fs::path &path) -> fs::path {
	std::vector<fs::path> candidates;
	std::vector<f64> weights;

	for (const auto &entry : fs::directory_iterator(path)) {
		const auto &entryPath = entry.path();
		if (mWallpaperBlacklist.contains(entryPath)) {
		} else if (Util::IsImageFile(entryPath)) {
			candidates.push_back(entryPath);
			weights.push_back(1.f);
		} else if (entry.is_directory()) {
			i32 count = mPathUseCount[entryPath];
			candidates.push_back(entryPath);
			weights.push_back(GetWeight(count));
		}
	}
	if (candidates.empty())
		return {};
	if (candidates.size() == 1)
		return candidates[0];

	std::discrete_distribution<> distr(weights.begin(), weights.end());
	return candidates[distr(mGen)];
}

auto App::RecursiveSelectEntry(const fs::path &current) -> fs::path {

	if (!fs::exists(current))
		return {};
	if (Util::IsImageFile(current))
		// TODO: Blacklist
		return current;
	if (!fs::is_directory(current))
		return {};
	else
		HandleCounts(current);

	auto selected = SelectWeightedEntry(current);
	if (selected.empty()) {
		log->warn("Leerer ordner: {}", current.string());
		return {};
	}
	return RecursiveSelectEntry(selected);
}

auto App::FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path path) -> bool {
	for (const auto &wp : folder) {
		if (wp.path == path) {
			return true;
		}
	}
	return false;
}

auto App::Update() -> void {
	mGUI.Update();
}

auto App::Run() -> void {
	while (mRunning) {
		HandleEvents();
		Update();
		if (SDL_GetWindowFlags(mGUI.GetWindow()) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
		} else {
			mGUI.Render();
		}
	}
}

auto App::Shutdown() -> void {
	if (mWallpaperThreadRunning) {
		mWallpaperThreadRunning = false;
		if (mWallpaperThread.joinable()) {
			mWallpaperThread.join();
		}
	}

	mGUI.Shutdown();
	Serialize();
	FreeImage_DeInitialise();

	UnregisterHotKey(NULL, GLOBAL_WINDOWS_SWITCH_HOTKEY_ID);
}

auto App::AddWallpaperFolder(const fs::path path, bool selected) -> void {
	if (!fs::exists(path)) {
		log->warn("Pfad {} exzestiert nicht. Kann nicht hinzufügen", path.string());
		return;
	}
	if (!fs::is_directory(path)) {
		log->warn("{} ist eine Datei. Kann nicht hinzufügen", path.string());
		return;
	}
	log->trace("Adde zu den Foldern: {}", path.string());

	for (const auto &folder : mWallpaperFolders) {
		if (folder.path == path) {
			log->info("{} ist schon im System.");
			return;
		}
	}
	WallpaperFolder wf;
	wf.path = path;
	wf.selected = selected;
	mWallpaperFolders.push_back(wf);
	std::sort(mWallpaperFolders.begin(), mWallpaperFolders.end(), [](WallpaperFolder a, WallpaperFolder b) -> bool { return a.path < b.path; });
}

auto App::RemoveWallpaperFolder(const fs::path /* path */) -> void {
}

auto App::SetWallpaper(const fs::path &wallpaperPath) -> void {
	if (!fs::exists(wallpaperPath) ||
		fs::is_directory(wallpaperPath) ||
		!Util::IsImageFile(wallpaperPath))
		return;
	auto now = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - mLastChange);
	if (duration.count() < 1000)
		return;

	log->trace("Setze {} als Wallpaper", wallpaperPath.string());

	mWallpaperBlacklist.emplace(wallpaperPath);
	fs::path outputDir = "./CurrentWallpapers";

	if (!fs::exists(outputDir)) {
		fs::create_directory(outputDir);
	}

	auto bitmap = Util::LoadImage(wallpaperPath);
	if (FreeImage_GetWidth(bitmap) != 1920)
		bitmap = FreeImage_Rescale(bitmap, 1920, 1080);

	fs::path temp = outputDir / "1.bmp";
	FreeImage_Save(FIF_BMP, bitmap, temp.string().c_str());
	FreeImage_Unload(bitmap);

	fs::path absolutePath = fs::absolute(temp);
	BOOL ok = SystemParametersInfo(
		SPI_SETDESKWALLPAPER,
		0,
		(void *)absolutePath.string().c_str(),
		SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	if (ok) {
		mCurrentWallpaper = wallpaperPath;
		std::lock_guard<std::mutex> lock(mTimeMutex);
		mLastChange = std::chrono::steady_clock::now();
	}
}

auto App::GetRandomWallpaper() -> fs::path {
	if (mWallpaperFolders.empty()) {
		log->info("Keine Ordner im System");
		return "";
	}
	i32 a = 0;

	std::vector<fs::path> eligible;
	for (const auto &wf : mWallpaperFolders) {
		if (wf.selected) {
			eligible.push_back(wf.path);
		}
	}

	if (eligible.empty()) {
		log->warn("Keine Ordner Ausgewählt");
		return "";
	}

	// TODO: Weights selber aussuchen können wenn per Settings gewünscht
	std::uniform_int_distribution<size_t> firstDist(0, eligible.size() - 1);

	while (true) {
		fs::path base = eligible[firstDist(mGen)];

		fs::path result = RecursiveSelectEntry(base);
		if (!result.empty())
			return result;
	}
	return "C:/Wallpaper/1.webp";
}

auto App::SetRandomWallpaper() -> void {
	SetWallpaper(GetRandomWallpaper());
}

auto App::WallpaperChangeThread() -> void {
	using namespace std::literals;
	while (mWallpaperThreadRunning) {
		SetWallpaper(GetRandomWallpaper());
		{
			std::lock_guard<std::mutex> lock(mTimeMutex);
			mLastChange = std::chrono::steady_clock::now();
		}
		while (mWallpaperThreadRunning &&
			   std::chrono::steady_clock::now() - mLastChange < mWallpaperInterval) {
			std::this_thread::sleep_for(1s);
		}
	}
}

auto App::Init() -> void {
	mGUI.Init(this);

	FreeImage_Initialise();

	mRunning = true;

	Deserialize();

	mGen = std::mt19937(std::random_device{}());

	mWallpaperThreadRunning = true;
	mWallpaperThread = std::thread(&App::WallpaperChangeThread, this);

	if (RegisterHotKey(NULL, GLOBAL_WINDOWS_SWITCH_HOTKEY_ID, MOD_CONTROL | MOD_ALT, 0x4E)) {
		log->trace("Windows Global Hotkey Registriert");
	}
}
