// TODO: Platform Stuff
#include <Windows.h>
#include "App.hpp"
#include "utility\Serializer.hpp"

// TODO: Platform Stuff
auto App::HandleEvents() -> void {
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
	auto &count = mPathUseCount[path];
	if (++count > 3)
		count = 0;
}

auto App::SelectWeightedEntry(const fs::path &path) -> fs::path {
	std::vector<fs::path> candidates;
	std::vector<f64> weights;

	for (const auto &entry : fs::directory_iterator(path)) {
		const auto &entryPath = entry.path();
		if (Util::IsImageFile(entryPath)) {
			candidates.push_back(entryPath);
			weights.push_back(1.f);
		} else if (entry.is_directory()) {
			i32 count = mPathUseCount.contains(entryPath) ? mPathUseCount[entryPath] : 0;
			candidates.push_back(entryPath);
			weights.push_back(std::pow(.7f, count));
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
	log->trace("Durchsuche {}", current.string());

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
	mGUI.Shutdown();
	PathMapSerializer::Serialize(mPathUseCount, USE_COUNT_FILE);
	FreeImage_DeInitialise();
}

auto App::AddWallpaperFolder(const fs::path path) -> void {
	if (!fs::exists(path)) {
		log->warn("Pfad {} exzestiert nicht. Kann nicht hinzufügen", path.string());
		return;
	}
	if (!fs::is_directory(path)) {
		log->warn("{} ist eine Datei. Kann nicht hinzufügen", path.string());
		return;
	}
	for (const auto &folder : mWallpaperFolders) {
		if (folder.path == path) {
			log->info("{} ist schon im System.");
			return;
		}
	}
	WallpaperFolder wf;
	wf.path = path;
	mWallpaperFolders.push_back(wf);
}

auto App::RemoveWallpaperFolder(const fs::path /* path */) -> void {
}

auto App::SetWallpaper(const fs::path &wallpaperPath) -> void {
	if (!fs::exists(wallpaperPath) ||
		fs::is_directory(wallpaperPath) ||
		!Util::IsImageFile(wallpaperPath))
		return;
	log->trace("Setze {} als Wallpaper", wallpaperPath.string());

	fs::path outputDir = "./CurrentWallpapers";

	// Ordner erstellen falls nicht vorhanden
	if (!fs::exists(outputDir)) {
		fs::create_directory(outputDir);
	}

	auto bitmap = Util::LoadImage(wallpaperPath);

	fs::path temp = outputDir / "1.bmp";
	FreeImage_Save(FIF_BMP, bitmap, temp.string().c_str());

	fs::path absolutePath = fs::absolute(temp);
	BOOL ok = SystemParametersInfo(
		SPI_SETDESKWALLPAPER,
		0,
		(void *)absolutePath.string().c_str(),
		SPIF_UPDATEINIFILE);
}

auto App::GetRandomWallpaper() -> fs::path {
	if (mWallpaperFolders.empty()) {
		log->warn("Keine Ordner im System");
		return "";
	}
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

	std::vector<f64> weights;
	for (const auto &path : eligible) {
		i32 count = mPathUseCount.contains(path) ? mPathUseCount[path] : 0;
		weights.push_back(std::pow(.7f, count));
	}

	std::discrete_distribution<> firstDist(weights.begin(), weights.end());

	while (true) {
		fs::path base = eligible[firstDist(mGen)];
		log->trace("Top lvl Auswahl: {}", base.string());

		fs::path result = RecursiveSelectEntry(base);
		if (!result.empty())
			return result;
	}
	return "C:/Wallpaper/1.webp";
}

auto App::Init() -> void {
	mGUI.Init(this);
	FreeImage_Initialise();
	mRunning = true;
	mPathUseCount = PathMapSerializer::Deserialize(USE_COUNT_FILE);
	mGen = std::mt19937(std::random_device{}());
}
