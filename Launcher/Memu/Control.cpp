#include "../D3D11/D3D11.h"
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")

HardwareInfo hw;

class WmiHelper {
public:
	IWbemServices* pSvc = nullptr;

	WmiHelper() {
		HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hres)) return;

		hres = CoInitializeSecurity(
			NULL, -1, NULL, NULL,
			RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL, EOAC_NONE, NULL
		);
		if (FAILED(hres) && hres != RPC_E_TOO_LATE) return;

		IWbemLocator* pLoc = NULL;
		hres = CoCreateInstance(
			CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
			IID_IWbemLocator, (LPVOID*)&pLoc
		);
		if (FAILED(hres)) return;

		hres = pLoc->ConnectServer(
			_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc
		);
		pLoc->Release();
		if (FAILED(hres)) return;

		CoSetProxyBlanket(
			pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
			RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL, EOAC_NONE
		);
	}

	~WmiHelper() {
		if (pSvc) pSvc->Release();
		CoUninitialize();
	}

	std::vector<std::wstring> Query(const wchar_t* wqlClass, const wchar_t* property) {
		std::vector<std::wstring> result;
		if (!pSvc) return result;

		std::wstring query = L"SELECT * FROM ";
		query += wqlClass;

		IEnumWbemClassObject* pEnumerator = NULL;
		HRESULT hres = pSvc->ExecQuery(
			bstr_t("WQL"), bstr_t(query.c_str()),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			NULL, &pEnumerator
		);
		if (FAILED(hres)) return result;

		IWbemClassObject* pclsObj = NULL;
		ULONG uReturn = 0;
		while (pEnumerator) {
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if (0 == uReturn) break;

			VARIANT vtProp;
			hr = pclsObj->Get(property, 0, &vtProp, 0, 0);
			if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
				result.push_back(vtProp.bstrVal);

			VariantClear(&vtProp);
			pclsObj->Release();
		}
		pEnumerator->Release();
		return result;
	}
};

void RunSilent(const char* cmd) {
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	CreateProcessA("C:\\Windows\\System32\\cmd.exe",
		(LPSTR)(std::string("/c ") + cmd).c_str(),
		NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

static WmiHelper g_Wmi;
HardwareInfo GetHardwareInfo() {
	HardwareInfo info;

	auto cpu = g_Wmi.Query(L"Win32_Processor", L"Name");
	if (!cpu.empty()) info.CPU = cpu[0];

	auto gpus = g_Wmi.Query(L"Win32_VideoController", L"Name");
	if (!gpus.empty()) {
		std::wstring gpuCombined;
		for (size_t i = 0; i < gpus.size(); ++i) {
			gpuCombined += gpus[i];
			if (i + 1 < gpus.size()) gpuCombined += L"\n";
		}
		info.GPU = gpuCombined;
	}

	auto board = g_Wmi.Query(L"Win32_BaseBoard", L"Product");
	if (!board.empty()) info.Motherboard = board[0];

	auto mems = g_Wmi.Query(L"Win32_PhysicalMemory", L"Capacity");
	unsigned long long totalMem = 0;
	for (auto& m : mems) totalMem += std::stoull(m);
	info.RAM = std::to_wstring(totalMem / (1024 * 1024 * 1024)) + L" GB";

	auto disk = g_Wmi.Query(L"Win32_DiskDrive", L"Model");
	if (!disk.empty()) info.Disk = disk[0];

	auto os = g_Wmi.Query(L"Win32_OperatingSystem", L"Caption");
	if (!os.empty()) info.OS = os[0];

	return info;
}

//绘制面板UI
void Control::DrawControlUI() {
	ImGui::Indent(10.0f);
	ImGui::SetWindowFontScale(1.5f);
	ImGui::TextColored(ImVec4(0.0f, 0.6667f, 0.9725f, 1.0f), (char*)u8"硬件信息");
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Text((char*)u8"系统: %s", WStringToUTF8(hw.OS).c_str());
	ImGui::Text((char*)u8"处理器: %s", WStringToUTF8(hw.CPU).c_str());

	//GPU
	if (!hw.GPU.empty()) {
		size_t pos = hw.GPU.find(L'\n');
		if (pos != std::wstring::npos) {
			ImGui::Text((char*)u8"显卡: %s", WStringToUTF8(hw.GPU.substr(0, pos)).c_str());

			ImVec2 cursorPos = ImGui::GetCursorPos();
			float labelWidth = ImGui::CalcTextSize("GPU: ").x;

			std::wstring rest = hw.GPU.substr(pos + 1);
			size_t start = 0, end = 0;
			while ((end = rest.find(L'\n', start)) != std::wstring::npos) {
				ImGui::SetCursorPosX(labelWidth + 9);
				ImGui::Text("%s", WStringToUTF8(rest.substr(start, end - start)).c_str());
				start = end + 1;
			}
			if (start < rest.length()) {
				ImGui::SetCursorPosX(labelWidth + 9);
				ImGui::Text("%s", WStringToUTF8(rest.substr(start)).c_str());
			}
		}
		else {
			ImGui::Text("GPU: %s", WStringToUTF8(hw.GPU).c_str());
		}
	}

	ImGui::Text((char*)u8"内存: %s", WStringToUTF8(hw.RAM).c_str());
	ImGui::Text((char*)u8"主板: %s", WStringToUTF8(hw.Motherboard).c_str());
	ImGui::Text((char*)u8"硬盘: %s", WStringToUTF8(hw.Disk).c_str());

	ULONGLONG uptimeMS = GetTickCount64();
	int days = static_cast<int>(uptimeMS / (1000ull * 60 * 60 * 24));
	int hours = static_cast<int>((uptimeMS / (1000ull * 60 * 60)) % 24);
	int mins = static_cast<int>((uptimeMS / (1000ull * 60)) % 60);
	int secs = static_cast<int>((uptimeMS / 1000) % 60);
	ImGui::Text((char*)u8"系统运行时间: %d 天 %d 时 %d 分 %d 秒", days, hours, mins, secs);

	ImGui::SetWindowFontScale(1.5f);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::TextColored(ImVec4(0.0f, 0.6667f, 0.9725f, 1.0f), (char*)u8"系统功能");
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	if (ImGui::ButtonExIMG((char*)u8"修复桌面图标", ImVec2(95, 25),0,NULL)) {
		RunSilent("taskkill /f /im explorer.exe & cd /d %userprofile%\\AppData\\Local & del IconCache.db /a & start explorer.exe");
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"关闭自动更新", ImVec2(95, 25), 0, NULL)) {
		RunSilent("net stop wuauserv & sc config wuauserv start= disabled");
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"关闭安全中心", ImVec2(95, 25), 0, NULL)) {
		RunSilent("sc config wscsvc start= disabled");
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"关闭系统休眠", ImVec2(95, 25), 0, NULL)) {
		RunSilent("powercfg -h off");
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"清理临时文件", ImVec2(95, 25), 0, NULL)) {
		RunSilent("del /s /f /q %temp%\\*.* & rd /s /q %temp% & md %temp% & "
			"del /s /f /q C:\\Windows\\Temp\\*.* & rd /s /q C:\\Windows\\Temp & md C:\\Windows\\Temp");
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"系统激活工具", ImVec2(95, 25), 0, NULL)) {
		RunSilent(R"(powershell.exe -Command "irm https://get.activated.win | iex")");
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SetWindowFontScale(1.5f);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::TextColored(ImVec4(0.0f, 0.6667f, 0.9725f, 1.0f), (char*)u8"实用工具");
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	if (ImGui::ButtonExIMG((char*)u8"火绒安全", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://www.huorong.cn/person", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"卸载工具", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://www.hibitsoft.ir/Uninstaller.html", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"翻墙工具", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://github.com/Z-Siqi/Clash-for-Windows_Chinese-Attached", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"硬件检测", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open","https://www.tbtool.cn/", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"下载工具", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://neatdownload.com/neat-download-manager-v1-4/", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"Cheat Engine", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://www.cheatengine.org/", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	if (ImGui::ButtonExIMG((char*)u8"DX修复工具", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://www.zysoftware.top/post/9.html", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"VC运行库", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://github.com/abbodi1406/vcredist", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"解压软件", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://www.7-zip.org/", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"音频工具", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://vb-audio.com/Voicemeeter/index.htm", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"系统镜像", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://next.itellyou.cn/Identity/Account/Login?ReturnUrl=%2FOriginal%2FIndex", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"壁纸软件", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://wallhaven.cc/", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SetWindowFontScale(1.5f);
	ImGui::Dummy(ImVec2(0.0f, 10.0f));
	ImGui::TextColored(ImVec4(0.0f, 0.6667f, 0.9725f, 1.0f), (char*)u8"福利网站");
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Dummy(ImVec2(0.0f, 1.0f));
	if (ImGui::ButtonExIMG((char*)u8"福利网站一", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://mc.kurogames.com/main", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::SameLine();
	if (ImGui::ButtonExIMG((char*)u8"福利网站二", ImVec2(95, 25), 0, NULL)) {
		ShellExecuteA(NULL, "open", "https://ys.mihoyo.com/main/", NULL, NULL, SW_SHOWNORMAL);
		Render::DrawMessage((char*)u8"操作成功", 3.0f);
	}

	ImGui::Unindent();
} 