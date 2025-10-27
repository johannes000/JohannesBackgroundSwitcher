#include "GUI.hpp"

#include "App.hpp"
#include "Settings.hpp"
#include "nfd.hpp"

namespace {

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

constexpr f32 FOLDERPANEL_SIZE_FAKTOR = 1.f / 3.f;

// Intervals in Sec
constexpr std::array<i32, 10> intervals{1 * 60, 2 * 60, 5 * 60, 10 * 60, 30 * 60, 1 * 60 * 60, 2 * 60 * 60, 5 * 60 * 60, 10 * 60 * 60, 24 * 60};
constexpr std::array<const char *, 10> intervalStrings{"1 min", "2 min", "5 min", "10 min", "30 min", "1 hour", "2 hours", "5 hours", "10 hours", "24 hours"};

namespace LocalSettings {
i32 PrintTextPaddingVertical = 5;
i32 PrintTextPaddingHorizontal = 5;
enum struct PrintTextPositions { Top_Left,
								 Top_Right,
								 Bottom_Left,
								 Bottom_Right };
PrintTextPositions PrintTextPosition{PrintTextPositions::Top_Right};
}; // namespace LocalSettings

} // namespace

auto GUI::HandleEvents() -> void {
}

auto GUI::Update() -> void {
}

auto GUI::Render() -> void {
	NewFrame();

	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags windowFlags =
		// ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
	ImGui::Begin("Main Window", nullptr, windowFlags);
	ImGui::PopStyleVar();

	ImGuiID dockspaceId = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	RenderMainWindow();

	// RenderDemoWindows();

	ImGui::End();
	EndFrame();
}

auto GUI::RenderTextInBitmap(FIBITMAP *bitmap, const std::string text) -> void {
	SDL_Color white = {255, 255, 255, 255};
	SDL_Color black = {0, 0, 0, 0};
	SDL_Surface *textSurface = TTF_RenderText_Blended(mFont, text.c_str(), 0, white);
	SDL_Surface *textOutlineSurface = TTF_RenderText_Blended(mFontOutline, text.c_str(), 0, black);
	if (!textSurface || !textOutlineSurface) {
		log->error("Fehler beim Text-Rendering: {}", SDL_GetError());
		return;
	}
	SDL_FlipSurface(textSurface, SDL_FLIP_VERTICAL);

	if (GetSetting<Setting::GUIRenderFilenameInBackgroundOutline>())
		SDL_FlipSurface(textOutlineSurface, SDL_FLIP_VERTICAL);

	i32 width = FreeImage_GetWidth(bitmap);
	i32 height = FreeImage_GetHeight(bitmap);
	i32 freeImagePitch = FreeImage_GetPitch(bitmap);
	BYTE *pixels = FreeImage_GetBits(bitmap);

	SDL_Surface *surface = SDL_CreateSurfaceFrom(
		width, height,
		SDL_PIXELFORMAT_BGRA8888,
		pixels, freeImagePitch);
	if (!surface) {
		log->error("Fehler beim surface erstellen: {}. Pitch: {}", SDL_GetError(), freeImagePitch);
		SDL_DestroySurface(textSurface);
		SDL_DestroySurface(textOutlineSurface);
		return;
	}

	SDL_Rect outlineDest = {1, 1, textSurface->w, textSurface->h};
	SDL_SetSurfaceBlendMode(textSurface, SDL_BLENDMODE_BLEND);
	SDL_BlitSurface(textSurface, NULL, textOutlineSurface, &outlineDest);

	SDL_Rect dst = {0, surface->h, textOutlineSurface->w, textOutlineSurface->h};
	switch (LocalSettings::PrintTextPosition) {
		case LocalSettings::PrintTextPositions::Top_Left: {
			dst.x = LocalSettings::PrintTextPaddingHorizontal;
			dst.y = surface->h - textOutlineSurface->h - LocalSettings::PrintTextPaddingVertical;
		} break;
		case LocalSettings::PrintTextPositions::Top_Right: {
			dst.x = surface->w - textOutlineSurface->w - LocalSettings::PrintTextPaddingHorizontal;
			dst.y = surface->h - textOutlineSurface->h - LocalSettings::PrintTextPaddingVertical;
		} break;
		case LocalSettings::PrintTextPositions::Bottom_Left: {
			dst.x = LocalSettings::PrintTextPaddingHorizontal;
			dst.y = LocalSettings::PrintTextPaddingVertical;
		} break;
		case LocalSettings::PrintTextPositions::Bottom_Right: {
			dst.x = surface->w - textOutlineSurface->w - LocalSettings::PrintTextPaddingHorizontal;
			dst.y = LocalSettings::PrintTextPaddingVertical;
		} break;
		default: {
			dst.x = LocalSettings::PrintTextPaddingHorizontal;
			dst.y = LocalSettings::PrintTextPaddingVertical;
		} break;
	}

	SDL_BlitSurface(textOutlineSurface, NULL, surface, &dst);

	SDL_DestroySurface(surface);
	SDL_DestroySurface(textSurface);
	SDL_DestroySurface(textOutlineSurface);
}

auto RenderFolderPathText(WallpaperFolder &folder) -> void {
	ImGui::Checkbox(folder.path.string().c_str(), &folder.selected);
	// for (const auto &pic : folder.picturePaths) {
	// 	ImGui::Text("  %s%s", indent.c_str(), pic.filename().string().c_str());
	// }
}

auto GUI::RenderMainWindow() -> void {
	f32 panelWidth = ImGui::GetContentRegionAvail().x;
	f32 buttonWidth = panelWidth - ImGui::GetStyle().WindowPadding.x * 2;

	ImGui::Begin("##FolderContent",
				 nullptr,
				 ImGuiWindowFlags_HorizontalScrollbar |
					 ImGuiWindowFlags_NoDecoration);
	{
		for (auto &folder : mApp->GetWallpaperFolders()) {
			RenderFolderPathText(folder);
		}
	}
	ImGui::End();

	ImGui::Begin("##Buttons",
				 nullptr,
				 ImGuiWindowFlags_HorizontalScrollbar |
					 ImGuiWindowFlags_NoDecoration);
	{
		f32 buttonRegionWidth = ImGui::GetContentRegionAvail().x;
		f32 buttonRegionButtonWidth = buttonRegionWidth - ImGui::GetStyle().WindowPadding.x;
		ImGui::BeginDisabled(mIsFileDialogOpen);
		if (ImGui::Button("+", ImVec2(buttonRegionButtonWidth, 0)) && !mIsFileDialogOpen) {
			// TODO: In App verlagern und den threadpool benutzen
			mIsFileDialogOpen = true;
			mFoldersFuture = std::async(std::launch::async, []() {
				NFD::UniquePathSet outPaths;
				nfdresult_t result = NFD::PickFolderMultiple(outPaths, "C:\\Wallpaper");
				if (result == NFD_OKAY) {
					nfdpathsetsize_t numPaths;
					NFD::PathSet::Count(outPaths, numPaths);
					std::vector<fs::path> returnVec;
					returnVec.reserve(numPaths);

					for (nfdpathsetsize_t i = 0; i < numPaths; ++i) {
						NFD::UniquePathSetPath path;
						NFD::PathSet::GetPath(outPaths, i, path);
						returnVec.push_back(path.get());
					}
					return returnVec;
				} else {
					return std::vector<fs::path>{};
				}
			});
		}
		ImGui::EndDisabled();

		using namespace std::literals;
		if (mIsFileDialogOpen && mFoldersFuture.valid() && mFoldersFuture.wait_for(0s) == std::future_status::ready) {
			auto folders = mFoldersFuture.get();
			if (folders.empty())
				log->warn("NFD hat keine Folder zurückgegeben: ", NFD::GetError());
			else {
				for (const auto &path : folders)
					mApp->AddWallpaperFolder(path);
			}
			mIsFileDialogOpen = false;
		}
		if (ImGui::Button("Manual", ImVec2(buttonRegionButtonWidth * 2 / 3, 0))) {
			mApp->SetRandomWallpaperAsync();
		}

		ImGui::SameLine();

		static size_t currentIntervalID = 3;
		for (size_t i = 0; i < intervals.size(); i++) {
			if (GetSetting<Setting::WallpaperIntervalInSeconds>() == intervals[i]) {
				currentIntervalID = i;
				break;
			}
		}
		f32 dropdownWidth = buttonRegionButtonWidth * 1 / 3 - ImGui::GetStyle().WindowPadding.x;

		if (ImGui::BeginCombo("##SelectInterval", intervalStrings[currentIntervalID], ImGuiComboFlags_NoArrowButton)) {
			for (size_t i = 0; i < intervals.size(); i++) {
				bool isSelected = (currentIntervalID == i);
				if (ImGui::Selectable(intervalStrings[i], isSelected, 0, ImVec2(dropdownWidth, 0))) {
					SetSetting<Setting::WallpaperIntervalInSeconds>(intervals[i]);
					log->trace("Interval auf {} gesetzt", intervalStrings[i]);
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
	ImGui::End();

	ImGui::Begin("Current Wallpapers", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration);
	{
		static fs::path currentWallpaper, lastWallpaper;
		currentWallpaper = mApp->GetCurrentWallpaperPath();

		if (currentWallpaper != lastWallpaper) {
			UpdateWallpaperTexture();
			lastWallpaper = currentWallpaper;
		}
		if (mCurrentWallpaper.texture != 0) {
			// Current Wallpaper Filename Text
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
			if (ImGui::Button(currentWallpaper.filename().string().c_str())) {
				std::string cmd = "explorer /select,\"" + currentWallpaper.string() + "\"";
				std::system(cmd.c_str());
			}
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar();
			ImGui::SameLine();

			// Remaining Time Text
			auto remainingTime = mApp->GetRemainingWallpaperIntervalTimeInS();
			if (remainingTime > 60 * 60)
				ImGui::Text("%dh:%dm:%ds", remainingTime / 60 / 60, (remainingTime / 60) % 60, remainingTime % 60);
			if (remainingTime > 60)
				ImGui::Text("%dm:%ds", remainingTime / 60, remainingTime % 60);
			else
				ImGui::Text("%ds", remainingTime);

			// Current Wallpaper Preview
			ImVec2 availSize = ImGui::GetContentRegionAvail();
			f32 ratio = (f32)mCurrentWallpaper.width / (f32)mCurrentWallpaper.height;
			f32 displayHeight = availSize.y;
			f32 displayWidth = displayHeight * ratio;
			if (displayWidth > availSize.x) {
				displayWidth = availSize.x;
				displayHeight = displayWidth / ratio;
			}
			if (ImGui::ImageButton(
					"##WallpaperBtn",
					(ImTextureID)(intptr_t)mCurrentWallpaper.texture,
					ImVec2(displayWidth, displayHeight),
					ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0),
					ImVec4(1, 1, 1, 1))) {
				if (!currentWallpaper.empty() && fs::exists(currentWallpaper.parent_path())) {
					std::string cmd = "start \"\" \"" + currentWallpaper.string() + "\"";
					std::system(cmd.c_str());
				}
			}
		} else {
			ImGui::Text("Kein Wallpaper");
		}
	}
	ImGui::End();
}

auto GUI::InitRenderer() -> void {
	SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
	// [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
	}

	if (!TTF_Init())
		throw std::runtime_error(std::string("TTF_Init failed "));

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100 (WebGL 1.0)
	const char *glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
	// GL ES 3.0 + GLSL 300 es (WebGL 2.0)
	const char *glsl_version = "#version 300 es";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char *glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	auto WindowWidth = GetSetting<Setting::GUIWindowWidth>();
	auto WindowHeight = GetSetting<Setting::GUIWindowHeight>();
	mWindow = SDL_CreateWindow(APP_NAME, WindowWidth, WindowHeight, window_flags);
	if (mWindow == nullptr) {
		throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
	}
	SDL_SetWindowPosition(mWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	mGLContext = SDL_GL_CreateContext(mWindow);
	if (mGLContext == nullptr) {
		throw std::runtime_error(std::string("SDL_GL_CreateContext failed: ") + SDL_GetError());
	}

	SDL_GL_MakeCurrent(mWindow, mGLContext);
	SDL_GL_SetSwapInterval(1); // Enable vsync
	SDL_ShowWindow(mWindow);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	  // Enable Multi-Viewport / Platform Windows
	// io.ConfigViewportsNoAutoMerge = true;
	// io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	// ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForOpenGL(mWindow, mGLContext);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	// - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
	// io.Fonts->AddFontDefault();
	// io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	// io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	// ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	// IM_ASSERT(font != nullptr);

	InitImguiStyle();
	clear_color = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
}

auto GUI::Shutdown() -> void {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DestroyContext(mGLContext);
	SDL_DestroyWindow(mWindow);

	TTF_CloseFont(mFont);
	TTF_CloseFont(mFontOutline);

	TTF_Quit();
	SDL_Quit();
}

auto GUI::Init(App *app) -> void {
	mApp = app;
	InitRenderer();

	fs::path TTFFontPath = GetSetting<Setting::LatoRegularFontFilePath>();
	auto TTFFontSize = GetSetting<Setting::TTFFontSize>();
	mFont = TTF_OpenFont(TTFFontPath.string().c_str(), TTFFontSize);
	mFontOutline = TTF_OpenFont(TTFFontPath.string().c_str(), TTFFontSize);
	if (!mFont || !mFontOutline) {
		log->error("Fehler beim Font laden.");
	} else {
		log->debug("Font: {} erfolgreich geladen.", TTFFontPath.filename().string());
	}
	TTF_SetFontOutline(mFontOutline, 1);
}

auto GUI::EndFrame() -> void {
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, mGLContext) directly)
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
		SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
	}

	SDL_GL_SwapWindow(mWindow);
}

auto GUI::NewFrame() -> void {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

auto GUI::RenderDemoWindows() -> void {
	ImGuiIO &io = ImGui::GetIO();
	(void)io;

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (GetSetting<Setting::GUIShowDemoWindow>())
		ImGui::ShowDemoWindow(&GetSetting<Setting::GUIShowDemoWindow>());

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");								   // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &GetSetting<Setting::GUIShowDemoWindow>()); // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &GetSetting<Setting::GUIShowAnotherWindow>());

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);			 // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (GetSetting<Setting::GUIShowAnotherWindow>()) {
		ImGui::Begin("Another Window", &GetSetting<Setting::GUIShowAnotherWindow>()); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			GetSetting<Setting::GUIShowAnotherWindow>() = false;
		ImGui::End();
	}
}

auto GUI::UpdateWallpaperTexture() -> void {
	if (mCurrentWallpaper.texture != 0) {
		glDeleteTextures(1, &mCurrentWallpaper.texture);
		mCurrentWallpaper.texture = 0;
	}
	LoadFreeImageAsTexture(mApp->GetCurrentWallpaperPath());
}

auto GUI::LoadFreeImageAsTexture(const fs::path &wallpaperPath) -> GLuint {
	FIBITMAP *bitmap = Util::LoadImage(wallpaperPath);
	FIBITMAP *converted = FreeImage_ConvertTo32Bits(bitmap);
	FreeImage_Unload(bitmap);

	if (!converted) {
		log->error("Fehler beim konvertieren von {}", wallpaperPath.string());
		return 0;
	}

	FreeImage_FlipVertical(converted);

	u8 *bits = FreeImage_GetBits(converted);
	mCurrentWallpaper.width = FreeImage_GetWidth(converted);
	mCurrentWallpaper.height = FreeImage_GetHeight(converted);

	if (!bits || mCurrentWallpaper.width == 0 || mCurrentWallpaper.height == 0) {
		log->error("Ungültige Bilddaten: {}", wallpaperPath.string());
		FreeImage_Unload(converted);
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mCurrentWallpaper.width, mCurrentWallpaper.height,
				 0, GL_BGRA, GL_UNSIGNED_BYTE, bits);

	FreeImage_Unload(converted);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		log->error("OpenGL Fehler beim Laden der Texture: {}", err);
		glDeleteTextures(1, &textureID);
		return 0;
	}
	mCurrentWallpaper.texture = textureID;
	return textureID;
}

auto GUI::InitImguiStyle() -> void {
	// Everforest style by DestroyerDarkNess from ImThemes
	ImGuiStyle &style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(6.0f, 3.0f);
	style.WindowRounding = 6.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(5.0f, 1.0f);
	style.FrameRounding = 3.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 13.0f;
	style.ScrollbarRounding = 16.0f;
	style.GrabMinSize = 20.0f;
	style.GrabRounding = 2.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;
	style.TabCloseButtonMinWidthSelected = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.8745098114013672f, 0.8705882430076599f, 0.8392156958580017f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5843137502670288f, 0.572549045085907f, 0.5215686559677124f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.3137255012989044f, 0.2862745225429535f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.3137255012989044f, 0.2862745225429535f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4000000059604645f, 0.3607843220233917f, 0.3294117748737335f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4862745106220245f, 0.43529412150383f, 0.3921568691730499f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.3137255012989044f, 0.2862745225429535f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.3137255012989044f, 0.2862745225429535f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2352941185235977f, 0.2196078449487686f, 0.2117647081613541f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.4862745106220245f, 0.43529412150383f, 0.3921568691730499f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4000000059604645f, 0.3607843220233917f, 0.3294117748737335f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3137255012989044f, 0.2862745225429535f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5960784554481506f, 0.5921568870544434f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.5960784554481506f, 0.5921568870544434f, 0.1019607856869698f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.4000000059604645f, 0.3607843220233917f, 0.3294117748737335f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4862745106220245f, 0.43529412150383f, 0.3921568691730499f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.4000000059604645f, 0.3607843220233917f, 0.3294117748737335f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4862745106220245f, 0.43529412150383f, 0.3921568691730499f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4862745106220245f, 0.43529412150383f, 0.3921568691730499f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.4000000059604645f, 0.3607843220233917f, 0.3294117748737335f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4862745106220245f, 0.43529412150383f, 0.3921568691730499f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.3137255012989044f, 0.2862745225429535f, 0.2705882489681244f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.4000000059604645f, 0.3607843220233917f, 0.3294117748737335f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.8392156958580017f, 0.7490196228027344f, 0.4000000059604645f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.7411764860153198f, 0.7176470756530762f, 0.4196078479290009f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.8392156958580017f, 0.7490196228027344f, 0.4000000059604645f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.8392156958580017f, 0.7490196228027344f, 0.4000000059604645f, 0.6094420552253723f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.8392156958580017f, 0.7490196228027344f, 0.4000000059604645f, 0.4313725531101227f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.8392156958580017f, 0.7490196228027344f, 0.4000000059604645f, 0.9019607901573181f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}
