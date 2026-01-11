#pragma once

#include "../Imgui/imconfig.h"
#include "../Imgui/imgui.h"
#include "../Imgui/imgui_impl_dx11.h"
#include "../Imgui/imgui_impl_win32.h"
#include "../Imgui/imgui_internal.h"
#include "../Imgui/imstb_rectpack.h"
#include "../Imgui/imstb_textedit.h"
#include "../Imgui/imstb_truetype.h"
#include "../Imgui/Font.h"

#include <Windows.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <tchar.h>
#include <wincodec.h>
#include <fstream>
#include <thread>
#include <d3d11.h>
#include <mmeapi.h>
#include <mutex>
#include <winhttp.h>
#include <map>
#include <string>
#include <wrl/client.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <chrono>
#include <mfobjects.h>
#include <deque>

#include "../Utils/json.h"
#include "../Memu/Global.h"
#include "../resource.h"
//#include "../Inject/InjectDLL.h"
#include "../Verify/Verify.h"

extern ID3D11RenderTargetView* g_mainRenderTargetView;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11Device* g_pd3dDevice;
extern HWND hwnd;

class Videos {
public:
	static bool LoadVideoFromFile(const wchar_t* path, const std::string& name, ID3D11Device* device);
	static ID3D11ShaderResourceView* GetVideoSRV(const std::string& name);
	static void UpdateAllVideos(ID3D11DeviceContext* ctx);
	static void UnloadAllVideos();
};

class GIF {
public:
	static bool LoadGifFromFile(const wchar_t* path, const std::string& name, ID3D11Device* device);
	static ID3D11ShaderResourceView* GetGifSRV(const std::string& name);
	static void UpdateAllGifs(float deltaTime);
	static void UnloadAllGifs();
};

class Image {
public:
	static HRESULT LoadTextureFromResource_DX11(ID3D11Device* device, ID3D11DeviceContext* context, HMODULE hModule, LPCWSTR resourceName, LPCWSTR resourceType, ID3D11ShaderResourceView** outSRV, int* width, int* height);
	static HRESULT LoadTextureFromFile_DX11(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
	static HRESULT LoadTextureFromURL_DX11(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& url, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CleanupRenderTarget();
void CreateRenderTarget();