#pragma once
#include "FreeImage.h"

#include "utility\Includes.hpp"

#include <future>

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

class App;

struct TextureData {
	GLuint texture{0};
	i32 width{0};
	i32 height{0};
};

class GUI {
public:
	GUI() {};
	~GUI() = default;
	GUI(GUI &&) = delete;
	GUI(const GUI &) = delete;
	GUI &operator=(GUI &&) = delete;
	GUI &operator=(const GUI &) = delete;

	auto Init(App *app) -> void;
	auto Shutdown() -> void;
	auto HandleEvents() -> void;
	auto Update() -> void;
	auto Render() -> void;

	auto RenderTextInBitmap(FIBITMAP *bitmap, const std::string text) -> void;

	auto GetWindow() -> SDL_Window * { return mWindow; }
	auto GetGLContext() -> SDL_GLContext * { return &mGLContext; }

private:
	auto InitRenderer() -> void;
	auto NewFrame() -> void;
	auto EndFrame() -> void;

	auto RenderDemoWindows() -> void;
	auto RenderMainWindow() -> void;

	auto InitImguiStyle() -> void;

	auto UpdateWallpaperTexture() -> void;
	auto LoadFreeImageAsTexture(const fs::path &wallpaperPath) -> GLuint;

private:
	SDL_Window *mWindow;
	SDL_GLContext mGLContext;
	ImGuiIO mImGuiIO;
	TextureData mCurrentWallpaper;

	TTF_Font *mFont;
	TTF_Font *mFontOutline;

	App *mApp;

	LogPtr log = GetLogger("GUI");

	std::future<std::vector<fs::path>> mFoldersFuture;
	std::atomic<bool> mIsFileDialogOpen{false};
};
