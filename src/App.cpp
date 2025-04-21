#include "App.hpp"

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

auto App::ConstructWallpaperFolder(fs::path path, WallpaperFolder *parent) -> void {
	WallpaperFolder wf;
	wf.path = path;

	for (const auto &entry : fs::directory_iterator(path)) {
		if (fs::is_directory(entry.path())) {
			ConstructWallpaperFolder(entry.path(), &wf);
		} else {
			wf.picturePaths.push_back(entry);
		}
	}

	if (!parent) {
		mWallpaperFolders.push_back(wf);
	} else {
		wf.depth = parent->depth + 1;
		parent->childFolders.push_back(wf);
	}

	for (const auto &p : wf.picturePaths)
		log->info("{}", p.string());
}

auto App::FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path path) -> bool {
	for (const auto &wp : folder) {
		if (wp.path == path) {
			return true;
		}
		return FindFolderByPath(wp.childFolders, path);
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
	if (FindFolderByPath(mWallpaperFolders, path)) {
		log->warn("{} ist schon im System. Wird nicht hinzugefügt", path.string());
		return;
	}
	ConstructWallpaperFolder(path, nullptr);
}

auto App::RemoveWallpaperFolder(const fs::path /* path */) -> void {
}

auto App::Init() -> void {
	mGUI.Init(this);
	mRunning = true;
}
