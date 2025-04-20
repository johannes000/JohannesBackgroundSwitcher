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

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

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

private:
	auto HandleEvents() -> void;
	auto Update() -> void;
	auto Render() -> void;
	auto EndFrame() -> void;
	auto RenderDemoWindows() -> void;
	auto NewFrame() -> void;

private:
	SDL_Window *mWindow;
	SDL_GLContext mGLContext;
	ImGuiIO mImGuiIO;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};
};