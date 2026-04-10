#pragma once
#include "FreeImage.h"

#include "utility\Includes.hpp"

#include <future>
#include <span>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3\SDL.h>
#include <SDL3_ttf\SDL_ttf.h>
#include <stdio.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

struct MonitorState;
class App;

struct TextureData {
	GLuint texture{0};
	i32 width{0};
	i32 height{0};
};

class GUI {
public:
	GUI() = default;
	~GUI() = default;
	GUI(GUI &&) = delete;
	GUI(const GUI &) = delete;
	auto operator=(GUI &&) -> GUI & = delete;
	auto operator=(const GUI &) -> GUI & = delete;

	auto Init(App *app) -> void;
	auto Shutdown() -> void;
	auto HandleEvents() -> void;
	auto Update() -> void;
	auto SlowUpdate() -> void;
	auto Render() -> void;

	auto RenderTextInBitmap(FIBITMAP *bitmap, const std::string &text) -> void;

	auto GetWindow() -> SDL_Window * { return mWindow; }
	auto GetGLContext() -> SDL_GLContext * { return &mGLContext; }

private:
	auto InitRenderer() -> void;
	static auto NewFrame() -> void;
	auto EndFrame() -> void;

	static auto RenderDemoWindows() -> void;
	void extracted(std::vector<MonitorState> &monitors, i32 &sActiveMon);
	auto RenderMainWindow() -> void;
	auto RenderMonitorTabs(std::vector<MonitorState> &monitors, i32 &sActiveMon) -> void;
	auto RenderButtons() -> void;
	auto RenderWallpaperInfo(u32 monitorID) -> void;
	auto RenderIntervalDropdown(f32 dropdownWidth) -> void;

	auto HandleFileDialogResult() -> void;

	static auto InitImguiStyle() -> void;

	auto UpdateThumbnailTextures() -> void;
	auto LoadFreeImageAsTexture(const fs::path &imagePath) -> GLuint;

	SDL_Window *mWindow{};
	SDL_GLContext mGLContext{};
	ImGuiIO mImGuiIO;
	std::vector<TextureData> mWallpaperTextureData;

	TTF_Font *mFont{};
	TTF_Font *mFontOutline{};

	App *mApp{};

	u32 mCurrentMonitorCount{1};

	LogPtr log = GetLogger("GUI");

	std::future<std::vector<fs::path>> mFoldersFuture;
	std::atomic<bool> mIsFileDialogOpen{false};
};
