#pragma once
#include "utility/Includes.hpp"
#include "GUI.hpp"

struct WallpaperFolder {
	fs::path path;
	bool selected{true};
};

class App {
public:
	App() {};
	~App() = default;
	App(App &&) = delete;
	App(const App &) = delete;
	App &operator=(App &&) = delete;
	App &operator=(const App &) = delete;

	auto Init() -> void;
	auto Run() -> void;
	auto Shutdown() -> void;

	auto AddWallpaperFolder(const fs::path path) -> void;
	auto RemoveWallpaperFolder(const fs::path path) -> void;
	auto GetWallpaperFolders() const -> const std::vector<WallpaperFolder> & { return mWallpaperFolders; };

	auto GetRandomWallpaper() const -> fs::path;
	auto GetRandomWallpaper(fs::path path) const -> fs::path;

private:
	auto Update() -> void;
	auto HandleEvents() -> void;

	auto FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path path) -> bool;

private:
	GUI mGUI;

	std::vector<WallpaperFolder> mWallpaperFolders;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};
};