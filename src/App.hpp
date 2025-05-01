#pragma once
#include "utility/Includes.hpp"
#include "GUI.hpp"

#include <random>
#include <chrono>

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
	auto GetCurrentWallpaperPath() const -> const fs::path & { return mCurrentWallpaper; };
	auto SetWallpaper(const fs::path &wallpaperPath) -> void;

	auto GetRandomWallpaper() -> fs::path;

	auto WallpaperChangeThread() -> void;

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
	std::vector<fs::path> mWallpaperBlacklist;
	std::vector<fs::path> mFolderBlacklist;

	fs::path mCurrentWallpaper;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};

	std::atomic<bool> mWallpaperThreadRunning{false};
	std::thread mWallpaperThread;
	std::chrono::minutes mWallpaperInterval{5};
};