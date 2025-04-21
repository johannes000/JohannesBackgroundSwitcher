#pragma once
#include "utility\Includes.hpp"
#include "GUI.hpp"

#include <filesystem>
namespace fs = std::filesystem;

struct WallpaperFolder {
	fs::path path;

	std::vector<WallpaperFolder> childFolders;
	std::vector<fs::path> picturePaths;

	i32 depth{0};
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

private:
	auto Update() -> void;
	auto HandleEvents() -> void;

	auto ConstructWallpaperFolder(fs::path path, WallpaperFolder *parent) -> void;
	auto FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path path) -> bool;

private:
	GUI mGUI;

	std::vector<WallpaperFolder> mWallpaperFolders;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};
};