#pragma once

#include "BS_thread_pool.hpp"	// IWYU pragma: keep
#include "utility/Includes.hpp" // IWYU pragma: keep

#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>

#include <chrono>
#include <random>
#include <unordered_set>

constexpr i32 MAX_MONITOR_COUNT = 10;

namespace cereal {
template <class Archive>
void serialize(Archive &archive, fs::path &path) {
	std::string path_str = path.string();
	archive(path_str);
	path = fs::path(path_str);
}
} // namespace cereal

struct MonitorState {
	fs::path currentWallpaper;
	std::chrono::steady_clock::time_point lastChange;
	bool needsTextureUpdate{false};

	template <class Archive>
	void serialize(Archive &archive) {
		archive(
			currentWallpaper,
			lastChange);
	}
};

struct WallpaperFolder {
	fs::path path;
	bool selected{true};

	template <class Archive>
	auto serialize(Archive &archive) -> void {
		archive(
			path,
			selected);
	};
};

class GUI;

class App {
public:
	App() = default;
	~App() = default;
	App(App &&) = delete;
	App(const App &) = delete;
	auto operator=(App &&) -> App & = delete;
	auto operator=(const App &) -> App & = delete;

	auto Init(GUI *gui) -> void;
	auto Shutdown() -> void;

	auto Update() -> void;
	auto SlowUpdate() -> void;
	auto HandleEvents() -> void;

	[[nodiscard]] auto IsRunning() const -> bool { return mRunning; }

	auto AddWallpaperFolder(const fs::path &path, bool selected = true) -> void;
	auto RemoveWallpaperFolder(fs::path path) -> void;
	[[nodiscard]] auto GetWallpaperFolders() const -> const auto & { return mWallpaperFolders; };
	auto GetWallpaperFolders() -> auto & { return mWallpaperFolders; };

	[[nodiscard]] auto GetPathCount() const -> const auto & { return mPathUseCount; };

	auto GetRandomWallpaper() -> fs::path;
	auto SetRandomWallpaper(u32 monitorID) -> void;
	auto SetRandomWallpaperForAllMonitors() -> void;
	auto SetWallpaper(const fs::path &wallpaperPath, u32 monitorID = 0) -> void;
	auto SetWallpaperForAllMonitors(const fs::path &wallpaperPath) -> void;

	auto GetMonitorStates() -> std::vector<MonitorState> & { return mMonitorStates; }

	auto GetRemainingWallpaperIntervalTimeInS() const -> i32;

	template <class Archive>
	auto serialize(Archive &archive) -> void {
		archive(
			mWallpaperFolders,
			mPathUseCount,
			// mWallpaperBlacklist,
			mFolderBlacklist,
			mMonitorStates);
	}

private:
	auto HandleCounts(const fs::path &path) -> void;

	auto SelectWeightedEntry(const fs::path &path) -> fs::path;
	auto RecursiveSelectEntry(const fs::path &current) -> fs::path;

	static auto FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path &path) -> bool;

	GUI *mGui{};

	std::mt19937 mGen;

	std::vector<WallpaperFolder> mWallpaperFolders;
	std::unordered_map<fs::path, i32> mPathUseCount;
	std::unordered_set<fs::path> mWallpaperBlacklist;
	std::unordered_set<fs::path> mFolderBlacklist;

	LogPtr log = GetLogger("APP");
	bool mRunning{true};

	std::vector<MonitorState> mMonitorStates;
};