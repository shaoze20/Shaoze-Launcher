#include "../D3D11/D3D11.h"

static float g_spawnAccumulator = 0.0f;
static std::chrono::high_resolution_clock::time_point g_lastTime = std::chrono::high_resolution_clock::now();

void StartConsole() {
	if (!GetConsoleWindow()) {
		AllocConsole();
		FILE* f;
		freopen_s(&f, "CONOUT$", "w", stdout);
		freopen_s(&f, "CONOUT$", "w", stderr);
		freopen_s(&f, "CONIN$", "r", stdin);
	}
}

void CloseConsole() {
	HWND hwndConsole = GetConsoleWindow();

	if (hwndConsole != NULL) {
		FreeConsole();
		PostMessage(hwndConsole, WM_CLOSE, 0, 0);
	}
}

void Json::LoadStringField(const json& j, const char* key, char* dest, size_t dest_size) {
	if (j.contains(key) && j[key].is_string()) {
		std::string value = j[key];
		if (!value.empty()) {
			strncpy_s(dest, dest_size, value.c_str(), _TRUNCATE);
		}
	}
}

bool Json::SaveConfig(const std::string& ConfigName, const std::string& Exepach) {
	// 创建文件夹
	LPCSTR folderName = "C:\\Shaoze";
	if (!CreateDirectoryA(folderName, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
		InfoBox::Information = "创建文件夹失败! 错误码: " + std::to_string(GetLastError());
		InfoBox::bEnable = true;
		return false;
	}

	json j;
	std::ifstream infile("C:\\Shaoze\\Data.json");
	if (infile.is_open()) {
		try { infile >> j; }
		catch (...) { j = json::object(); }
		infile.close();
	}

	j[ConfigName] = Exepach;

	std::ofstream outfile("C:\\Shaoze\\Data.json");
	if (outfile.is_open()) {
		outfile << j.dump(4);
		outfile.close();
	}
	else {
		InfoBox::Information = "保存失败!无法写入配置文件!";
		InfoBox::bEnable = true;
		return false;
	}

	return true;
}

std::string UTF8ToGBK(const std::string& utf8Str) {
	int lenW = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
	std::wstring wstr(lenW, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], lenW);

	int lenA = WideCharToMultiByte(936, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string gbkStr(lenA, 0);
	WideCharToMultiByte(936, 0, wstr.c_str(), -1, &gbkStr[0], lenA, nullptr, nullptr);

	return gbkStr;
}

std::string GBKToUTF8(const std::string& gbkStr) {
	int lenW = MultiByteToWideChar(936, 0, gbkStr.c_str(), -1, nullptr, 0);
	std::wstring wstr(lenW, 0);
	MultiByteToWideChar(936, 0, gbkStr.c_str(), -1, &wstr[0], lenW);

	int lenU8 = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	std::string utf8Str(lenU8, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8Str[0], lenU8, nullptr, nullptr);

	return utf8Str;
}

std::string WStringToUTF8(const std::wstring& wstr) {
	if (wstr.empty()) return std::string();

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	std::string strTo(size_needed - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

std::wstring UTF8ToWString(const std::string& utf8){
	if (utf8.empty()) return L"";

	int wide_size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
	if (wide_size <= 0) return L"";

	std::wstring wide(wide_size, 0);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide[0], wide_size);

	if (!wide.empty() && wide.back() == L'\0')wide.pop_back();

	return wide;
}

bool IsRunAsAdmin() {
	BOOL fIsRunAsAdmin = FALSE;
	PSID pAdminGroup = NULL;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdminGroup)) {
		CheckTokenMembership(NULL, pAdminGroup, &fIsRunAsAdmin);
		FreeSid(pAdminGroup);
	}

	return fIsRunAsAdmin;
}

float BounceEaseOut(float t) {
	if (t < 4 / 11.0f) return (121 * t * t) / 16.0f;
	else if (t < 8 / 11.0f) return (363 / 40.0f * t * t) - (99 / 10.0f * t) + 17 / 5.0f;
	else if (t < 9 / 10.0f) return (4356 / 361.0f * t * t) - (35442 / 1805.0f * t) + 16061 / 1805.0f;
	else return (54 / 5.0f * t * t) - (513 / 25.0f * t) + 268 / 25.0f;
}

std::wstring GetExePath() {
	wchar_t path[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::wstring exePath(path);

	size_t pos = exePath.find_last_of(L"\\/");
	if (pos != std::wstring::npos)
		exePath = exePath.substr(0, pos);

	return exePath;
}