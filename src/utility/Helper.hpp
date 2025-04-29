#pragma once

#include "Includes.hpp"
#include "FreeImage.h"

#include <unordered_set>
#include <filesystem>

namespace fs = std::filesystem;
namespace Util {
constexpr inline auto IsImageFile(const fs::path &path) -> bool {
	const std::unordered_set<std::string> extensions = {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".tga", ".gif"};
	return extensions.contains(path.extension().string());
}

constexpr inline auto LoadImage(std::filesystem::path path) -> FIBITMAP * {
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.string().c_str(), 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(path.string().c_str());
	if (fif == FIF_UNKNOWN)
		return nullptr;
	return FreeImage_Load(fif, path.string().c_str(), 0);
}
} // namespace Util