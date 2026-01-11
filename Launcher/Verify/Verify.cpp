#include "../D3D11/D3D11.h"

char Account[128] = "", Password[128] = "";
bool Agreement = false, iskey = false;
std::string g_Announcem_str, g_Version_str;

std::string ret;
void Render::DrawLoginUI() {
	ImGui::SetCursorPos(ImVec2(280, 60));
	ImGui::Image((ImTextureID)Img::Login, ImVec2(80, 80));

	ImGui::SetCursorPos(ImVec2(195, 170));
	SimpleCenteredInput("account", Account, sizeof(Account), 250.0f, 40.0f, (char*)u8"请输入账号");

	ImGui::SetCursorPos(ImVec2(195, 225));
	InputPassword("Password", Password, sizeof(Password), 250.0f, 40.0f, (char*)u8"请输入密码");

	ImGui::SetCursorPos(ImVec2(240, 280));;
	RoundCheckboxStyled((char*)u8"已阅读并同意服务协议", &Agreement);

	ImGui::SetCursorPos(ImVec2(195, 320));
	if (ImGui::Button((char*)u8"登录", ImVec2(250.0f, 35.0f))) {
		if (!Agreement) {
			InfoBox::Information = "请勾选服务协议!";
			InfoBox::bEnable = false;
			return;
		}

		bool Iskey = true;//验证
		if (Iskey) {
			//InfoBox::Information = Enc("登录成功!\n授权时间:永久.");
			//InfoBox::bEnable = false;

			Json::SaveConfig("User", Account);
			Json::SaveConfig("Password", Password);
			Json::SaveConfig("Agreement", Agreement ? "true" : "false");

			iskey = true;
			username = Account;
		}
		else {
			InfoBox::Information = ret;
			InfoBox::bEnable = false;
		}
	}

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.9f, 1.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.85f, 1.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.45f, 1.0f, 1.0f));
	ImGui::SetCursorPos(ImVec2(192, 360));
	if (ImGui::Button((char*)u8"服务协议", ImVec2(80.0f, 35.0f))) {
		std::wstring folderPath = ExePath + L"\\使用说明.txt";
		ShellExecuteW(nullptr, L"open", folderPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	}

	ImGui::SameLine();
	if (ImGui::Button((char*)u8"注册账号", ImVec2(80.0f, 35.0f)))Register = 1;

	ImGui::SameLine();
	if (ImGui::Button((char*)u8"修改密码", ImVec2(80.0f, 35.0f)))Register = 2;
	ImGui::PopStyleColor(4);
}

char RegisterUser[128] = "", RegisterPassword[128] = "", SuperPassword[128] = "";
void Render::DrawRegisterUI() {
	static float fadeAlpha = 0.0f;         // 当前透明度 (0~1)
	static bool fadingIn = true;           // 是否正在淡入
	static bool fadingOut = false;         // 是否正在淡出
	static float fadeSpeed = 2.0f;         // 淡入淡出速度
	static bool needReset = false;         // 是否需要重新初始化

	if (needReset) {
		fadeAlpha = 0.0f;
		fadingIn = true;
		fadingOut = false;
		needReset = false;
	}

	if (fadingIn && !fadingOut) {
		fadeAlpha += ImGui::GetIO().DeltaTime * fadeSpeed;
		if (fadeAlpha >= 1.0f) {
			fadeAlpha = 1.0f;
			fadingIn = false;
		}
	}
	else if (fadingOut) {
		fadeAlpha -= ImGui::GetIO().DeltaTime * fadeSpeed;
		if (fadeAlpha <= 0.0f) {
			fadeAlpha = 0.0f;
			fadingOut = false;

			needReset = true;
			Register = 0;

			//清除输入内容
			RegisterUser[0] = '\0';
			RegisterPassword[0] = '\0';
			SuperPassword[0] = '\0';
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImClamp(fadeAlpha, 0.001f, 1.0f));
	ImGui::SetNextWindowBgAlpha(ImClamp(fadeAlpha, 0.001f, 1.0f));

	// ---- 绘制 UI 内容 ----
	ImGui::PushFont(Font_Big);
	ImGui::SetCursorPos(ImVec2(280, 50));
	ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), (char*)u8"注册界面");
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(195, 100));
	SimpleCenteredInput("RegisterUser", RegisterUser, sizeof(RegisterUser), 250.0f, 40.0f, (char*)u8"请输入账号");

	ImGui::SetCursorPos(ImVec2(195, 160));
	SimpleCenteredInput("RegisterPassword", RegisterPassword, sizeof(RegisterPassword), 250.0f, 40.0f, (char*)u8"请输入密码");

	ImGui::SetCursorPos(ImVec2(195, 215));
	SimpleCenteredInput("SuperPassword", SuperPassword, sizeof(SuperPassword), 250.0f, 40.0f, (char*)u8"请输入安全码(修改密码用!)");

	ImGui::SetCursorPos(ImVec2(195, 270));
	if (ImGui::Button((char*)u8"注册账号", ImVec2(250.0f, 35.0f))) {

	}

	ImGui::SetCursorPos(ImVec2(195, 320));
	if (ImGui::Button((char*)u8"返回登录", ImVec2(250.0f, 35.0f))) {
		fadingOut = true;
		fadingIn = false;
	}

	ImGui::PopStyleVar();
}

char ModifyUser[128] = "", SafetyPassword[128] = "", ModifyPassword[128] = "";
void Render::DrawModifyUI() {
	static float fadeAlpha = 0.0f;         // 当前透明度 (0~1)
	static bool fadingIn = true;           // 是否正在淡入
	static bool fadingOut = false;         // 是否正在淡出
	static float fadeSpeed = 2.0f;         // 淡入淡出速度
	static bool needReset = false;         // 是否需要重新初始化

	if (needReset) {
		fadeAlpha = 0.0f;
		fadingIn = true;
		fadingOut = false;
		needReset = false;
	}

	if (fadingIn && !fadingOut) {
		fadeAlpha += ImGui::GetIO().DeltaTime * fadeSpeed;
		if (fadeAlpha >= 1.0f) {
			fadeAlpha = 1.0f;
			fadingIn = false;
		}
	}
	else if (fadingOut) {
		fadeAlpha -= ImGui::GetIO().DeltaTime * fadeSpeed;
		if (fadeAlpha <= 0.0f) {
			fadeAlpha = 0.0f;
			fadingOut = false;

			needReset = true;
			Register = 0;

			//清除输入内容
			ModifyUser[0] = '\0';
			SafetyPassword[0] = '\0';
			ModifyPassword[0] = '\0';
		}
	}

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImClamp(fadeAlpha, 0.001f, 1.0f));
	ImGui::SetNextWindowBgAlpha(ImClamp(fadeAlpha, 0.001f, 1.0f));

	// ---- 绘制 UI 内容 ----
	ImGui::PushFont(Font_Big);
	ImGui::SetCursorPos(ImVec2(280, 50));
	ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), (char*)u8"修改密码");
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(195, 100));
	SimpleCenteredInput("ModifyUser", ModifyUser, sizeof(ModifyUser), 250.0f, 40.0f, (char*)u8"请输入账号");

	ImGui::SetCursorPos(ImVec2(195, 160));
	SimpleCenteredInput("SafetyPassword", SafetyPassword, sizeof(SafetyPassword), 250.0f, 40.0f, (char*)u8"请输入安全码或原密码");

	ImGui::SetCursorPos(ImVec2(195, 215));
	SimpleCenteredInput("ModifyPassword", ModifyPassword, sizeof(ModifyPassword), 250.0f, 40.0f, (char*)u8"请输入新密码");

	ImGui::SetCursorPos(ImVec2(195, 270));
	if (ImGui::Button((char*)u8"修改密码", ImVec2(250.0f, 35.0f))) {
		InfoBox::Information = ret;
		InfoBox::bEnable = false;
	}

	ImGui::SetCursorPos(ImVec2(195, 320));
	if (ImGui::Button((char*)u8"返回登录", ImVec2(250.0f, 35.0f))) {
		fadingOut = true;
		fadingIn = false;
	}

	ImGui::PopStyleVar();
}