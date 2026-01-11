#include <winsock2.h>
#include <deque>
#include <ws2tcpip.h>
#include <regex>
#include "../D3D11/D3D11.h"

#pragma comment(lib, "ws2_32.lib")

SOCKET g_socket = INVALID_SOCKET;
std::thread recvThread;
std::mutex msgMutex;
std::deque<std::filesystem::path> messages;
std::atomic<int> onlineCount = 0;
bool running = true;
char inputBuf[256] = "";
std::string username = "Unauthorized";
bool isSelf = false;
bool b_initSocket = false;
bool Isconnect = false;
bool b_usercmemu = false;

//绘制用户界面
void DrawUserSettings(bool usercmemu) {
	struct AnimState {
		float x = -150.0f;
		bool active = false;
	};

	static AnimState anim;

	const float targetX = 10.0f;   // 打开时目标位置
	const float startX = -150.0f;  // 初始隐藏位置
	const float speed = 15.0f;     // 动画速度

	float destX = usercmemu ? targetX : startX;

	if (usercmemu || anim.x > startX) {
		anim.x += (destX - anim.x) * ImGui::GetIO().DeltaTime * speed;
		if (fabs(anim.x - destX) < 0.5f) anim.x = destX;

		float alpha = (anim.x - startX) / (targetX - startX);
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, alpha));

		ImGui::SetCursorPos(ImVec2(anim.x, 300));
		ImGui::BeginChild("musicmemu", ImVec2(200, 85), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

		ImGui::SetCursorPos(ImVec2(6, 6));
		ImGui::Image((ImTextureID)Img::Useravatar, ImVec2(45, 45));

		ImGui::SetCursorPos(ImVec2(57, 13));
		ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), username.c_str());

		ImGui::SetCursorPos(ImVec2(6, 60));
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), (char*)u8"授权时间:永久.");

		ImGui::EndChild();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		anim.active = true; // 保持动画状态
	}
	else {
		anim.active = false;
	}
}

//绘制界面UI
void Chat::DrawChatUI() {
	//服务器连接
	if (!b_initSocket) {
		if (ret == "验证成功" && std::string(banbena) == g_Version) {
			//std::thread t(InitSocket_);
			//t.detach();
		}

		b_initSocket = true;
	}

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));

	//状态
	ImGui::SetCursorPos(ImVec2(10, 0));
	ImGui::BeginChild("在线信息", ImVec2(615, 30), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
	{
		ImGui::SetCursorPos(ImVec2(8, 8));
		ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), (char*)u8"当前状态:");
		ImGui::SameLine();
		if (Isconnect) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), (char*)u8"服务器已连接");
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (char*)u8"服务器已断开");
		}

		ImGui::SetCursorPos(ImVec2(200, 8));
		ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), (char*)u8"当前在线:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), (char*)u8"%d", onlineCount.load());
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), (char*)u8"人");

		ImGui::SetCursorPos(ImVec2(551.5, 3.5));
		if (ImGui::Button((char*)u8"刷新", ImVec2(60, 23))) {
			b_initSocket = false;
			Render::DrawMessage((char*)u8"刷新成功,请勿再次操作!", 3.0f);
		}
	}
	ImGui::EndChild();

	// 显示消息
	ImGui::SetCursorPos(ImVec2(10, 35));
	ImGui::BeginChild("消息", ImVec2(615, 350), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	std::lock_guard<std::mutex> lock(msgMutex);
	for (auto& msg : messages){
		auto u8 = msg.u8string();
		std::string utf8(u8.begin(), u8.end());

		// 提取用户名
		size_t start = utf8.find('[');
		size_t sep = utf8.find('|', start);
		size_t end = utf8.find(']', sep);

		std::string senderName;
		if (start != std::string::npos && sep != std::string::npos && end != std::string::npos){
			senderName = utf8.substr(sep + 1, end - sep - 1);
		}

		// 提取消息内容
		std::string pureMsg = (end != std::string::npos) ? utf8.substr(end + 1) : utf8;
		bool isSelf = (senderName == username);	//判断是否是自己发送的消息

		ImGui::SetCursorPosX(10);
		DrawChatMessage(senderName, pureMsg, (ImTextureID)Img::Useravatar, isSelf);
	}

	if (!messages.empty()) ImGui::SetScrollHereY(1.0f);

	ImGui::EndChild();

	// 输入框
	ImGui::SetCursorPos(ImVec2(10, 390));
	ImGui::BeginChild("输入框", ImVec2(615, 55), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
	{
		ImGui::SetCursorPos(ImVec2(10, 10));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
		if (ImGui::ButtonExIMG("##User", ImVec2(35, 35), 0, (ImTextureID)Img::User)) b_usercmemu = !b_usercmemu;

		ImGui::PopStyleColor(3);

		ImGui::SetCursorPos(ImVec2(53, 10));
		if (Render::SimpleLeftInput("input", inputBuf, IM_ARRAYSIZE(inputBuf), 487, 35, (char*)u8"请输入内容...")) {
			if (strlen(inputBuf) > 0) {
				std::string gbkmsg = UTF8ToGBK(inputBuf);
				send(g_socket, gbkmsg.c_str(), (int)gbkmsg.size(), 0);

				inputBuf[0] = '\0'; //清空
				ImGui::SetKeyboardFocusHere(-1);
			}
		}

		ImGui::SetCursorPos(ImVec2(550, 10));
		if (ImGui::Button((char*)u8"发送", ImVec2(55, 35))) {
			if (strlen(inputBuf) > 0) {
				std::string gbkmsg = UTF8ToGBK(inputBuf);
				send(g_socket, gbkmsg.c_str(), (int)gbkmsg.size(), 0);

				inputBuf[0] = '\0';
			}
		}
	}

	ImGui::EndChild();
	ImGui::PopStyleColor();

	DrawUserSettings(b_usercmemu);
}
