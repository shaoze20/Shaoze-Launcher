#include "D3D11.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "d3d11.lib")
#pragma warning(disable : 4996)

using Microsoft::WRL::ComPtr;

static inline double HnsToSec(LONGLONG t100ns) { return t100ns * 1e-7; }

struct GifData {
	std::vector<ComPtr<ID3D11ShaderResourceView>> frames;
	std::vector<int> delays;
	UINT width = 0, height = 0;
	size_t current = 0;
	float timer_ms = 0.f;
	bool loaded = false;
	bool visible = false;
};

struct VideoData {
	ComPtr<IMFSourceReader> reader;
	ComPtr<ID3D11Texture2D> tex;
	ComPtr<ID3D11ShaderResourceView> srv;
	UINT width = 0, height = 0;
	int  stride = 0;
	double frameDur = 1.0 / 30.0;
	std::atomic<bool> loaded{ false };
	std::atomic<bool> eof{ false };
	std::atomic<bool> stop{ false };
	std::mutex mtx;
	std::deque<std::vector<uint8_t>> queue;
	size_t queue_cap = 3;
	std::thread th;
	std::vector<uint8_t> cpuRGBA;
	std::atomic<bool> active{ false };
	std::chrono::steady_clock::time_point lastActiveTime;
};

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
IWICImagingFactory* pWIC = nullptr;
ID3D11ShaderResourceView* pTextureSRV = nullptr;

std::map<std::string, GifData> g_gifs;
bool g_wic_init = false, g_mf_init = false;
std::map<std::string, std::shared_ptr<VideoData>> g_videos;

void CreateRenderTarget() {
	if (g_pSwapChain) {
		ID3D11Texture2D* pBackBuffer;
		g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
		pBackBuffer->Release();
	}
}

#include <dxgi1_2.h> 
bool CreateDeviceD3D(HWND hWnd) {
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupRenderTarget() {
	if (g_mainRenderTargetView) {
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = nullptr;
	}
}

void CleanupDeviceD3D() {
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

HRESULT LoadTextureHelper(IWICImagingFactory* pWIC, IWICBitmapDecoder* pDecoder, IWICBitmapFrameDecode* pFrame, IWICFormatConverter* pConverter) {
	if (pConverter) {
		pConverter->Release();
	}
	if (pFrame) {
		pFrame->Release();
	}
	if (pDecoder) {
		pDecoder->Release();
	}
	if (pWIC) {
		pWIC->Release();
	}
	return E_FAIL;
}

bool CreateTextureFromData(ID3D11Device* device, const std::vector<BYTE>& imageData, int width, int height, ID3D11ShaderResourceView** out_srv) {
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageData.data();
	initData.SysMemPitch = width * 4;

	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hr = device->CreateTexture2D(&desc, &initData, &pTexture);
	if (FAILED(hr)) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	return hr == S_OK;
}

HRESULT LoadTextureFromMemory_DX11(const uint8_t* imageData, size_t dataSize, ID3D11Device* device, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height) {
	*out_srv = nullptr;

	// 创建WIC ImagingFactory
	IWICImagingFactory* pWIC = nullptr;
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWIC));
	if (FAILED(hr)) return hr;

	IWICStream* pStream = nullptr;
	hr = pWIC->CreateStream(&pStream);
	if (FAILED(hr)) {
		pWIC->Release();
		return hr;
	}

	hr = pStream->InitializeFromMemory(const_cast<unsigned char*>(imageData), static_cast<UINT>(dataSize));
	if (FAILED(hr)) {
		pStream->Release();
		pWIC->Release();
		return hr;
	}

	IWICBitmapDecoder* pDecoder = nullptr;
	hr = pWIC->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr)) {
		pStream->Release();
		pWIC->Release();
		return hr;
	}

	IWICBitmapFrameDecode* pFrame = nullptr;
	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr)) {
		pDecoder->Release();
		pStream->Release();
		pWIC->Release();
		return hr;
	}

	IWICFormatConverter* pConverter = nullptr;
	hr = pWIC->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) {
		pFrame->Release();
		pDecoder->Release();
		pStream->Release();
		pWIC->Release();
		return hr;
	}

	hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) {
		pConverter->Release();
		pFrame->Release();
		pDecoder->Release();
		pStream->Release();
		pWIC->Release();
		return hr;
	}

	UINT w, h;
	pConverter->GetSize(&w, &h);
	*out_width = (int)w;
	*out_height = (int)h;

	std::vector<BYTE> pixels(w * h * 4);
	hr = pConverter->CopyPixels(nullptr, w * 4, (UINT)pixels.size(), pixels.data());
	if (FAILED(hr)) {
		pConverter->Release();
		pFrame->Release();
		pDecoder->Release();
		pStream->Release();
		pWIC->Release();
		return hr;
	}

	hr = CreateTextureFromData(device, pixels, w, h, out_srv);

	pConverter->Release();
	pFrame->Release();
	pDecoder->Release();
	pStream->Release();
	pWIC->Release();

	return hr;
}

//通过路径加载图片
HRESULT Image::LoadTextureFromFile_DX11(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height) {
	*out_srv = nullptr;

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWIC));
	if (FAILED(hr)) return hr;

	IWICBitmapDecoder* pDecoder = nullptr;
	hr = pWIC->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ,WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr)) return LoadTextureHelper(pWIC, pDecoder, nullptr, nullptr);

	IWICBitmapFrameDecode* pFrame = nullptr;
	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr)) return LoadTextureHelper(pWIC, pDecoder, pFrame, nullptr);

	IWICFormatConverter* pConverter = nullptr;
	pWIC->CreateFormatConverter(&pConverter);
	pConverter->Initialize(
		pFrame, GUID_WICPixelFormat32bppRGBA,
		WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);

	UINT w, h;
	pConverter->GetSize(&w, &h);
	*out_width = (int)w;
	*out_height = (int)h;

	std::vector<BYTE> pixels(w * h * 4);
	pConverter->CopyPixels(nullptr, w * 4, (UINT)pixels.size(), pixels.data());

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = w;
	desc.Height = h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels.data();
	initData.SysMemPitch = w * 4;

	ID3D11Texture2D* tex = nullptr;
	hr = device->CreateTexture2D(&desc, &initData, &tex);
	if (FAILED(hr)) return LoadTextureHelper(pWIC, pDecoder, pFrame, pConverter);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(tex, &srvDesc, out_srv);
	tex->Release();

	pConverter->Release();
	pFrame->Release();
	pDecoder->Release();
	pWIC->Release();
	CoUninitialize();

	return hr;
}

//通过资源加载图片
HRESULT Image::LoadTextureFromResource_DX11(ID3D11Device* device, ID3D11DeviceContext* context, HMODULE hModule, LPCWSTR resourceName, LPCWSTR resourceType, ID3D11ShaderResourceView** outSRV, int* width, int* height) {
	if (!device || !outSRV) return E_INVALIDARG;
	*outSRV = nullptr;

	HRSRC hRes = FindResource(hModule, resourceName, resourceType);
	if (!hRes) return E_FAIL;

	DWORD resSize = SizeofResource(hModule, hRes);
	if (resSize == 0) return E_FAIL;

	HGLOBAL hGlobal = LoadResource(hModule, hRes);
	if (!hGlobal) return E_FAIL;

	void* pData = LockResource(hGlobal);
	if (!pData) return E_FAIL;

	IWICImagingFactory* pFactory = nullptr;
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) return hr;

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	if (FAILED(hr)) return hr;

	IWICStream* pStream = nullptr;
	hr = pFactory->CreateStream(&pStream);
	if (FAILED(hr)) { pFactory->Release(); return hr; }

	hr = pStream->InitializeFromMemory(reinterpret_cast<BYTE*>(pData), resSize);
	if (FAILED(hr)) { pStream->Release(); pFactory->Release(); return hr; }

	IWICBitmapDecoder* pDecoder = nullptr;
	hr = pFactory->CreateDecoderFromStream(pStream, nullptr, WICDecodeMetadataCacheOnLoad, &pDecoder);
	if (FAILED(hr)) { pStream->Release(); pFactory->Release(); return hr; }

	IWICBitmapFrameDecode* pFrame = nullptr;
	hr = pDecoder->GetFrame(0, &pFrame);
	if (FAILED(hr)) { pDecoder->Release(); pStream->Release(); pFactory->Release(); return hr; }

	UINT w = 0, h = 0;
	pFrame->GetSize(&w, &h);
	if (width) *width = w;
	if (height) *height = h;

	IWICFormatConverter* pConverter = nullptr;
	hr = pFactory->CreateFormatConverter(&pConverter);
	if (FAILED(hr)) { pFrame->Release(); pDecoder->Release(); pStream->Release(); pFactory->Release(); return hr; }

	hr = pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) { pConverter->Release(); pFrame->Release(); pDecoder->Release(); pStream->Release(); pFactory->Release(); return hr; }

	std::vector<BYTE> imageData(w * h * 4);
	hr = pConverter->CopyPixels(nullptr, w * 4, (UINT)imageData.size(), imageData.data());
	if (FAILED(hr)) { pConverter->Release(); pFrame->Release(); pDecoder->Release(); pStream->Release(); pFactory->Release(); return hr; }

	return CreateTextureFromData(device, imageData, w, h, outSRV);
}

//通过网络加载图片
HRESULT Image::LoadTextureFromURL_DX11(ID3D11Device* device, ID3D11DeviceContext* context, const std::wstring& url, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height) {
	*out_srv = nullptr;

	HINTERNET hSession = WinHttpOpen(L"A Windows HTTP/1.1 WinHTTP Example", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return E_FAIL;

	URL_COMPONENTS urlComponents = { 0 };
	urlComponents.dwStructSize = sizeof(urlComponents);
	urlComponents.dwHostNameLength = (DWORD)-1;
	urlComponents.dwUrlPathLength = (DWORD)-1;

	if (!WinHttpCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComponents)) {
		WinHttpCloseHandle(hSession);
		return E_FAIL;
	}

	HINTERNET hConnect = WinHttpConnect(hSession, std::wstring(urlComponents.lpszHostName, urlComponents.dwHostNameLength).c_str(), urlComponents.nPort, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return E_FAIL;
	}

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlComponents.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return E_FAIL;
	}

	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return E_FAIL;
	}

	if (!WinHttpReceiveResponse(hRequest, NULL)) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return E_FAIL;
	}

	DWORD dwSize = 0, dwDownloaded = 0;
	LPSTR pszOutBuffer;
	DWORD dwBufferLength = 0;
	std::vector<BYTE> imageData;

	do {
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return E_FAIL;
		}

		pszOutBuffer = (LPSTR)malloc(dwSize + 1);
		if (!pszOutBuffer) {
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return E_OUTOFMEMORY;
		}

		ZeroMemory(pszOutBuffer, dwSize + 1);
		if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
			free(pszOutBuffer);
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			WinHttpCloseHandle(hSession);
			return E_FAIL;
		}

		imageData.insert(imageData.end(), pszOutBuffer, pszOutBuffer + dwDownloaded);
		free(pszOutBuffer);
	} while (dwSize > 0);

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	HRESULT hr = LoadTextureFromMemory_DX11(imageData.data(), imageData.size(), device, out_srv, out_width, out_height);
	if (FAILED(hr)) return hr;

	return S_OK;
}

//加载GIF动画
int GetGifDelay(IWICBitmapFrameDecode* frame) {
	ComPtr<IWICMetadataQueryReader> meta;
	if (SUCCEEDED(frame->GetMetadataQueryReader(&meta)) && meta) {
		PROPVARIANT v;
		PropVariantInit(&v);
		if (SUCCEEDED(meta->GetMetadataByName(L"/grctlext/Delay", &v))) {
			UINT cs = (v.vt == VT_UI2) ? v.uiVal : v.ulVal;
			PropVariantClear(&v);
			if (cs == 0) cs = 10;
			return (int)cs * 10; // 1/100s -> ms
		}
	}
	return 100;
}

bool CreateGifFrameTexture(ID3D11Device* device, const std::vector<BYTE>& pixels, UINT w, UINT h, ID3D11ShaderResourceView** out_srv) {
	D3D11_TEXTURE2D_DESC td{};
	td.Width = w;
	td.Height = h;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA data{};
	data.pSysMem = pixels.data();
	data.SysMemPitch = w * 4;

	ComPtr<ID3D11Texture2D> tex;
	if (FAILED(device->CreateTexture2D(&td, &data, &tex))) return false;
	return SUCCEEDED(device->CreateShaderResourceView(tex.Get(), nullptr, out_srv));
}

bool GIF::LoadGifFromFile(const wchar_t* path, const std::string& name, ID3D11Device* device) {
	if (!g_wic_init) {
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)return false;
		g_wic_init = true;
	}

	if (!device) return false;
	GifData gif;

	ComPtr<IWICImagingFactory> factory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	if (FAILED(hr)) return false;

	ComPtr<IWICBitmapDecoder> decoder;
	hr = factory->CreateDecoderFromFilename(path, nullptr, GENERIC_READ,
		WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr)) return false;

	UINT canvasW = 0, canvasH = 0;
	{
		ComPtr<IWICMetadataQueryReader> decMeta;
		decoder->GetMetadataQueryReader(&decMeta);
		PROPVARIANT v; PropVariantInit(&v);
		if (SUCCEEDED(decMeta->GetMetadataByName(L"/logscrdesc/Width", &v))) { canvasW = v.uiVal; PropVariantClear(&v); }
		if (SUCCEEDED(decMeta->GetMetadataByName(L"/logscrdesc/Height", &v))) { canvasH = v.uiVal; PropVariantClear(&v); }
	}

	if (canvasW == 0 || canvasH == 0) {
		ComPtr<IWICBitmapFrameDecode> f0;
		decoder->GetFrame(0, &f0);
		f0->GetSize(&canvasW, &canvasH);
	}
	gif.width = canvasW; gif.height = canvasH;

	std::vector<BYTE> canvas(canvasW * canvasH * 4, 0);

	UINT frameCount = 0;
	decoder->GetFrameCount(&frameCount);
	if (frameCount == 0) return false;

	std::vector<BYTE> backup;

	for (UINT i = 0; i < frameCount; ++i) {
		ComPtr<IWICBitmapFrameDecode> frame;
		if (FAILED(decoder->GetFrame(i, &frame))) continue;

		UINT left = 0, top = 0, fw = 0, fh = 0, disposal = 0;
		{
			ComPtr<IWICMetadataQueryReader> fmeta;
			frame->GetMetadataQueryReader(&fmeta);
			auto getU = [&](LPCWSTR name, UINT& out) {
				PROPVARIANT v; PropVariantInit(&v);
				if (SUCCEEDED(fmeta->GetMetadataByName(name, &v))) {
					if (v.vt == VT_UI2) out = v.uiVal;
					else if (v.vt == VT_UI4) out = v.ulVal;
				}
				PropVariantClear(&v);
				};
			getU(L"/imgdesc/Left", left);
			getU(L"/imgdesc/Top", top);
			getU(L"/imgdesc/Width", fw);
			getU(L"/imgdesc/Height", fh);
			getU(L"/grctlext/Disposal", disposal);
		}

		if (fw == 0 || fh == 0)frame->GetSize(&fw, &fh);

		ComPtr<IWICFormatConverter> converter;
		factory->CreateFormatConverter(&converter);
		converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);

		std::vector<BYTE> src(fw * fh * 4);
		WICRect rc{ 0, 0, (INT)fw, (INT)fh };
		converter->CopyPixels(&rc, fw * 4, (UINT)src.size(), src.data());

		if (disposal == 3) backup = canvas;

		for (UINT y = 0; y < fh; ++y) {
			UINT cy = top + y;
			if (cy >= canvasH) continue;
			BYTE* dstRow = canvas.data() + cy * canvasW * 4 + left * 4;
			BYTE* srcRow = src.data() + y * fw * 4;
			for (UINT x = 0; x < fw; ++x) {
				if (left + x >= canvasW) continue;
				BYTE* d = dstRow + x * 4;
				BYTE* s = srcRow + x * 4;
				float sa = s[3] / 255.0f;
				float da = d[3] / 255.0f;
				float outA = sa + da * (1 - sa);
				if (outA > 0.0f) {
					d[0] = BYTE((s[0] * sa + d[0] * da * (1 - sa)) / outA);
					d[1] = BYTE((s[1] * sa + d[1] * da * (1 - sa)) / outA);
					d[2] = BYTE((s[2] * sa + d[2] * da * (1 - sa)) / outA);
				}
				d[3] = BYTE(outA * 255);
			}
		}

		ID3D11ShaderResourceView* srv = nullptr;
		if (CreateGifFrameTexture(device, canvas, canvasW, canvasH, &srv)) {
			gif.frames.push_back(srv);
			gif.delays.push_back(GetGifDelay(frame.Get()));
		}

		if (disposal == 2) {
			for (UINT y = 0; y < fh; ++y) {
				UINT cy = top + y;
				if (cy >= canvasH) continue;
				BYTE* dstRow = canvas.data() + cy * canvasW * 4 + left * 4;
				memset(dstRow, 0, fw * 4);
			}
		}
		else if (disposal == 3 && !backup.empty()) {
			canvas = backup;
		}
	}

	gif.loaded = !gif.frames.empty();
	g_gifs[name] = std::move(gif);
	return g_gifs[name].loaded;
}

void GIF::UpdateAllGifs(float deltaTime) {
	for (auto& kv : g_gifs) {
		GifData& g = kv.second;
		if (!g.loaded) continue;
		if (!g.visible)continue;

		g.timer_ms += deltaTime * 1000.f;
		while (g.timer_ms >= g.delays[g.current]) {
			g.timer_ms -= (float)g.delays[g.current];
			g.current = (g.current + 1) % g.frames.size();
		}

		g.visible = false;
	}
}

ID3D11ShaderResourceView* GIF::GetGifSRV(const std::string& name) {
	auto it = g_gifs.find(name);
	if (it == g_gifs.end()) return nullptr;
	GifData& g = it->second;
	if (g.frames.empty()) return nullptr;

	g.visible = true;
	return g.frames[g.current].Get();
}

void GIF::UnloadAllGifs() {
	g_gifs.clear();
}

//播放视频
bool CreateVideoTexture(ID3D11Device* device, UINT w, UINT h,
	ID3D11Texture2D** out_tex, ID3D11ShaderResourceView** out_srv) {
	D3D11_TEXTURE2D_DESC td{};
	td.Width = w; td.Height = h;
	td.MipLevels = 1; td.ArraySize = 1;
	td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(device->CreateTexture2D(&td, nullptr, out_tex))) return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
	sd.Format = td.Format;
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MipLevels = 1;
	return SUCCEEDED(device->CreateShaderResourceView(*out_tex, &sd, out_srv));
}

void DecoderThread(VideoData* v){
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	bool firstFrame = true;
	bool wasInactive = false;
	auto t0 = std::chrono::steady_clock::now();
	double nextTime = 0.0;

	while (!v->stop.load()){
		if (!v->active.load()){
			wasInactive = true;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		else if (wasInactive){
			t0 = std::chrono::steady_clock::now();
			nextTime = 0.0;
			wasInactive = false;
			firstFrame = true;
		}

		auto now = std::chrono::steady_clock::now();
		double elapsed = std::chrono::duration<double>(now - t0).count();

		if (elapsed < nextTime){
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		DWORD si = 0, flags = 0; LONGLONG ts = 0;
		ComPtr<IMFSample> smp;
		if (FAILED(v->reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &si, &flags, &ts, &smp)))
			continue;

		if (flags & MF_SOURCE_READERF_ENDOFSTREAM){
			PROPVARIANT var; PropVariantInit(&var);
			var.vt = VT_I8; var.hVal.QuadPart = 0;
			v->reader->SetCurrentPosition(GUID_NULL, var);
			PropVariantClear(&var);
			v->eof = true;
			firstFrame = true;
			continue;
		}

		if (!smp) continue;

		LONGLONG dur100 = 0;
		double durSec = v->frameDur;
		if (SUCCEEDED(smp->GetSampleDuration(&dur100)) && dur100 > 0)
			durSec = dur100 * 1e-7;

		ComPtr<IMFMediaBuffer> buf;
		smp->ConvertToContiguousBuffer(&buf);
		BYTE* data = nullptr; DWORD max = 0, cur = 0;
		buf->Lock(&data, &max, &cur);

		size_t need = (size_t)v->width * v->height * 4;
		if (cur >= need){
			std::vector<uint8_t> frame(need);
			memcpy(frame.data(), data, need);
			for (size_t i = 3; i < need; i += 4) frame[i] = 255;
			std::lock_guard<std::mutex> lk(v->mtx);
			if (v->queue.size() >= v->queue_cap) v->queue.pop_front();
			v->queue.emplace_back(std::move(frame));
		}
		buf->Unlock();

		if (firstFrame){
			t0 = std::chrono::steady_clock::now();
			nextTime = durSec;
			firstFrame = false;
		}
		else{
			nextTime += durSec;
		}
	}

	CoUninitialize();
}

bool Videos::LoadVideoFromFile(const wchar_t* path, const std::string& name, ID3D11Device* device){
	if (!g_mf_init) {
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		MFStartup(MF_VERSION, MFSTARTUP_FULL);
		g_mf_init = true;
	}

	if (!device) return false;

	g_pd3dDevice = device;
	device->GetImmediateContext(&g_pd3dDeviceContext);

	auto v = std::make_shared<VideoData>();
	HRESULT hr;
	ComPtr<IMFAttributes> attrs;
	MFCreateAttributes(&attrs, 2);
	attrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, FALSE);
	attrs->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

	hr = MFCreateSourceReaderFromURL(path, attrs.Get(), &v->reader);
	if (FAILED(hr)) return false;

	ComPtr<IMFMediaType> typeOut;
	MFCreateMediaType(&typeOut);
	typeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	typeOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	v->reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, typeOut.Get());

	ComPtr<IMFMediaType> typeActual;
	v->reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &typeActual);
	MFGetAttributeSize(typeActual.Get(), MF_MT_FRAME_SIZE, &v->width, &v->height);

	// stride
	LONG stride = 0; UINT32 sTmp = 0;
	if (SUCCEEDED(typeActual->GetUINT32(MF_MT_DEFAULT_STRIDE, &sTmp)))
		stride = (LONG)sTmp;
	else
		MFGetStrideForBitmapInfoHeader(MFVideoFormat_RGB32.Data1, v->width, &stride);
	v->stride = abs(stride);

	UINT32 frNum = 0, frDen = 0;
	if (SUCCEEDED(MFGetAttributeRatio(typeActual.Get(), MF_MT_FRAME_RATE, &frNum, &frDen)) && frNum && frDen)
		v->frameDur = (double)frDen / (double)frNum;
	else
		v->frameDur = 1.0 / 30.0; // 默认30FPS

	CreateVideoTexture(device, v->width, v->height, &v->tex, &v->srv);

	ComPtr<ID3D11DeviceContext> ctx;
	device->GetImmediateContext(&ctx);

	DWORD si = 0, flags = 0; LONGLONG ts = 0;
	ComPtr<IMFSample> smp;
	hr = v->reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &si, &flags, &ts, &smp);
	if (SUCCEEDED(hr) && smp){
		ComPtr<IMFMediaBuffer> buf;
		smp->ConvertToContiguousBuffer(&buf);
		BYTE* data = nullptr; DWORD max = 0, cur = 0;
		buf->Lock(&data, &max, &cur);

		size_t need = (size_t)v->width * v->height * 4;
		if (cur >= need){
			ctx->UpdateSubresource(v->tex.Get(), 0, nullptr, data, v->width * 4, 0);

			{
				std::lock_guard<std::mutex> lk(v->mtx);
				v->queue.emplace_back(data, data + need);
			}
		}
		buf->Unlock();
	}

	v->loaded = true;
	v->active = true;
	v->lastActiveTime = std::chrono::steady_clock::now();
	v->stop = false;
	v->eof = false;

	g_videos[name] = v;

	v->th = std::thread(DecoderThread, v.get());

	v->th.detach();

	return true;
}

void Videos::UpdateAllVideos(ID3D11DeviceContext* ctx){
	for (auto& kv : g_videos) {
		auto v = kv.second;
		if (!v->loaded) continue;

		std::vector<uint8_t> frame;
		{
			std::lock_guard<std::mutex> lk(v->mtx);
			if (v->queue.empty()) continue;
			frame = std::move(v->queue.back());
			v->queue.clear();
		}

		if (!frame.empty()) {
			D3D11_BOX box{ 0,0,0, v->width, v->height, 1 };
			ctx->UpdateSubresource(v->tex.Get(), 0, &box, frame.data(), v->width * 4, 0);
		}

		v->active = false;
	}
}

ID3D11ShaderResourceView* Videos::GetVideoSRV(const std::string& name) {
	auto it = g_videos.find(name);
	if (it == g_videos.end()) return nullptr;
	auto v = it->second;
	v->active = true;
	v->lastActiveTime = std::chrono::steady_clock::now();
	return v->srv.Get();
}

void Videos::UnloadAllVideos() {
	for (auto& kv : g_videos) kv.second->stop = true;
	for (auto& kv : g_videos)
		if (kv.second->th.joinable()) kv.second->th.join();
	g_videos.clear();
	
	if (g_mf_init) {
		MFShutdown();
		CoUninitialize();
		g_mf_init = false;
	}
}