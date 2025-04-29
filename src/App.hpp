#pragma once
#include "utility/Includes.hpp"
#include "GUI.hpp"

#include <random>

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
	auto GetWallpaperFolders() const -> const auto & { return mWallpaperFolders; };
	auto GetPathCount() const -> const auto & { return mPathUseCount; };

	auto GetRandomWallpaper() -> fs::path;

private:
	auto Update() -> void;
	auto HandleEvents() -> void;
	auto HandleCounts(const fs::path &path) -> void;

	auto SelectWeightedEntry(const fs::path &path) -> fs::path;
	auto RecursiveSelectEntry(const fs::path &current) -> fs::path;

	auto FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path path) -> bool;

private:
	GUI mGUI;

	std::mt19937 mGen;

	std::vector<WallpaperFolder> mWallpaperFolders;
	std::map<fs::path, i32> mPathUseCount;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};
};