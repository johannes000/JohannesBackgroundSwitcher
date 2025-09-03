#pragma once

#include "utility/BS_thread_pool.hpp" // IWYU pragma: keep
#include "utility/Includes.hpp"		  // IWYU pragma: keep

#include <cereal/archives/binary.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>

#include <chrono>
#include <random>
#include <unordered_set>

namespace cereal {
template <class Archive>
void serialize(Archive &archive, fs::path &path) {
	std::string path_str = path.string();
	archive(path_str);
	path = fs::path(path_str);
}
} // namespace cereal

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

class App {
public:
	App() {};
	~App() = default;
	App(App &&) = delete;
	App(const App &) = delete;
	App &operator=(App &&) = delete;
	App &operator=(const App &) = delete;

	auto Init() -> void;
	auto Shutdown() -> void;

	auto Update() -> void;
	auto HandleEvents() -> void;

	auto IsRunning() -> bool { return mRunning; }

	auto AddWallpaperFolder(const fs::path path, bool selected = true) -> void;
	auto RemoveWallpaperFolder(const fs::path path) -> void;
	auto GetWallpaperFolders() const -> const auto & { return mWallpaperFolders; };
	auto GetWallpaperFolders() -> auto & { return mWallpaperFolders; };

	auto GetPathCount() const -> const auto & { return mPathUseCount; };
	auto GetCurrentWallpaperPath() const -> const fs::path & { return mCurrentWallpaper; };

	auto SetWallpaper(const fs::path &wallpaperPath) -> void;
	auto GetRandomWallpaper() -> fs::path;
	auto GetRandomWallpaperAsync() -> void;
	auto SetRandomWallpaper() -> void;
	auto SetRandomWallpaperAsync() -> void;

	auto SetWallpaperIntervalInMinutes(u32 minutes) -> void { mWallpaperInterval = std::chrono::minutes(minutes); };
	auto SetWallpaperIntervalInSeconds(u32 seconds) -> void { mWallpaperInterval = std::chrono::minutes(seconds / 60); };
	auto GetWallpaperInterval() const -> const auto & { return mWallpaperInterval; };
	auto GetWallpaperInterval() -> auto & { return mWallpaperInterval; };
	auto GetRemainingWallpaperIntervalTimeInS() const -> i32;

public:
	template <class Archive>
	auto serialize(Archive &archive) -> void {
		archive(
			mWallpaperFolders,
			mPathUseCount,
			// mWallpaperBlacklist,
			mFolderBlacklist,
			mCurrentWallpaper,
			mWallpaperInterval,
			mLastChange);
	}

private:
	auto HandleCounts(const fs::path &path) -> void;

	auto SelectWeightedEntry(const fs::path &path) -> fs::path;
	auto RecursiveSelectEntry(const fs::path &current) -> fs::path;

	auto GenerateTextBitmap(const std::string text) -> void;

	auto FindFolderByPath(const std::vector<WallpaperFolder> &folder, fs::path path) -> bool;

private:
	std::mt19937 mGen;

	std::vector<WallpaperFolder> mWallpaperFolders;
	std::unordered_map<fs::path, i32> mPathUseCount;
	std::unordered_set<fs::path> mWallpaperBlacklist;
	std::unordered_set<fs::path> mFolderBlacklist;

	fs::path mCurrentWallpaper;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};

	std::chrono::minutes mWallpaperInterval{5};
	std::chrono::steady_clock::time_point mLastChange;

	BS::thread_pool mThreadPool;
	std::future<fs::path> mWallpaperFuture;
	std::atomic<bool> mWallpaperLoading{false};
};