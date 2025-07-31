#pragma once

#include <filesystem>

namespace Platform {
auto OpenFileDialogue() -> std::filesystem::path;
};