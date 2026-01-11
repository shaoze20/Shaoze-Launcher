#define MINIAUDIO_IMPLEMENTATION
#include "../Utils/miniaudio.h"
#include "../D3D11/D3D11.h"
#include <filesystem>
#include <shlobj.h>

std::filesystem::path MusicPath;

namespace fs = std::filesystem;

// 全局播放器变量
ma_engine engine;
ma_sound currentSound;
bool IsPlaying = false;
float volume = 0.1f;
std::vector<std::filesystem::path> Playlist;
int currentIndex = 0;
float currentSampleRate = 0.0f;
bool b_volume = false;
bool threadStarted = false;
bool b_musicmemu = false;

// 初始化音乐引擎
void Music::InitAudio() {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        MessageBoxA(NULL, "Failed to initialize audio engine", "Error", MB_OK);
    }
}

// 扫描文件夹,生成播放列表
void Music::ScanMusicFolder(const std::string& folder) {
    Playlist.clear();

    if (fs::exists(folder)) {
        for (auto& p : fs::directory_iterator(folder)) {
            if (p.is_regular_file()) {
                std::string ext = p.path().extension().string();
                if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac") {
                    Playlist.push_back(p.path().string());
                }
            }
        }
    }
}

void SelectMusicFolderThread() {
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"请选择音乐文件夹";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != 0) {
        char path[MAX_PATH];

        if (SHGetPathFromIDListA(pidl, path)) {
            Music::ScanMusicFolder(path);
            if (Playlist.empty()) {
                threadStarted = false;
            }
            else {
                currentIndex = 0;
                MusicPath = path;
            }
        }
        CoTaskMemFree(pidl);
    }
    else{
        threadStarted = false;
    }
}

// 播放当前索引的音乐
void PlayCurrent() {
    ma_sound_uninit(&currentSound);
    if (currentIndex < 0 || currentIndex >= Playlist.size()) return;
    ma_result result = ma_sound_init_from_file(&engine, Playlist[currentIndex].string().c_str(), 0, NULL, NULL, &currentSound);
    if (result == MA_SUCCESS) {
        ma_sound_set_volume(&currentSound, volume);
        ma_sound_start(&currentSound);
        IsPlaying = true;
    }
}

// 播放/暂停切换
void TogglePlayPause() {
    if (IsPlaying) {
        ma_sound_stop(&currentSound);
        IsPlaying = false;
    }
    else {
        ma_sound_start(&currentSound);
        IsPlaying = true;
    }
}

// 播放下一首
void PlayNext() {
    currentIndex = (currentIndex + 1) % Playlist.size();
    PlayCurrent();
}

// 播放上一首
void PlayPrev() {
    currentIndex = (currentIndex - 1 + Playlist.size()) % Playlist.size();
    PlayCurrent();
}
 
// 更新音乐
void Music::UpdatePlayer() {
    if (IsPlaying) {
        ma_uint64 lenFrames = 0;
        ma_uint64 curFrame = 0;
        if (ma_sound_get_length_in_pcm_frames(&currentSound, &lenFrames) != MA_SUCCESS) lenFrames = 0;
        if (ma_sound_get_cursor_in_pcm_frames(&currentSound, &curFrame) != MA_SUCCESS) curFrame = 0;

        if (curFrame >= lenFrames && lenFrames > 0) {
            PlayNext();
        }
    }
}

void DrawMusicSettings(bool b_musicmemu) {
    struct AnimState {
        float x = -150.0f;
        bool active = false;
    };

    static AnimState anim;

    const float targetX = 10.0f;   // 打开时目标位置
    const float startX = -150.0f;  // 初始隐藏位置
    const float speed = 15.0f;     // 动画速度

    float destX = b_musicmemu ? targetX : startX;

    if (b_musicmemu || anim.x > startX) {
        anim.x += (destX - anim.x) * ImGui::GetIO().DeltaTime * speed;
        if (fabs(anim.x - destX) < 0.5f) anim.x = destX;

        float alpha = (anim.x - startX) / (targetX - startX);
        alpha = std::clamp(alpha, 0.0f, 1.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, alpha));

        ImGui::SetCursorPos(ImVec2(anim.x, 300));
        ImGui::BeginChild("musicmemu", ImVec2(140, 80), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

        ImGui::SetCursorPos(ImVec2(6, 6));
        if (ImGui::ButtonExIMG((char*)u8"下载歌曲", ImVec2(60, 20), 0, NULL)) {
            ShellExecuteW(nullptr, L"open", L"http://www.eev3.com/", nullptr, nullptr, SW_SHOWNORMAL);
        }

        ImGui::SameLine();
        if (ImGui::ButtonExIMG((char*)u8"刷新歌曲", ImVec2(60, 20), 0, NULL)) {
            Playlist.clear();
            Music::ScanMusicFolder(MusicPath.string());
        }

        ImGui::SetCursorPos(ImVec2(6, 31));
        if (ImGui::ButtonExIMG((char*)u8"选择文件", ImVec2(60, 20), 0, NULL)) {
            if (!threadStarted) {
                std::thread t(SelectMusicFolderThread);
                t.detach();
                threadStarted = true;
            }
        }

        ImGui::SameLine();
        if (ImGui::ButtonExIMG((char*)u8"打开文件", ImVec2(60, 20), 0, NULL)) {
            ShellExecuteW(nullptr, L"open", MusicPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        }

        ImGui::EndChild();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        anim.active = true; // 保持动画状态
    }
    else {
        anim.active = false;
    }
}

// 绘制播放器UI
void Music::DrawPlayerUI() {
    //音乐列表界面
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::SetCursorPos(ImVec2(10, 0));
    ImGui::BeginChild("musiclie", ImVec2(615, 380), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
    {
        if (!Playlist.empty()) {
            static float itemHeight = 35.0f;
            static float itemWidth = 595.0f;
            static float startX = 10.0f;
            static float startY = 10.0f;

            for (int i = 0; i < Playlist.size(); ++i) {
                std::filesystem::path name = fs::path(Playlist[i]).stem();
                auto u8 = name.u8string();
                std::string Musicname(u8.begin(), u8.end());
                std::string label = std::to_string(i + 1) + ". " + Musicname;

                std::string id = "music_item_" + std::to_string(i);
                float posY = startY + i * (itemHeight + 5);
                ImGui::SetCursorPos(ImVec2(startX, posY));

                if (DrawMusicItem(id.c_str(), label, i == currentIndex, itemWidth, itemHeight,8.0f, NULL)) {
                    currentIndex = i;
                    PlayCurrent();
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    //播放按钮
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 0.0f, 0.0f));

    ImGui::SetCursorPos(ImVec2(10, 393));
    ImGui::BeginChild("music", ImVec2(615, 55), false, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
    {
        ImGui::SetCursorPos(ImVec2(10, 8));
        if (ImGui::ButtonExIMG("##设置", ImVec2(40, 40), 0, (ImTextureID)Img::Settings)) b_musicmemu = !b_musicmemu;

        ImGui::SetCursorPos(ImVec2(240, 10));
        if (ImGui::ButtonExIMG("##上一曲", ImVec2(35, 35), 0, (ImTextureID)Img::Tp)) PlayPrev();
        ImGui::SameLine();
        if (ImGui::ButtonExIMG("##播放暂停", ImVec2(35, 35), 0, IsPlaying ? (ImTextureID)Img::Pause : (ImTextureID)Img::circled))TogglePlayPause();
        ImGui::SameLine();
        if (ImGui::ButtonExIMG("##下一曲", ImVec2(35, 35), 0, (ImTextureID)Img::Np)) PlayNext();
        ImGui::SetCursorPos(ImVec2(565, 10));
        if (ImGui::ButtonExIMG("##音量", ImVec2(35, 35), 0, (ImTextureID)Img::Volume)) b_volume = !b_volume;
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(4);

    //进度条
    static float displayProgress = 0.0f;
    static bool isDragging = false;
    ma_uint64 lenFrames = 0;
    ma_uint64 curFrame = 0;
    if (ma_sound_get_length_in_pcm_frames(&currentSound, &lenFrames) != MA_SUCCESS) lenFrames = 0;
    if (ma_sound_get_cursor_in_pcm_frames(&currentSound, &curFrame) != MA_SUCCESS) curFrame = 0;
    if (!isDragging) displayProgress = (lenFrames > 0) ? (float)curFrame / lenFrames : 0.0f;
    ImGui::SetCursorPos(ImVec2(10, 386));
    ImGui::SetNextItemWidth(615.0f);
    if (ImGui::SliderFloat("##进度条", &displayProgress, 0.0f, 1.0f, "")) isDragging = true;
    if (isDragging && ImGui::IsItemDeactivatedAfterEdit()) {
        if (lenFrames > 0) {
            ma_sound_seek_to_pcm_frame(&currentSound, (ma_uint64)(displayProgress * lenFrames));
        }
        isDragging = false;
    }

    //音量条
    if (b_volume) {
        ImGui::SetCursorPos(ImVec2(580, 250));
        ImGui::BeginChild("VolumeChild", ImVec2(30, 150), false,ImGuiWindowFlags_NoTitleBar |ImGuiWindowFlags_NoResize |ImGuiWindowFlags_NoMove |ImGuiWindowFlags_AlwaysAutoResize);
        {
            ImGui::SetCursorPos(ImVec2(0, 10));
            ImGui::VSliderFloat("##音量", ImVec2(20, 120), &volume, 0.0f, 1.0f, "%.2f");
            ma_sound_set_volume(&currentSound, volume);
        }
        ImGui::EndChild();
    }

    DrawMusicSettings(b_musicmemu);//设置UI
}