#pragma once
#include "../D3D11/D3D11.h"

using json = nlohmann::json;

extern int Tab, Color_, currentTrack, Register;
extern ImFont* Font_Big;
extern ImFont* Font_Login;
extern const char* gonggao;
extern const char* banbena;
extern bool b_volume;
extern std::wstring ExePath;
extern char TKA_pach[260], Wuwa_pach[260], g_Version[128];
extern std::string username, ret;
extern std::filesystem::path MusicPath;

struct DateTimeCN {
	std::string timeStr;
	std::string dateStr;
};

struct HardwareInfo {
	std::wstring CPU;
	std::wstring GPU;
	std::wstring Motherboard;
	std::wstring RAM;
	std::wstring Disk;
	std::wstring OS;
};
extern HardwareInfo hw;

class Render {
public:
	static bool CircleButton(const char* str_id, float radius, ImU32 color, ImU32 color_hover, bool drawX = false);
	static void Textwrapped(const char* text, ImVec2 size);
	static void Announcement(const char* label, ImVec2 size, bool& Switch);
	static bool RoundCheckboxStyled(const char* label, bool* v, float radius = 8.0f);
	static void DrawMessage(const std::string& msg = "", float duration = 0.0f, int type = 0);
	static bool SimpleCenteredInput(const char* id, char* buf, size_t buf_size, float width, float height, const char* placeholder);
	static bool SimpleLeftInput(const char* id, char* buf, size_t buf_size, float width, float height, const char* placeholder);
	static bool InputPassword(const char* id, char* buf, size_t buf_size, float width, float height, const char* placeholder);
	static void CenterWindowIfSizeChanged(const ImVec2& desired_size);
	static void ShowVerifyResult(const std::filesystem::path& msg);

public:
	static void DrawHomeUI();
	static void DrawLoginUI();
	static void DrawModifyUI();
	static void DrawRegisterUI();
	static void Inject_Interface();
	static void Login_Interface();
};

class Music {
public:
	static bool DrawMusicItem(const char* id, const std::string& label, bool selected, float width, float height, float rounding, ID3D11ShaderResourceView* textureSRV);

public:
	static void DrawPlayerUI();
	static void InitAudio();
	static void UpdatePlayer();
	static void ScanMusicFolder(const std::string& folder);
};

class Chat {
public:
	static void DrawChatUI();
	static void DrawChatMessage(const std::string& username, const std::string& message_utf8, ImTextureID avatar_id, bool isSelf);
};

class Control {
public:
	static void DrawControlUI();
};

class Json {
public:
	static bool SaveConfig(const std::string& ConfigName, const std::string& Exepach);
	static void LoadStringField(const json& j, const char* key, char* dest, size_t dest_size);
};

enum Tab {
	Panel,
	Button,
	Input,
	CheckBox
};

enum Color_ {
	Red,
	Green,
	Blue,
	Orange
};

namespace InfoBox {
	inline std::filesystem::path Information = "";
	inline bool bEnable = true;
}

namespace Img {
	inline ID3D11ShaderResourceView* TKA;
	inline ID3D11ShaderResourceView* wuwa;
	inline ID3D11ShaderResourceView* Home;
	inline ID3D11ShaderResourceView* Home_bj;
	inline ID3D11ShaderResourceView* wuwa_bj;
	inline ID3D11ShaderResourceView* TKA_bj;
	inline ID3D11ShaderResourceView* Settings;
	inline ID3D11ShaderResourceView* bilibili;
	inline ID3D11ShaderResourceView* qq;
	inline ID3D11ShaderResourceView* afd;
	inline ID3D11ShaderResourceView* Login;
	inline ID3D11ShaderResourceView* Pause;
	inline ID3D11ShaderResourceView* Np;
	inline ID3D11ShaderResourceView* Tp;
	inline ID3D11ShaderResourceView* Volume;
	inline ID3D11ShaderResourceView* circled;
	inline ID3D11ShaderResourceView* Login_bj;
	inline ID3D11ShaderResourceView* music;
	inline ID3D11ShaderResourceView* Music_bj;
	inline ID3D11ShaderResourceView* Panel;
	inline ID3D11ShaderResourceView* messag;
	inline ID3D11ShaderResourceView* messag_bj;
	inline ID3D11ShaderResourceView* Panel_bj;
	inline ID3D11ShaderResourceView* Useravatar;
	inline ID3D11ShaderResourceView* github;
	inline ID3D11ShaderResourceView* unknownmusic;
	inline ID3D11ShaderResourceView* Discord;
	inline ID3D11ShaderResourceView* User;
}

namespace Draw {
	inline bool CheckBox_1 = false;
	inline bool	CheckBox_2 = true;
}

namespace Functional {

}

namespace ColorByte {
	inline	ImVec4 zombies = ImVec4(0.9647f, 1.0f, 0.0118f, 1.0f);
	inline	ImVec4	Mash = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	inline	ImVec4	rays = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	inline	ImVec4	box = ImVec4(0.596f, 0.961f, 0.976f, 1.0f);
	inline	ImVec4	items = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	inline	ImVec4	debug = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	inline	ImVec4	Single = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

namespace IntInput {
	inline int InputInt = 0;
	inline int Comb = 0;
}

namespace FloatInput {
	inline float InputFloat = 0;
}

void StartConsole();
void CloseConsole();
std::string UTF8ToGBK(const std::string& utf8Str);
std::string GBKToUTF8(const std::string& gbkStr);
std::string WStringToUTF8(const std::wstring& wstr);
std::wstring UTF8ToWString(const std::string& utf8);
bool IsRunAsAdmin();
float BounceEaseOut(float t);
std::wstring GetExePath();
HardwareInfo GetHardwareInfo();