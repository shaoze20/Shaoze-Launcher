#include "../D3D11/D3D11.h"

bool showNotice = true;

DateTimeCN GetCurrentDateTimeCN() {
	std::time_t t = std::time(nullptr);
	std::tm localTime;
	localtime_s(&localTime, &t);

	// 星期
	const char* weekDays[] = { (char*)u8"日", (char*)u8"一",(char*)u8"二", (char*)u8"三", (char*)u8"四", (char*)u8"五", (char*)u8"六" };
	std::string weekStr = std::string((char*)u8"星期") + weekDays[localTime.tm_wday];

	std::string period;
	if (localTime.tm_hour >= 0 && localTime.tm_hour < 6)
		period = (char*)u8"凌晨";
	else if (localTime.tm_hour >= 6 && localTime.tm_hour < 12)
		period = (char*)u8"上午";
	else if (localTime.tm_hour == 12)
		period = (char*)u8"中午";
	else if (localTime.tm_hour > 12 && localTime.tm_hour < 18)
		period = (char*)u8"下午";
	else
		period = (char*)u8"晚上";

	// 时分秒（24 小时制）
	std::ostringstream timeOss;
	timeOss << period
		<< std::put_time(&localTime, "%H") << (char*)u8"时"
		<< std::put_time(&localTime, "%M") << (char*)u8"分"
		<< std::put_time(&localTime, "%S") << (char*)u8"秒";

	// 日期 年月日
	std::ostringstream dateOss;
	dateOss << std::put_time(&localTime, "%Y") << (char*)u8"年"
		<< std::put_time(&localTime, "%m") << (char*)u8"月"
		<< std::put_time(&localTime, "%d") << (char*)u8"日 "
		<< weekStr;

	return { timeOss.str(), dateOss.str() };
}

void Render::DrawHomeUI() {
	if (showNotice && InfoBox::bEnable) Announcement(gonggao, ImVec2(150, 130), showNotice);

	DateTimeCN dt = GetCurrentDateTimeCN();
	ImGui::SetCursorPos(ImVec2(10, 0));
	ImGui::PushFont(Font_Big);
	ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), "%s", dt.timeStr.c_str());
	ImGui::PopFont();

	ImGui::SetCursorPos(ImVec2(10, 20));
	ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.9f, 1.0f), "%s", dt.dateStr.c_str());

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
	ImGui::SetCursorPos(ImVec2(10, 410));
	if (ImGui::ButtonExIMG("##QQ", ImVec2(35, 35), 0, (ImTextureID)Img::qq)) {
		ShellExecuteA(NULL, "open", "https://qm.qq.com/q/OkQhJ9KxKs", NULL, NULL, SW_SHOWNORMAL);
		DrawMessage((char*)u8"欢迎加入群聊!", 3.0f);
	}
	Textwrapped((char*)u8"点我加入QQ群!", ImVec2(95, 20));

	ImGui::SetCursorPos(ImVec2(48, 410));
	if (ImGui::ButtonExIMG("##discord", ImVec2(35, 35), 0, (ImTextureID)Img::Discord)) {
		ShellExecuteA(NULL, "open", "https://discord.gg/9xztfDnC5k", NULL, NULL, SW_SHOWNORMAL);
		DrawMessage((char*)u8"欢迎加入频道!", 3.0f);
	}
	Textwrapped((char*)u8"点我加入频道!", ImVec2(95, 20));

	ImGui::SetCursorPos(ImVec2(90, 410));
	if (ImGui::ButtonExIMG("##Bilibili", ImVec2(35, 35), 0, (ImTextureID)Img::bilibili)) {
		ShellExecuteA(NULL, "open", "https://b23.tv/0noox5s", NULL, NULL, SW_SHOWNORMAL);
		DrawMessage((char*)u8"感谢关注!", 3.0f);
	}
	Textwrapped((char*)u8"点我进入作者B站!", ImVec2(110, 20));

	ImGui::SetCursorPos(ImVec2(130, 410));
	if (ImGui::ButtonExIMG("##afd", ImVec2(35, 35), 0, (ImTextureID)Img::afd)) {
		ShellExecuteA(NULL, "open", "https://afdian.com/a/shaoze", NULL, NULL, SW_SHOWNORMAL);
		DrawMessage((char*)u8"感谢赞助!", 3.0f);
	}
	Textwrapped((char*)u8"点我赞助作者!", ImVec2(90, 20));

	ImGui::SetCursorPos(ImVec2(170, 410));
	if (ImGui::ButtonExIMG((char*)u8"##github", ImVec2(35, 35), 0, (ImTextureID)Img::github)) {
		ShellExecuteA(NULL, "open", "https://github.com/shaoze20/Shaoze-Launcher", NULL, NULL, SW_SHOWNORMAL);
		DrawMessage((char*)u8"欢迎进入!", 3.0f);
	}
	Textwrapped((char*)u8"点我进入项目主页!", ImVec2(115, 20));
	ImGui::PopStyleColor(3);
}