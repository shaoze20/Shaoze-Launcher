#include "../D3D11/D3D11.h"

int Register = 0;
char g_Version[128] = "";

void Render::Login_Interface() {
    CenterWindowIfSizeChanged(ImVec2(640.0f, 400.0f));
    ImGui::PushFont(Font_Login);
	ImGui::Begin("Shaoze##Login", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);//开始绘制窗口

    auto beijing = Videos::GetVideoSRV("Login");
    if (beijing) {
        ImGui::Image((ImTextureID)beijing, ImVec2(640, 400));
    }
    else{
        ImGui::Image((ImTextureID)Img::Login_bj, ImVec2(640, 400));
    }

    switch (Register) {
    case 0: DrawLoginUI(); break;
    case 1: DrawRegisterUI(); break;
    case 2: DrawModifyUI(); break;
    }

    ImGui::SetCursorPos(ImVec2(610, 5));
    if (CircleButton("exit_Login", 10.0f, IM_COL32(255, 0, 0, 0), IM_COL32(255, 100, 100, 0), true)) PostQuitMessage(0);

    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 1.0f), g_Version);

	ImGui::End();
    ImGui::PopFont();
}