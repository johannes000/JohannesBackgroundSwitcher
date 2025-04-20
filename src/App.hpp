#pragma once
#include "utility\Includes.hpp"
#include "GUI.hpp"

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
	auto Update() -> void;
	auto HandleEvents() -> void;

	GUI mGUI;

	LogPtr log = AddLogger("APP");
	bool mRunning{true};
};