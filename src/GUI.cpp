#include "GUI.hpp"

#include "App.hpp"

namespace {
bool show_demo_window = false;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

constexpr f32 FOLDERPANEL_SIZE_FAKTOR = 1.f / 3.f;
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

	// Styling für beide Child-Fenster
	const ImGuiWindowFlags childFlags =
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize;

	// Linke Seitenleiste (Folder Panel)
	f32 folderPanelWidth = viewport->Size.x * FOLDERPANEL_SIZE_FAKTOR;
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
	ImGui::BeginChild("Sidebar", ImVec2(folderPanelWidth, 0), true, childFlags);
	{
		RenderFolderPanel();
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
	ImGui::BeginChild("MainContent", ImVec2(0, 0), true, childFlags);
	{
		static fs::path currentWallpaper;

		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - 200) * 0.5f);
		if (ImGui::Button("Random Wallpaper", ImVec2(200, 40))) {
			currentWallpaper = mApp->GetRandomWallpaper();
			auto bitmap = Util::LoadImage(currentWallpaper);
			mCurrentWallpaper.texture = LoadFreeImageAsTexture(bitmap, mCurrentWallpaper.width, mCurrentWallpaper.height);
			mApp->SetWallpaper(currentWallpaper);
		}

		if (!currentWallpaper.empty()) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
			ImGui::TextUnformatted("Current Wallpaper:");
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.8f, 1.0f, 1.0f));
			ImGui::TextWrapped("%s", currentWallpaper.string().c_str());
			ImGui::PopStyleColor();
			if (mCurrentWallpaper.texture != 0) {
				ImVec2 avail = ImGui::GetContentRegionAvail();

				f32 scale = std::min(avail.x / mCurrentWallpaper.width, avail.y / mCurrentWallpaper.height);
				ImVec2 size = ImVec2(mCurrentWallpaper.width * scale, mCurrentWallpaper.height * scale);

				ImGui::Image(reinterpret_cast<ImTextureID>((uintptr_t)mCurrentWallpaper.texture), size);
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	// RenderDemoWindows(); // Falls noch benötigt

	ImGui::End();
	EndFrame();
}

auto RenderFolderPathText(const WallpaperFolder &folder) -> void {
	ImGui::Text("%s", folder.path.string().c_str());
	// for (const auto &pic : folder.picturePaths) {
	// 	ImGui::Text("  %s%s", indent.c_str(), pic.filename().string().c_str());
	// }
}

auto GUI::RenderFolderPanel() -> void {
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	f32 contentHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() * 2;
	f32 panelWidth = viewport->Size.x * FOLDERPANEL_SIZE_FAKTOR;
	f32 buttonWidth = panelWidth - ImGui::GetStyle().WindowPadding.x * 2;

	ImGui::BeginChild("##FolderContent",
					  ImVec2(buttonWidth, contentHeight),
					  true,
					  ImGuiWindowFlags_HorizontalScrollbar |
						  ImGuiWindowFlags_NoDecoration);
	for (const auto &folder : mApp->GetWallpaperFolders()) {
		RenderFolderPathText(folder);
	}
	ImGui::Separator();
	for (const auto &[path, count] : mApp->GetPathCount()) {
		ImGui::Text("%s\t %d", path.string().c_str(), count);
	}
	ImGui::EndChild();

	if (ImGui::Button("+", ImVec2(buttonWidth, 0))) {
		mApp->AddWallpaperFolder("C:\\Wallpaper\\Test");
		mApp->AddWallpaperFolder("C:\\Wallpaper\\Test\\1");
		mApp->AddWallpaperFolder("C:\\Wallpaper\\Test\\2");
		mApp->AddWallpaperFolder("C:\\Wallpaper\\Test\\3");
		mApp->AddWallpaperFolder("C:\\Wallpaper\\Test\\Anime");
		mApp->AddWallpaperFolder("C:\\Wallpaper\\Test\\Fake");
	}
	if (ImGui::Button("Random", ImVec2(buttonWidth, 0))) {
		auto wallpaper = mApp->GetRandomWallpaper();
		mApp->SetWallpaper(wallpaper);
	}
}

auto GUI::InitRenderer() -> void {
	// [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
	}

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
	mWindow = SDL_CreateWindow(APP_NAME, 1280, 720, window_flags);
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
	ImGui::StyleColorsDark();
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

	style.WindowRounding = 0.0f;
	style.ChildRounding = 5.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 3.0f;

	ImVec4 *colors = style.Colors;
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
}

auto GUI::Shutdown() -> void {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DestroyContext(mGLContext);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}

auto GUI::Init(App *app) -> void {
	mApp = app;
	InitRenderer();
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
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");		   // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

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
	if (show_another_window) {
		ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
}

auto GUI::LoadFreeImageAsTexture(FIBITMAP *bitmap, i32 &width, i32 &height) -> GLuint {
	width = FreeImage_GetWidth(bitmap);
	height = FreeImage_GetHeight(bitmap);
	auto b = FreeImage_ConvertTo32Bits(bitmap);
	FreeImage_FlipVertical(b);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, FreeImage_GetBits(b));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	FreeImage_Unload(bitmap);
	return tex;
}