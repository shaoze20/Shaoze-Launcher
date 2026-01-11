#include "../D3D11/D3D11.h"

bool Render::CircleButton(const char* str_id, float radius, ImU32 color, ImU32 color_hover, bool drawX) {
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);

	ImGui::InvisibleButton(str_id, ImVec2(radius * 2, radius * 2));

	bool hovered = ImGui::IsItemHovered();
	bool clicked = ImGui::IsItemClicked();

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddCircleFilled(center, radius, hovered ? color_hover : color, 64);

	if (drawX) {
		float crossHalf = radius * 0.5f;
		float thickness = 1.0f;
		ImU32 crossColor = IM_COL32(0, 0, 0, 255);

		center.x = IM_FLOOR(center.x) + 0.8f;
		center.y = IM_FLOOR(center.y) + 0.5f;

		draw_list->AddLine(
			ImVec2(center.x - crossHalf, center.y - crossHalf),
			ImVec2(center.x + crossHalf, center.y + crossHalf),
			crossColor, thickness);

		draw_list->AddLine(
			ImVec2(center.x - crossHalf, center.y + crossHalf),
			ImVec2(center.x + crossHalf, center.y - crossHalf),
			crossColor, thickness);
	}

	return clicked;
}

void Render::Textwrapped(const char* text, ImVec2 size) {
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (!window) return;

	struct TooltipState {
		float timer = 0.0f;
		bool hoveredLast = false;
		bool fadingOut = false;
	};

	static std::unordered_map<ImGuiID, TooltipState> stateMap;

	ImGuiID id = ImGui::GetItemID();
	TooltipState& st = stateMap[id];

	ImVec2 itemMin = ImGui::GetItemRectMin();
	ImVec2 itemMax = ImGui::GetItemRectMax();
	ImVec2 mousePos = ImGui::GetIO().MousePos;
	float dt = ImGui::GetIO().DeltaTime;

	bool hoveredNow = (mousePos.x >= itemMin.x && mousePos.x <= itemMax.x && mousePos.y >= itemMin.y && mousePos.y <= itemMax.y);

	const float duration = 0.20f;

	if (hoveredNow) {
		if (!st.hoveredLast)st.fadingOut = false;

		st.timer += dt;
		if (st.timer > duration) st.timer = duration;
	}
	else {
		if (st.hoveredLast)st.fadingOut = true;
		if (st.fadingOut) {
			st.timer -= dt;
			if (st.timer < 0.0f) st.timer = 0.0f;
		}
	}

	st.hoveredLast = hoveredNow;

	if (st.timer <= 0.0f)return;

	float t = st.timer / duration;
	float easeOut = 1.0f - powf(1.0f - t, 3.0f); // 缓出曲线
	float scale = 0.8f + 0.2f * easeOut;        // 从 0.8 → 1.0 缩放
	float alpha = easeOut;                      // 从 0 → 1 透明度
	float slideOffset = (1.0f - easeOut) * 10.0f; // 从右滑入 10px

	ImVec2 itemPos = itemMin;
	ImVec2 itemSize = ImGui::GetItemRectSize();
	const float offsetRight = 6.0f;

	ImVec2 tipPos = ImVec2(itemPos.x + itemSize.x + offsetRight + slideOffset + 5.0f,
		itemPos.y + (itemSize.y - size.y) * 0.5f);

	// 缩放中心
	ImVec2 center = ImVec2(tipPos.x + size.x * 0.5f, tipPos.y + size.y * 0.5f);
	ImVec2 scaledSize = ImVec2(size.x * scale, size.y * scale);
	ImVec2 scaledPos = ImVec2(center.x - scaledSize.x * 0.5f, center.y - scaledSize.y * 0.5f);

	// 背景
	ImU32 bgColor = IM_COL32(255, 255, 255, (int)(255 * alpha));
	draw_list->AddRectFilled(scaledPos,
		ImVec2(scaledPos.x + scaledSize.x, scaledPos.y + scaledSize.y),
		bgColor, 3.0f);

	// 小三角形
	float tri = 5.0f;
	float midY = scaledPos.y + scaledSize.y * 0.5f;
	ImVec2 p0(scaledPos.x, midY - tri);
	ImVec2 p1(scaledPos.x, midY + tri);
	ImVec2 p2(scaledPos.x - tri, midY);
	draw_list->AddTriangleFilled(p0, p1, p2, bgColor);

	// 文字
	ImVec2 textPos = ImVec2(scaledPos.x + 3.0f, scaledPos.y + 3.0f);
	draw_list->AddText(textPos, IM_COL32(0, 0, 0, (int)(255 * alpha)), text);
}

void Render::Announcement(const char* label, ImVec2 size, bool& Switch) {
	static float t = 0.0f;
	static bool isClosing = false;

	float dt = ImGui::GetIO().DeltaTime;
	float scale = isClosing ? t : BounceEaseOut(t);

	// 动画进度
	if (isClosing) {
		t -= dt * 3.0f;
		if (t < 0.0f) {
			t = 0.0f;
			Switch = false;
			isClosing = false;
		}
	}
	else {
		t += dt;
		if (t > 1.0f) t = 1.0f;
	}

	ImVec4 bgColor = ImVec4(1.0f, 1.0f, 1.0f, 0.8f * scale);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);

	ImGui::SetCursorPos(size);
	ImGui::BeginChild("Small Window", ImVec2(260 * scale, 150 * scale), false, ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	ImGui::SetCursorPos(ImVec2(240, 2));
	if (CircleButton("Content", 8.0f, IM_COL32(255, 0, 0, 0), IM_COL32(255, 100, 100, 0),true)) {
		isClosing = true;
	}

	// 正文
	ImGui::SetCursorPos(ImVec2(7, 7));
	ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, t), label);

	ImGui::EndChild();
	ImGui::PopStyleColor();
}

bool Render::RoundCheckboxStyled(const char* label, bool* v, float radius) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)return false;

	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;

	ImVec2 pos = ImGui::GetCursorScreenPos();
	float diameter = radius * 2.0f;

	ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 total_size = ImVec2(diameter + style.ItemInnerSpacing.x + label_size.x, ImMax(diameter, label_size.y));

	ImRect bb(pos, ImVec2(pos.x + total_size.x, pos.y + total_size.y));
	ImGui::ItemSize(bb);

	ImGuiID id = ImGui::GetID(label);
	if (!ImGui::ItemAdd(bb, id))return false;

	bool hovered = false, held = false;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
	if (pressed)*v = !*v;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 center = ImVec2(pos.x + radius, pos.y + total_size.y * 0.5f);

	// -------- 样式定义 --------
	ImU32 col_border = IM_COL32(160, 160, 160, 255);  // 灰色边框
	ImU32 col_fill = IM_COL32(0, 122, 255, 255);      // 蓝色填充
	ImU32 col_check = IM_COL32(255, 255, 255, 255);   // 白色勾
	ImU32 col_text = IM_COL32(120, 120, 120, 255);    // 灰色文字

	float border_thickness = 1.0f;// 边框更细
	float check_thickness = 1.0f; // 勾更细

	if (*v) {
		draw_list->AddCircleFilled(center, radius, col_fill, 64);
	}
	else {
		draw_list->AddCircle(center, radius, col_border, 64, border_thickness);
	}

	if (*v) {
		ImVec2 start(center.x - radius * 0.45f, center.y - radius * 0.05f);
		ImVec2 mid(center.x - radius * 0.1f, center.y + radius * 0.35f);
		ImVec2 end(center.x + radius * 0.45f, center.y - radius * 0.45f);

		draw_list->AddLine(start, mid, col_check, check_thickness);
		draw_list->AddLine(mid, end, col_check, check_thickness);
	}

	ImVec2 text_pos = ImVec2(pos.x + diameter + style.ItemInnerSpacing.x, pos.y + (total_size.y - label_size.y) * 0.5f); draw_list->AddText(text_pos, col_text, label);
	return pressed;
}

void Render::DrawMessage(const std::string& msg, float duration, int type) {
	struct MsgState {
		bool active = false;
		std::string text;
		float duration = 0.0f;
		int type = 0;
		std::chrono::steady_clock::time_point startTime;
	};
	static MsgState state;

	if (!msg.empty() && duration > 0.0f) {
		state.text = msg;
		state.duration = duration;
		state.type = type;
		state.startTime = std::chrono::steady_clock::now();
		state.active = true;
	}

	if (!state.active) return;

	float elapsed = std::chrono::duration<float>(
		std::chrono::steady_clock::now() - state.startTime
	).count();

	if (elapsed > state.duration) {
		state.active = false;
		return;
	}

	// =============== 动画控制 ===============
	const float fadeTime = 0.3f;    // 淡入/淡出时长
	const float slideTime = 0.35f;  // 滑动动画时长
	float alpha = 1.0f;
	float slideOffset = 0.0f;

	if (elapsed < fadeTime)
		alpha = elapsed / fadeTime;
	else if (elapsed > state.duration - fadeTime)
		alpha = (state.duration - elapsed) / fadeTime;

	if (elapsed < slideTime) {
		float t = elapsed / slideTime;
		t = ImSaturate(sinf(t * IM_PI * 0.5f));
		slideOffset = (1.0f - t) * -40.0f;
	}
	else if (elapsed > state.duration - slideTime) {
		float t = (elapsed - (state.duration - slideTime)) / slideTime;
		t = ImSaturate(sinf(t * IM_PI * 0.5f));
		slideOffset = -t * 40.0f;
	}

	ImVec2 winPos = ImGui::GetWindowPos();
	ImVec2 winSize = ImGui::GetWindowSize();

	ImVec2 boxSize(150, 70);
	ImVec2 boxPos(
		winPos.x + (winSize.x - boxSize.x) / 2.0f,
		winPos.y + (winSize.y - boxSize.y) / 13.0f + slideOffset
	);

	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	drawList->AddRectFilled(boxPos,
		ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y),
		IM_COL32(255, 255, 255, (int)(alpha * 230)), 10.0f);
	drawList->AddRect(boxPos,
		ImVec2(boxPos.x + boxSize.x, boxPos.y + boxSize.y),
		IM_COL32(0, 0, 0, (int)(alpha * 100)), 10.0f);

	// 图标中心位置
	ImVec2 circleCenter(boxPos.x + boxSize.x / 2.0f, boxPos.y + 25.0f);
	float circleRadius = 12.0f;

	// ==================== 图标类型选择 ====================
	if (state.type == 0) {
		//绿色勾
		drawList->AddCircle(circleCenter, circleRadius,
			IM_COL32(0, 200, 0, (int)(alpha * 255)), 0, 2.0f);

		ImVec2 p1(circleCenter.x - 6.0f, circleCenter.y);
		ImVec2 p2(circleCenter.x - 1.0f, circleCenter.y + 4.0f);
		ImVec2 p3(circleCenter.x + 6.0f, circleCenter.y - 4.0f);
		drawList->AddLine(p1, p2, IM_COL32(0, 180, 0, (int)(alpha * 255)), 2.5f);
		drawList->AddLine(p2, p3, IM_COL32(0, 180, 0, (int)(alpha * 255)), 2.5f);
	}
	else if (state.type == 1) {
		//黄色感叹号
		drawList->AddCircle(circleCenter, circleRadius,
			IM_COL32(255, 210, 0, (int)(alpha * 255)), 0, 2.0f); // 边框

		ImVec2 pTop(circleCenter.x, circleCenter.y - 7.0f);
		ImVec2 pMid(circleCenter.x, circleCenter.y + 2.0f);
		drawList->AddLine(pTop, pMid, IM_COL32(200, 160, 0, (int)(alpha * 255)), 3.0f);

		ImVec2 dot(circleCenter.x + 0.5f, circleCenter.y + 6.0f);
		drawList->AddCircleFilled(dot, 1.5f, IM_COL32(200, 160, 0, (int)(alpha * 255)));
	}
	else if (state.type == 2) {
		//红色 X
		drawList->AddCircle(circleCenter, circleRadius,
			IM_COL32(220, 0, 0, (int)(alpha * 255)), 0, 2.0f); // 边框

		ImVec2 p1(circleCenter.x - 5.0f, circleCenter.y - 5.0f);
		ImVec2 p2(circleCenter.x + 5.0f, circleCenter.y + 5.0f);
		ImVec2 p3(circleCenter.x + 5.0f, circleCenter.y - 5.0f);
		ImVec2 p4(circleCenter.x - 5.0f, circleCenter.y + 5.0f);
		drawList->AddLine(p1, p2, IM_COL32(200, 0, 0, (int)(alpha * 255)), 2.5f);
		drawList->AddLine(p3, p4, IM_COL32(200, 0, 0, (int)(alpha * 255)), 2.5f);
	}

	// ==================== 文本 ====================
	ImVec2 textSize = ImGui::CalcTextSize(state.text.c_str());
	ImVec2 textPos(
		boxPos.x + (boxSize.x - textSize.x) / 2.0f,
		boxPos.y + (boxSize.y - textSize.y) / 2.0f + 15.0f
	);
	drawList->AddText(textPos, IM_COL32(0, 0, 0, (int)(alpha * 255)), state.text.c_str());
}

bool Render::SimpleCenteredInput(const char* id, char* buf, size_t buf_size, float width, float height, const char* placeholder) {
	ImGui::PushID(id);
	ImGui::SetNextItemWidth(width);

	ImGuiStyle& style = ImGui::GetStyle();
	float fontHeight = ImGui::GetFontSize();

	ImVec2 oldPadding = style.FramePadding;
	float oldRounding = style.FrameRounding;

	style.FramePadding.y = ImMax(0.0f, (height - fontHeight) * 0.5f);
	style.FrameRounding = 6.0f; // 控制圆角大小

	ImVec2 textSize = ImGui::CalcTextSize(buf);
	float textOffsetX = (width - textSize.x) * 0.5f;
	if (textOffsetX < 0.0f) textOffsetX = oldPadding.x;
	style.FramePadding.x = textOffsetX;

	bool changed = ImGui::InputText(("##" + std::string(id)).c_str(), buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue, NULL);

	if (buf[0] == '\0' && !ImGui::IsItemActive() && !ImGui::IsItemFocused()) {
		ImVec2 pos = ImGui::GetItemRectMin();
		ImVec2 size = ImGui::GetItemRectSize();

		ImVec2 placeholderSize = ImGui::CalcTextSize(placeholder);
		float textX = pos.x + (size.x - placeholderSize.x) * 0.5f;
		float textY = pos.y + (size.y - placeholderSize.y) * 0.5f;

		ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(textX, textY),
			ImGui::GetColorU32(ImGuiCol_TextDisabled), placeholder);
	}

	style.FramePadding = oldPadding;
	style.FrameRounding = oldRounding;

	ImGui::PopID();
	return changed;
}

bool Render::InputPassword(const char* id, char* buf, size_t buf_size, float width, float height, const char* placeholder) {
	ImGui::PushID(id);
	ImGui::SetNextItemWidth(width);

	ImGuiStyle& style = ImGui::GetStyle();
	float fontHeight = ImGui::GetFontSize();

	ImVec2 oldPadding = style.FramePadding;
	float oldRounding = style.FrameRounding;

	style.FramePadding.y = ImMax(0.0f, (height - fontHeight) * 0.5f);
	style.FrameRounding = 6.0f; // 控制圆角大小

	{
		std::string masked(buf);
		for (auto& c : masked) c = '*';
		ImVec2 textSize = ImGui::CalcTextSize(masked.c_str());

		float textOffsetX = (width - textSize.x) * 0.5f;
		if (textOffsetX < oldPadding.x)
			textOffsetX = oldPadding.x;

		style.FramePadding.x = textOffsetX;
	}

	bool changed = ImGui::InputText(("##" + std::string(id)).c_str(), buf, buf_size,
		ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_Password, NULL);

	if (buf[0] == '\0' && !ImGui::IsItemActive() && !ImGui::IsItemFocused()) {
		ImVec2 pos = ImGui::GetItemRectMin();
		ImVec2 size = ImGui::GetItemRectSize();

		ImVec2 placeholderSize = ImGui::CalcTextSize(placeholder);
		float textX = pos.x + (size.x - placeholderSize.x) * 0.5f;
		float textY = pos.y + (size.y - placeholderSize.y) * 0.5f;

		ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
			ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_TextDisabled), placeholder);
	}

	style.FramePadding = oldPadding;
	style.FrameRounding = oldRounding;

	ImGui::PopID();
	return changed;
}

bool Render::SimpleLeftInput(const char* id, char* buf, size_t buf_size, float width, float height, const char* placeholder) {
	ImGui::PushID(id);
	ImGui::SetNextItemWidth(width);

	ImGuiStyle& style = ImGui::GetStyle();
	float fontHeight = ImGui::GetFontSize();

	ImVec2 oldPadding = style.FramePadding;
	float oldRounding = style.FrameRounding;

	style.FramePadding.y = ImMax(0.0f, (height - fontHeight) * 0.5f);
	style.FramePadding.x = 8.0f;
	style.FrameRounding = 6.0f;

	bool changed = ImGui::InputText(("##" + std::string(id)).c_str(), buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue, NULL);

	if (buf[0] == '\0' && !ImGui::IsItemActive() && !ImGui::IsItemFocused()) {
		ImVec2 pos = ImGui::GetItemRectMin();
		ImVec2 size = ImGui::GetItemRectSize();

		ImVec2 placeholderSize = ImGui::CalcTextSize(placeholder);
		float textX = pos.x + style.FramePadding.x; // 靠左
		float textY = pos.y + (size.y - placeholderSize.y) * 0.5f; // 垂直居中

		ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
			ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_TextDisabled), placeholder);
	}

	style.FramePadding = oldPadding;
	style.FrameRounding = oldRounding;

	ImGui::PopID();
	return changed;
}

void Render::ShowVerifyResult(const std::filesystem::path& msg) {
	static bool needOpenPopup = true;
	static bool firstTime = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
	if (needOpenPopup) {
		ImGui::OpenPopup((char*)u8"温馨提示");
		needOpenPopup = false;
	}

	if (firstTime) {
		const int screen_width = GetSystemMetrics(SM_CXSCREEN);
		const int screen_height = GetSystemMetrics(SM_CYSCREEN);
		ImGui::SetNextWindowPos(ImVec2(screen_width / 2.2, screen_height / 2.2), ImGuiCond_Appearing);
		firstTime = false;
	}

	if (ImGui::BeginPopupModal((char*)u8"温馨提示", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextWrapped("%s", msg.u8string().c_str());
		ImGui::Spacing();

		if (ImGui::Button((char*)u8"确定", ImVec2(150, 0))) {
			ImGui::CloseCurrentPopup();
			InfoBox::bEnable = true;
			needOpenPopup = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

// 居中窗口
void Render::CenterWindowIfSizeChanged(const ImVec2& desired_size) {
	static ImVec2 g_last_window_size = ImVec2(0, 0);

	if (desired_size.x != g_last_window_size.x || desired_size.y != g_last_window_size.y) {
		const int screen_width = GetSystemMetrics(SM_CXSCREEN);
		const int screen_height = GetSystemMetrics(SM_CYSCREEN);

		ImGui::SetNextWindowPos(ImVec2((screen_width - desired_size.x) / 2.0f, (screen_height - desired_size.y) / 2.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(desired_size, ImGuiCond_Always);

		g_last_window_size = desired_size;
	}

	ImGui::SetNextWindowContentSize(desired_size);
}

// 信息样式
void Chat::DrawChatMessage(const std::string& username, const std::string& message_utf8, ImTextureID avatar_id, bool isSelf) {
	ImGui::PushID((username + message_utf8).c_str());
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 startPos = ImGui::GetCursorScreenPos();

	const float avatarSize = 36.0f;
	const float spacing = 10.0f;
	const float bubblePadding = 8.0f;
	const float textMaxWidth = 350.0f;
	const float lineSpacing = 10.0f;

	if (message_utf8.rfind("[SERVER_MESSAGE]", 0) == 0) {
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 windowSize = ImGui::GetWindowSize();

		std::string pureMsg = message_utf8.substr(16);//截取
		ImVec2 textSize = ImGui::CalcTextSize(pureMsg.c_str());

		ImVec2 centerPos = ImVec2(windowPos.x + (windowSize.x - textSize.x) * 0.5f, startPos.y + 5);

		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float bubblePadding = 6.0f;
		ImVec2 bubbleMin = ImVec2(centerPos.x - bubblePadding, centerPos.y - bubblePadding + 4);
		ImVec2 bubbleMax = ImVec2(centerPos.x + textSize.x + bubblePadding, centerPos.y + textSize.y + bubblePadding - 2);
		draw_list->AddRectFilled(bubbleMin, bubbleMax, IM_COL32(255, 255, 255, 150), 8.0f); // 背景颜色 + 圆角
		draw_list->AddRect(bubbleMin, bubbleMax, IM_COL32(255, 255, 255, 150), 8.0f); // 边框

		ImGui::SetCursorScreenPos(centerPos);
		ImGui::TextColored(ImVec4(0.3f, 0.3f, 0.3f, 1.0f), "%s", pureMsg.c_str());

		ImGui::SetCursorScreenPos(startPos);
		ImGui::Dummy(ImVec2(0.0f, textSize.y + bubblePadding * 2 + 10.0f));

		ImGui::PopID();
		return;
	}

	ImVec2 avatarPos = startPos;
	float contentStartX = startPos.x + avatarSize + spacing;

	float windowRight = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	if (isSelf) {
		avatarPos.x = windowRight - avatarSize - spacing;
		contentStartX = avatarPos.x - spacing - 0;
	}

	// === 绘制头像 ===
	ImVec2 avatarEnd = ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize);
	if (avatar_id)
		draw_list->AddImageRounded(avatar_id, avatarPos, avatarEnd, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE, 10.0f, ImDrawFlags_RoundCornersAll);
	else
		draw_list->AddRectFilled(avatarPos, avatarEnd, IM_COL32(120, 180, 255, 255), 6.0f);

	// === 用户名 ===
	ImVec2 namePos = ImVec2(contentStartX, startPos.y);
	ImGui::SetCursorScreenPos(namePos);
	if (isSelf) {
		float nameWidth = ImGui::CalcTextSize(username.c_str()).x;
		ImGui::SetCursorScreenPos(ImVec2(windowRight - avatarSize - spacing * 2 - nameWidth, startPos.y));
	}
	ImGui::TextColored(ImVec4(0, 0, 0, 0.5f), "%s", username.c_str());


	ImVec2 bubbleMin = ImVec2(contentStartX, namePos.y + ImGui::GetTextLineHeightWithSpacing());
	ImVec2 textSize = ImGui::CalcTextSize(message_utf8.c_str(), nullptr, false, textMaxWidth);
	ImVec2 bubbleMax = ImVec2(
		bubbleMin.x + textSize.x + bubblePadding * 2,
		bubbleMin.y + textSize.y + bubblePadding * 2
	);

	if (isSelf) {
		float bubbleWidth = bubbleMax.x - bubbleMin.x;
		bubbleMax.x = avatarPos.x - spacing;
		bubbleMin.x = bubbleMax.x - bubbleWidth;
	}

	ImU32 bubbleColor = isSelf ? IM_COL32(255, 170, 200, 150) : IM_COL32(255, 255, 255, 150);
	ImU32 borderColor = isSelf ? IM_COL32(255, 170, 200, 150) : IM_COL32(255, 255, 255, 150);

	draw_list->AddRectFilled(bubbleMin, bubbleMax, bubbleColor, 10.0f);
	draw_list->AddRect(bubbleMin, bubbleMax, borderColor, 10.0f);

	ImVec2 textPos = ImVec2(bubbleMin.x + bubblePadding, bubbleMin.y + bubblePadding);
	ImGui::SetCursorScreenPos(textPos);
	ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + textMaxWidth);
	ImGui::TextUnformatted(message_utf8.c_str());
	ImGui::PopTextWrapPos();

	float totalHeight = (bubbleMax.y + lineSpacing) - startPos.y;
	ImGui::SetCursorScreenPos(startPos);
	ImGui::Dummy(ImVec2(0.0f, totalHeight));

	ImGui::PopID();
}

bool Music::DrawMusicItem(const char* id, const std::string& label, bool selected, float width, float height, float rounding, ID3D11ShaderResourceView* textureSRV) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return false;

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size(width, height);
	ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

	ImGuiID widgetId = ImGui::GetID(id);

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, widgetId)) return false;

	bool hovered, held;
	bool clicked = ImGui::ButtonBehavior(bb, widgetId, &hovered, &held);

	// 固定底色，用透明度和选中状态区分
	ImU32 bg_col;
	if (selected) {
		// 选中状态：深蓝色实心
		bg_col = IM_COL32(30, 144, 255, 100);
	}
	else if (hovered) {
		// hover 状态：半透明
		bg_col = IM_COL32(30, 144, 255, 200);
	}
	else {
		// 普通状态：透明
		bg_col = IM_COL32(70, 130, 180, 0);
	}

	window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_col, rounding);

	// 图片参数
	float imageWidth = 25.0f;
	float imageHeight = 25.0f;
	float roundImage = 5.0f;

	ImVec2 imgPos = ImVec2(pos.x + 5, pos.y + (size.y - imageHeight) * 0.5f);
	ImVec2 imgEnd = ImVec2(imgPos.x + imageWidth, imgPos.y + imageHeight);

	if (textureSRV) {
		window->DrawList->AddImageRounded(
			(ImTextureID)textureSRV,
			imgPos,
			imgEnd,
			ImVec2(0, 0), ImVec2(1, 1),
			IM_COL32_WHITE,
			roundImage,
			ImDrawFlags_RoundCornersAll
		);
	}
	else {
		window->DrawList->AddImageRounded(
			(ImTextureID)Img::unknownmusic,
			imgPos,
			imgEnd,
			ImVec2(0, 0), ImVec2(1, 1),
			IM_COL32_WHITE,
			roundImage,
			ImDrawFlags_RoundCornersAll
		);
	}

	// 绘制文本
	ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
	ImVec2 textPos(pos.x + 10 + imageWidth, pos.y + (size.y - textSize.y) * 0.5f);
	window->DrawList->AddText(textPos, IM_COL32(0, 0, 0, 255), label.c_str());

	static ImGuiID lastClickedId = 0;
	static bool firstClick = false;
	bool pressed = false;

	if (clicked) {
		if (firstClick && lastClickedId == widgetId) {
			pressed = true;
			firstClick = false;
		}
		else {
			firstClick = true;
			lastClickedId = widgetId;
		}
	}

	return pressed;
}