#pragma once
#include "utility\Includes.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

class App;
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

	auto GetWindow() -> SDL_Window * { return mWindow; }
	auto GetGLContext() -> SDL_GLContext * { return &mGLContext; }

private:
	auto NewFrame() -> void;
	auto EndFrame() -> void;

	auto RenderDemoWindows() -> void;
	auto RenderFolderPanel() -> void;

	auto InitRenderer() -> void;

private:
	SDL_Window *mWindow;
	SDL_GLContext mGLContext;
	ImGuiIO mImGuiIO;

	App *mApp;

	LogPtr log = AddLogger("GUI");
};