#include "../D3D11/D3D11.h"

ID3D11ShaderResourceView* beijing = nullptr;

bool showSmallTKA = false;
bool showSmallwuwa = false;
float alpha = 0.0f;

char TKA_pach[260] = "steam://rungameid/2254890";
char Wuwa_pach[260] = "";

void Render::Inject_Interface() {
	CenterWindowIfSizeChanged(ImVec2(720, 480));
	if (!ImGui::Begin("Shaoze##Inject", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))return;//绘制界面
	ImVec2 Windowpos = ImGui::GetWindowPos();
	ImVec2 Windowsize = ImGui::GetWindowSize();
	ImGuiStyle& Style = ImGui::GetStyle();
	static int current_tab = 0;
	auto Color = Style.Colors;
	float OveralHeight = 10.0f;
	Music::UpdatePlayer();

	if (!beijing)beijing = Img::Home_bj;
	ImVec2 winMax = ImVec2(Windowpos.x + Windowsize.x, Windowpos.y + Windowsize.y);
	ImGui::GetWindowDrawList()->AddImage((ImTextureID)beijing, Windowpos, winMax);//背景图

	ImGui::SetCursorPos(ImVec2(700, 3));
	if (CircleButton("exit_Injector",8.0f, IM_COL32(255, 0, 0, 255), IM_COL32(255, 100, 100, 255))) PostQuitMessage(0);

	ImGui::SetCursorPos(ImVec2(15.0f, 15.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
	ImGui::BeginChild((char*)u8"Option", { 60.0f, Windowsize.y - 30.0f }, false);
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 child_pos = ImGui::GetWindowPos();

		static bool first_init = true;
		static float bar_top = 5.0f;
		static float bar_bottom = 45.0f;
		float bar_height = 35.0f;   //窄边高度

		static float lastFrameTime = 0.0f;
		float currentFrameTime = ImGui::GetTime();
		float deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		float moveSpeed = 25.0f;    //跟随边速度
		float stretchSpeed = 15.0f; //先动边速度

		float target_top = 10.0f + current_tab * 50.0f;
		float target_bottom = target_top + bar_height;

		if (first_init) {
			bar_top = target_top;
			bar_bottom = target_bottom;
			first_init = false;
		}
		else {
			if (bar_top < target_top) {
				bar_top += (target_top - bar_top) * deltaTime * stretchSpeed;
				bar_bottom += (target_bottom - bar_bottom) * deltaTime * moveSpeed;
			}
			else if (bar_top > target_top) {
				bar_bottom += (target_bottom - bar_bottom) * deltaTime * stretchSpeed;
				bar_top += (target_top - bar_top) * deltaTime * moveSpeed;
			}
		}

		draw_list->AddRectFilled(
			ImVec2(child_pos.x + 1, (child_pos.y + 5) + bar_top),
			ImVec2(child_pos.x + 5, child_pos.y + bar_bottom),
			IM_COL32(238, 180, 180, 255),
			7.0f 
		);

		static float box_alpha = 1.0f;
		static int fading_from = -1;
		static int fading_to = 0;
		static bool is_fading = false;
		static int last_tab = current_tab;

		if (current_tab != last_tab && !is_fading) {
			fading_from = last_tab;
			fading_to = current_tab;
			is_fading = true;
		}
		last_tab = current_tab;

		if (is_fading) {
			box_alpha -= deltaTime * 10.0f;
			if (box_alpha <= 0.0f) {
				box_alpha = 0.0f;
				current_tab = fading_to;
				is_fading = false;
			}
		}
		else if (box_alpha < 1.0f) {
			box_alpha += deltaTime * 10.0f;
			if (box_alpha > 1.0f)
				box_alpha = 1.0f;
		}

		float OveralHeight = 10.0f;
		auto DrawTab = [&](const char* id, int tab_index, ImTextureID tex) {
			ImGui::SetCursorPos({ 10.0f, OveralHeight });
			ImVec2 cursor = ImGui::GetCursorScreenPos();
			ImVec2 size(40, 40);

			if ((tab_index == current_tab || (is_fading && tab_index == fading_from)) && box_alpha > 0.01f) {
				int alpha = static_cast<int>(box_alpha * 50.0f);
				draw_list->AddRectFilled(
					ImVec2(cursor.x - 4, cursor.y - 4),
					ImVec2(cursor.x + size.x + 4, cursor.y + size.y + 4),
					IM_COL32(255, 255, 255, alpha),
					7.0f
				);
			}

			if (ImGui::ButtonExIMG(id, size, 0, tex)) {
				if (!is_fading && tab_index != current_tab) {
					fading_from = current_tab;
					fading_to = tab_index;
					is_fading = true;
				}
			}

			OveralHeight += 50.0f;
			};

		DrawTab("##主页", 0, (ImTextureID)Img::Home);
		DrawTab("##聊天", 1, (ImTextureID)Img::messag);
		DrawTab("##面板", 2, (ImTextureID)Img::Panel);
		DrawTab("##音乐", 3, (ImTextureID)Img::music);
		DrawTab("##致命", 4, (ImTextureID)Img::TKA);
		DrawTab("##鸣潮", 5, (ImTextureID)Img::wuwa);
	}

	ImGui::EndChild();
	ImGui::PopStyleColor(4);

	ImGui::Dummy(ImVec2(0, 5.0f));
	ImGui::SetCursorPos(ImVec2(80.0f, 17));
	ImGui::BeginChild((char*)u8"Component", { Windowsize.x - 85.0f,Windowsize.y - 32.0f }, false);
	{
		switch (current_tab) {
		case 0://主页
		{
			auto homebeijing = Videos::GetVideoSRV("Home");
			if (homebeijing) {
				beijing = homebeijing;
			}
			else{
				beijing = Img::Home_bj;
			}

			DrawHomeUI();
			break;
		}

		case 1://聊天
		{
			beijing = Img::messag_bj;
			Chat::DrawChatUI();
			break;
		}

		case 2://面板
		{
			beijing = Img::Panel_bj;
			Control::DrawControlUI();
			break;
		}

		case 3://音乐
		{
			beijing = Img::Music_bj;
			Music::DrawPlayerUI();
			break;
		}

		case 4://致命解药
		{
			beijing = Img::TKA_bj;

			//说明
			ImVec4 bgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
			ImGui::SetCursorPos(ImVec2(10, 0));
			ImGui::BeginChild("Small Window", ImVec2(230, 120), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

			// 正文
			ImGui::SetCursorPos(ImVec2(7, 7));
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0), (char*)u8"说明:");
			ImGui::SetCursorPos(ImVec2(7, 25));
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0), (char*)u8"1.支持的游戏版本为：v0.6.3b.\n2.如果闪退请删除功能性的mod文件.\n3.Steam版使用默认的路径启动即可,\n   否则游戏无法加载mod.");

			ImGui::EndChild();
			ImGui::PopStyleColor();

			ImGui::SetCursorPos(ImVec2(578, 403));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
			if (ImGui::ButtonExIMG("##TKA", ImVec2(45, 45), 0, (ImTextureID)Img::Settings)) showSmallTKA = !showSmallTKA;
			ImGui::PopStyleColor(3);

			//Inject::ShowSmall("##致命解药路径", TKA_pach, sizeof(TKA_pach), (char*)u8"TKA_pach", showSmallTKA);

			//std::wstring dllpach = ExePath + Enc(L"\\Data\\TKA\\shaoze.dll");
			//Inject::DrawMemoryUI(L"TheKillingAntidote  ", TKA_pach, dllpach);

			break;
		}

		case 5://鸣潮
		{
			beijing = Img::wuwa_bj;

			//说明
			ImVec4 bgColor = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
			ImGui::SetCursorPos(ImVec2(10, 0));
			ImGui::BeginChild("Small Window", ImVec2(230, 120), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

			// 正文
			ImGui::SetCursorPos(ImVec2(7, 7));
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0), (char*)u8"说明:");
			ImGui::SetCursorPos(ImVec2(7, 25));
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0), (char*)u8"1.支持的游戏版本：v2.8.\n2.支持的服务器：国服官服.\n3.测试阶段,建议小号.");

			ImGui::EndChild();
			ImGui::PopStyleColor();

			ImGui::SetCursorPos(ImVec2(578, 403));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
			if (ImGui::ButtonExIMG("##鸣潮", ImVec2(45, 45), 0, (ImTextureID)Img::Settings))showSmallwuwa = !showSmallwuwa;
			ImGui::PopStyleColor(3);

			//Inject::ShowSmall("##鸣潮路径", Wuwa_pach, sizeof(Wuwa_pach), (char*)u8"Wuwa_pach", showSmallwuwa);

			//std::wstring dllpach = ExePath + Enc(L"\\Data\\Wuwa\\shaoze.dll");
			//Inject::DrawHijackUI(L"鸣潮  ", Wuwa_pach, dllpach);

			break;
		}
		}
	}

	DrawMessage();
	ImGui::EndChild();
	ImGui::End();
}