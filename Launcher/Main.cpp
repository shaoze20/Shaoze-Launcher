#include "D3D11/D3D11.h"

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int Tab = 0, Color_ = 0;;
ImVec4* Color = nullptr;
ImFont* Font_Big = nullptr;
ImFont* Font_Login = nullptr;
HWND hwnd = nullptr;
std::wstring ExePath;
int width = 0, height = 0;
const char* gonggao = nullptr;
const char* banbena = nullptr;
HMODULE MainHandle = nullptr;
std::unordered_map<HWND, ImVec2> viewportSizeCache;

void ImGui_MemuSetstyles() {
	ImGuiStyle& styles = ImGui::GetStyle();

	styles.AntiAliasedFill = true;                    // enable抗锯齿填充
	styles.AntiAliasedLines = true;                   // enable抗锯齿线条
	styles.AntiAliasedLinesUseTex = true;             // 使用纹理进行抗锯齿线条
	styles.ButtonTextAlign = ImVec2(0.5, 0.5);        // 按钮文本对齐方式
	styles.CellPadding = ImVec2(4.0, 2.0);            // 单元格内边距
	styles.ChildBorderSize = 1.0;                     // 子窗口边框大小
	styles.ChildRounding = 10.0;                      // 子窗口圆角半径
	styles.CircleTessellationMaxError = 0.3;          // 圆形细分最大误差
	styles.ColorButtonPosition = 1;                   // 颜色按钮位置
	styles.ColumnsMinSpacing = 6.0;                   // 列间最小间距
	styles.CurveTessellationTol = 1.25;               // 曲线细分容差
	styles.DisabledAlpha = 0.6;                       // 禁用状态透明度
	styles.DisplaySafeAreaPadding = ImVec2(3.0, 3.0); // 显示安全区域内边距
	styles.DisplayWindowPadding = ImVec2(19.0, 19.0); // 显示窗口内边距
	styles.FrameBorderSize = 0.0;                     // 框架边框大小
	styles.FramePadding = ImVec2(4.0, 3.0);           // 框架内边距
	styles.FrameRounding = 5.0;                       // 框架圆角半径
	styles.GrabMinSize = 15.0;                        // 拖动控件最小尺寸
	styles.GrabRounding = 5.0;                        // 拖动控件圆角半径
	styles.IndentSpacing = 21.0;                      // 缩进间距
	styles.ItemInnerSpacing = ImVec2(4.0, 4.0);       // 项目内部间距
	styles.ItemSpacing = ImVec2(8.0, 4.0);            // 项目间距
	styles.LogSliderDeadzone = 4.0;                   // 对数滑块死区
	styles.MouseCursorScale = 1.0;                    // 鼠标光标缩放
	styles.PopupBorderSize = 1.0;                     // 弹出窗口边框大小
	styles.PopupRounding = 0.0;                       // 弹出窗口圆角半径
	styles.ScrollbarRounding = 11.0;                  // 滚动条圆角半径
	styles.ScrollbarSize = 10.0;                      // 滚动条大小
	styles.SelectableTextAlign = ImVec2(0.0, 0.0);    // 可选文本对齐方式
	styles.TabBorderSize = 0.0;                       // 标签页边框大小
	styles.TabMinWidthForCloseButton = 0.0;           // 关闭按钮的标签页最小宽度
	styles.TabRounding = 4.0;                         // 标签页圆角半径
	styles.TouchExtraPadding = ImVec2(0.0, 0.0);      // 触控额外内边距
	styles.WindowBorderSize = 0.0;                    // 窗口边框大小
	styles.WindowMenuButtonPosition = 0;              // 窗口菜单按钮位置
	styles.WindowMinSize = ImVec2(32.0, 32.0);        // 窗口最小尺寸
	styles.WindowPadding = ImVec2(0.0, 0.0);          // 窗口内边距
	styles.WindowRounding = 15.0f;					  // 窗口圆角半径
	styles.WindowTitleAlign = ImVec2(0.5, 0.5);       // 窗口标题对齐
}

bool Loadimge() {
	ExePath = GetExePath();

	//图片
	std::wstring Img_Home_bj = ExePath + L"\\Data\\Image\\Home.png";
	std::wstring Img_wuwa_bj = ExePath + L"\\Data\\Image\\Wuwa.png";
	std::wstring Img_TKA_bj = ExePath + L"\\Data\\Image\\TKA.png";
	std::wstring Img_Login_bj = ExePath + L"\\Data\\Image\\Login.png";
	std::wstring Img_Music_bj = ExePath + L"\\Data\\Image\\Music.png";
	std::wstring Img_Panel_bj = ExePath + L"\\Data\\Image\\Panel.png";
	std::wstring Img_Messag_bj = ExePath + L"\\Data\\Image\\Messag.png";

	//动图
	//std::wstring lian = ExePath + L"\\Data\\Videos\\Home.gif";

	//视频
	std::wstring mp4_Home_bj = ExePath + L"\\Data\\Videos\\Home.mp4";
	std::wstring mp4_Login_bj = ExePath + L"\\Data\\Videos\\Login.mp4";

	//内部图片
	HRESULT res[] = {
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG1), L"PNG", &Img::User, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG2), L"PNG", &Img::Home, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG3), L"PNG", &Img::wuwa, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG4), L"PNG", &Img::Settings, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG5), L"PNG", &Img::bilibili, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG6), L"PNG", &Img::qq, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG7), L"PNG", &Img::afd, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG8), L"PNG", &Img::Login, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG9), L"PNG", &Img::TKA, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG10), L"PNG", &Img::circled, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG11), L"PNG", &Img::Pause, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG12), L"PNG", &Img::Tp, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG13), L"PNG", &Img::Np, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG14), L"PNG", &Img::Volume, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG15), L"PNG", &Img::music, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG16), L"PNG", &Img::Panel, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG17), L"PNG", &Img::messag, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG18), L"PNG", &Img::Useravatar, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG19), L"PNG", &Img::github, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG20), L"PNG", &Img::unknownmusic, &width, &height),
	Image::LoadTextureFromResource_DX11(g_pd3dDevice, g_pd3dDeviceContext,MainHandle, MAKEINTRESOURCE(IDB_PNG21), L"PNG", &Img::Discord, &width, &height),
	};

	for (auto hr : res) {
		if (FAILED(hr)) {
			return false;
		}
	}

	//外部图片
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_Home_bj.c_str(), &Img::Home_bj, &width, &height),
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_wuwa_bj.c_str(), &Img::wuwa_bj, &width, &height),
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_TKA_bj.c_str(), &Img::TKA_bj, &width, &height),
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_Login_bj.c_str(), &Img::Login_bj, &width, &height),
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_Music_bj.c_str(), &Img::Music_bj, &width, &height),
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_Panel_bj.c_str(), &Img::Panel_bj, &width, &height),
	Image::LoadTextureFromFile_DX11(g_pd3dDevice, g_pd3dDeviceContext, Img_Messag_bj.c_str(), &Img::messag_bj, &width, &height),

	//加载动图
	//GIF::LoadGifFromFile(lian.c_str(), "Home", g_pd3dDevice);

	//加载视频
	Videos::LoadVideoFromFile(mp4_Home_bj.c_str(), "Home", g_pd3dDevice);
	Videos::LoadVideoFromFile(mp4_Login_bj.c_str(), "Login", g_pd3dDevice);

	return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	strcpy_s(g_Version, sizeof(g_Version), "1.6.0b");
	//StartConsole();

	if (FindWindowA(NULL, "shaoze")) {
		MessageBoxW(NULL, L"禁止程序多开!", L"网络验证提示", MB_OK | MB_ICONERROR);
		return 0;
	}

	//if (!IsRunAsAdmin()) {
	//	MessageBoxW(NULL, Enc(L"请右键以管理员身份运行本程序!"), Enc(L"网络验证提示"), MB_OK | MB_ICONERROR);
	//	return 0;
	//}

	//if (!Verify::Loadserver()) {
	//	MessageBoxW(NULL, Enc(L"服务器连接失败!"), Enc(L"网络验证提示"), MB_OK | MB_ICONERROR);
	//	return 0;
	//}

	//if (!Verify::Announcem()) {
	//	MessageBoxW(NULL, Enc(L"公告获取失败!"), Enc(L"网络验证提示"), MB_OK | MB_ICONERROR);
	//	return 0;
	//}

	//if (!Verify::Version()) {
	//	MessageBoxW(NULL, Enc(L"您使用的版本过低,请进官网下载最新版!"), Enc(L"网络验证提示"), MB_OK | MB_ICONERROR);
	//	ShellExecuteA(NULL, "open", Enc("https://github.com/shaoze20/Shaoze-Launcher"), NULL, NULL, SW_SHOWNORMAL);
	//	return 0;
	//}

	//读取配置
	json j;
	std::ifstream infile("C:\\Shaoze\\Data.json");
	if (infile.is_open()) {
		try {
			infile >> j;
			infile.close();

			Json::LoadStringField(j, "User", Account, sizeof(Account));
			Json::LoadStringField(j, "Password", Password, sizeof(Password));
			Json::LoadStringField(j, "TKA_pach", TKA_pach, sizeof(TKA_pach));
			Json::LoadStringField(j, "Wuwa_pach", Wuwa_pach, sizeof(Wuwa_pach));

			char buffer[16];
			Json::LoadStringField(j, "Agreement", buffer, sizeof(buffer));
			if (strcmp(buffer, "true") == 0) Agreement = true;
		}
		catch (const std::exception& e) {}
	}


	// 注册窗口
	MainHandle = GetModuleHandleW(NULL);
	const wchar_t* className = L"Shaoze";
	HICON hIcon = LoadIcon(MainHandle, MAKEINTRESOURCE(IDI_ICON1));

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = MainHandle;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszClassName = className;
	wc.hIcon = hIcon;
	wc.hIconSm = hIcon;

	if (!::RegisterClassExW(&wc)) {
		MessageBoxW(NULL, L"窗口类注册失败!", L"错误", MB_OK | MB_ICONERROR);
		return 0;
	}

	//创建窗口
	hwnd = ::CreateWindowExW(WS_EX_LAYERED, wc.lpszClassName, L"shaoze", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (!hwnd) {
		MessageBoxW(NULL, L"窗口创建失败!", L"错误", MB_OK | MB_ICONERROR);
		::UnregisterClassW(className, wc.hInstance);
		return 0;
	}

	if (!CreateDeviceD3D(hwnd)) {
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	//加载背景
	if (!Loadimge()) {
		MessageBoxW(NULL, L"背景图加载失败!", L"错误", MB_OK | MB_ICONERROR);
		CleanupDeviceD3D();
		::DestroyWindow(hwnd);
		::UnregisterClassW(className, wc.hInstance);
		return 0;
	}

	//初始音乐播放器
	Music::InitAudio();
	MusicPath = ExePath + L"\\Data\\Music";
	Music::ScanMusicFolder(MusicPath.string());

	//初始化系统信息
	hw = GetHardwareInfo();

	::ShowWindow(hwnd, SW_HIDE);
	::UpdateWindow(hwnd);

	// 初始化 ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	io.IniFilename = nullptr;

	ImFontConfig font_cfg;
	font_cfg.FontDataOwnedByAtlas = false;
	ImFont* Font = io.Fonts->AddFontFromMemoryTTF((void*)Font_data, Font_size, 13.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
	Font_Login = io.Fonts->AddFontFromMemoryTTF((void*)Font_data, Font_size, 14.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());
	Font_Big = io.Fonts->AddFontFromMemoryTTF((void*)Font_data, Font_size, 20.0f, &font_cfg, io.Fonts->GetGlyphRangesChineseFull());

	//ImGui样式
	ImGui_MemuSetstyles();
	ImGui::StyleColorsLight();

	static bool done = false;
	while (!done) {
		MSG msg;
		bool userInteracted = false;
		static auto lastInteractionTime = std::chrono::steady_clock::now();
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> timeSinceLastInteraction = currentTime - lastInteractionTime;

		while (::PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
			if (msg.message == WM_QUIT) {
				done = true;
				break;
			}

			if (msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN) {
				userInteracted = true;
				lastInteractionTime = std::chrono::steady_clock::now();
			}
		}

		if (done) break;
		if (!userInteracted && timeSinceLastInteraction.count() > 0.1f) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 渲染界面
		//GIF::UpdateAllGifs(ImGui::GetIO().DeltaTime);
		Videos::UpdateAllVideos(g_pd3dDeviceContext);

		if (!InfoBox::bEnable) Render::ShowVerifyResult(InfoBox::Information);
		if (!iskey) {
			Render::Login_Interface();
		}
		else {
			Render::Inject_Interface();
		}

		ImGui::Render();
		const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

			for (int i = 1; i < platform_io.Viewports.Size; i++) {
				ImGuiViewport* viewport = platform_io.Viewports[i];
				HWND hwnd_view = (HWND)viewport->PlatformHandleRaw;
				if (!hwnd_view) continue;

				//LONG_PTR ex = GetWindowLongPtr(hwnd_view, GWL_EXSTYLE);
				//if (!(ex & WS_EX_LAYERED)) {
				//	SetWindowLongPtr(hwnd_view, GWL_EXSTYLE, ex | WS_EX_LAYERED);
				//	SetLayeredWindowAttributes(hwnd_view, RGB(0, 0, 255), 0, LWA_COLORKEY);//背景透明
				//}

				ImVec2 currSize = viewport->Size;
				auto it = viewportSizeCache.find(hwnd_view);
				bool sizeChanged = (it == viewportSizeCache.end()) || (it->second.x != currSize.x) || (it->second.y != currSize.y);
				if (sizeChanged) {
					viewportSizeCache[hwnd_view] = currSize;
					HRGN hRgn = CreateRoundRectRgn(0, 0, (int)currSize.x, (int)currSize.y, 12, 12);
					SetWindowRgn(hwnd_view, hRgn, TRUE);
					DeleteObject(hRgn);
				}
			}

			ImGui::RenderPlatformWindowsDefault();
		}

		g_pSwapChain->Present(1, 0);
	}

	// 清理
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	GIF::UnloadAllGifs();
	Videos::UnloadAllVideos();
	WSACleanup();
	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_SIZE:
		if (g_pd3dDevice && g_pSwapChain && wParam != SIZE_MINIMIZED) {
			const UINT width = LOWORD(lParam);
			const UINT height = HIWORD(lParam);

			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;

	case WM_SYSCOMMAND:
		// 禁用 ALT 菜单
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case WM_DPICHANGED:
		if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports) && lParam) {
			const RECT* suggested_rect = reinterpret_cast<const RECT*>(lParam);
			SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}