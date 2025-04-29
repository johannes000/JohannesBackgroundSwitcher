#include "App.hpp"
#include "utility\MapSerializer.hpp"

#include <random>

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

auto App::GetRandomWallpaper() const -> fs::path {
	if (mWallpaperFolders.empty()) {
		log->warn("Keine Ordner im System");
		return "";
	}
	std::vector<WallpaperFolder> eligible;
	for (const auto &wf : mWallpaperFolders) {
		if (wf.selected)
			eligible.push_back(wf);
	}
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> folderDist(0, eligible.size() - 1);
	return GetRandomWallpaper(eligible.at(folderDist(gen)).path);
}

auto App::GetRandomWallpaper(fs::path path) const -> fs::path {
	std::vector<fs::path> images;
	for (const auto &pic : fs::directory_iterator(path)) {
		if (Util::IsImageFile(pic.path())) {
			images.push_back(pic.path());
		}
	}
	log->info("{}", path.filename().string());

	for (auto const &i : images) {
		log->info("{}", i.filename().string());
	}
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> picsDist(0, images.size() - 1);
	auto rr = images[picsDist(gen)];
	log->info("Bild Ausgewählt {}", rr.string());
	return rr;
}

auto App::Init() -> void {
	mGUI.Init(this);
	FreeImage_Initialise();
	mRunning = true;
}
