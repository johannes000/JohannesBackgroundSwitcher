
#include "App.hpp"
#include "GUI.hpp"
#include "Platform.hpp"
#include "Settings.hpp"
#include <SDL3/SDL.h>

namespace {
i32 maxSelectionCount = 10;

constexpr f64 PI = 3.14159265358979323846;

namespace Settings2 {
bool RenderFilename = true;
}
} // namespace

auto GetWeight(i32 count) -> f64 {
	f64 norm = static_cast<f64>(count % maxSelectionCount) / maxSelectionCount;
	f64 cos = std::cos(2.f * PI * norm);
	return 0.1f + 0.9f * (cos + 1.f) / 2.f;
}

auto App::HandleEvents() -> void {
	// Windows-only Events

	// SDL Events
}

auto App::HandleCounts(const fs::path &path) -> void {
	++mPathUseCount[path];
}

auto App::GetRemainingWallpaperIntervalTimeInS() const -> i32 {
	return GetSetting<Setting::WallpaperIntervalInSeconds>() -
		   std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - mLastChange).count();
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

auto App::GenerateTextBitmap(const std::string /* text */) -> void {
	// Windows only
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
	if (mWallpaperLoading && mWallpaperFuture.valid()) {
		if (mWallpaperFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			try {
				fs::path wallpaper = mWallpaperFuture.get();
				if (!wallpaper.empty())
					SetWallpaper(wallpaper);
			} catch (const std::exception &e) {
				log->error("Fehler beim Async Wallpaper wechsel: {}", e.what());
			}
			mWallpaperLoading = false;
		}
	}

	auto now = std::chrono::steady_clock::now();
	auto elapsed = now - mLastChange;
	std::chrono::seconds chronoInterval{GetSetting<Setting::WallpaperIntervalInSeconds>()};

	if (elapsed > chronoInterval) {
		SetRandomWallpaperAsync();
		mLastChange = now;
	}
}

auto App::Shutdown() -> void {
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

auto App::SetWallpaper(const fs::path &wallpaperPath, u32 monitorNr) -> void {
	if (!fs::exists(wallpaperPath) ||
		fs::is_directory(wallpaperPath) ||
		!Util::IsImageFile(wallpaperPath)) {
		log->info("Kein Legitimes Wallpaper. {} {}", __FUNCTION__, wallpaperPath.string());
		return;
	}

	FIBITMAP *bitmap = nullptr;
	FIBITMAP *tempBitmap = nullptr;

	auto now = std::chrono::steady_clock::now();

	log->trace("Setze {} als Wallpaper", wallpaperPath.string());

	mWallpaperBlacklist.emplace(wallpaperPath);
	fs::path outputDir = "./CurrentWallpapers";

	if (!fs::exists(outputDir)) {
		fs::create_directory(outputDir);
	}

	bitmap = Util::LoadImage(wallpaperPath);
	if (!bitmap) {
		log->error("Bild konnte nicht geladen werden");
		return;
	}

	tempBitmap = FreeImage_ConvertTo32Bits(bitmap);
	FreeImage_Unload(bitmap);
	bitmap = tempBitmap;
	tempBitmap = nullptr;

	if (!bitmap) {
		log->error("Konvertierung zu 32Bit fehlgeschlagen");
		return;
	}

	if (FreeImage_GetWidth(bitmap) != 1920) {
		FIBITMAP *rescaledBitmap = FreeImage_Rescale(bitmap, 1920, 1080);
		FreeImage_Unload(bitmap);
		bitmap = rescaledBitmap;
	}

	if (!bitmap) {
		bitmap = Util::LoadImage(outputDir / "default.bmp");
		if (!bitmap) {
			log->error("Default-Bild konnte nicht geladen werden");
			return;
		}
	}

	if (GetSetting<Setting::GUIRenderFilenameInBackground>()) {
		mGui->RenderTextInBitmap(bitmap, wallpaperPath.filename().string());
	}

	log->info("bitmap size: {}", bitmap->data);
	fs::path temp = outputDir / (std::to_string(monitorNr + 1) + ".bmp");
	FreeImage_Save(FIF_BMP, bitmap, temp.string().c_str());
	FreeImage_Unload(bitmap);

	if (Platform::ChangeWallpaper(temp, monitorNr)) {
		mCurrentWallpaper = wallpaperPath;
		mLastChange = now;
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

auto App::GetRandomWallpaperAsync() -> void {
	if (mWallpaperLoading) {
		return;
	}

	mWallpaperLoading = true;
	mWallpaperFuture = mThreadPool.submit([this]() -> fs::path {
		try {
			return GetRandomWallpaper();
		} catch (const std::exception &e) {
			log->error("Fehler in {}: {}", __FUNCTION__, e.what());
			return "";
		}
	});
}

auto App::SetRandomWallpaper() -> void {
	auto monCount = Platform::GetMonitorCount();
	if (GetSetting<Setting::SeperateWallpapersForEachMonitor>() && monCount > 1) {
		for (u32 i = 0; i < monCount; i++) {
			SetWallpaper(GetRandomWallpaper(), i);
		}
	} else {
		SetWallpaper(GetRandomWallpaper());
	}
}

auto App::SetRandomWallpaperAsync() -> void {
	if (mWallpaperLoading) {
		return;
	}
	mWallpaperLoading = true;
	mWallpaperFuture = mThreadPool.submit([this]() -> fs::path {
		try {
			auto result = GetRandomWallpaper();
			return result;
		} catch (const std::exception &e) {
			log->error("Fehler in {}: {}", __FUNCTION__, e.what());
			return "";
		}
	});
}

auto App::Init(GUI *gui) -> void {
	mRunning = true;
	mGen = std::mt19937(std::random_device{}());
	mGui = gui;
}
