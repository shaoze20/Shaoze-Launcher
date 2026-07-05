#include "Server.h"

std::vector<SOCKET> clients;
std::mutex mtx, logMtx, timeMtx, ipRateMutex, rateMtx;
bool serverRunning = true;
const std::string logFilePath = "chat_log.txt";
std::unordered_map<SOCKET, std::string> clientUsernames;
std::chrono::steady_clock::time_point lastBroadcastTime = std::chrono::steady_clock::now();
SOCKET clientSock;

//广播当前在线人数
void BroadcastOnlineCount() {
    std::lock_guard<std::mutex> lock(mtx);
    std::string msg = "[SERVER_COUNT]" + std::to_string(clients.size()) + "\n";
    for (auto& client : clients) {
        send(client, msg.c_str(), (int)msg.size(), 0);
    }
}

// 追加消息到日志文件
void AppendLog(const std::string& msg) {
    std::lock_guard<std::mutex> lock(logMtx);
    std::ofstream logFile(logFilePath, std::ios::app);
    if (logFile.is_open()) {
        logFile << msg << std::endl;
    }
}

// 向单个客户端发送历史消息
void SendHistory(SOCKET clientSock) {
    std::lock_guard<std::mutex> lock(logMtx);
    std::ifstream logFile(logFilePath);
    if (!logFile.is_open()) return;

    std::string line;
    while (std::getline(logFile, line)) {
        send(clientSock, line.c_str(), (int)line.size(), 0);
        send(clientSock, "\n", 1, 0); // 保留换行
    }
}

// 发送给所有客户端
void BroadcastMessage(const std::string& msg, SOCKET sender) {
    AppendLog(msg); // 写入日志

    std::lock_guard<std::mutex> lock(mtx);
    for (auto& client : clients) {
        send(client, msg.c_str(), (int)msg.size(), 0);
        send(client, "\n", 1, 0);
    }
}

// 清理没用字符
std::string sanitize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        unsigned char uc = (unsigned char)c;
        if (uc >= 0x20 || c == '\n' || c == '\r' || c == '\t') {
            out += c;
        }
    }
    return out;
}

//广播时间
void CheckAndBroadcastTime() {
    using namespace std::chrono;
    std::lock_guard<std::mutex> lock(timeMtx);

    static const auto interval = minutes(5);
    auto now = steady_clock::now();

    if (now - lastBroadcastTime >= interval) {
        lastBroadcastTime = now;

        // 获取服务器时间
        time_t t = time(nullptr);
        tm localTime;
        localtime_s(&localTime, &t);

        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &localTime);

        std::string msg = std::string("[SERVER_MESSAGE]") + buf;
        std::cout << msg << std::endl;
        BroadcastMessage(msg, INVALID_SOCKET);
    }
}

// 单个客户端线程
void CChatServerDlg::ClientHandler(SOCKET clientSock, sockaddr_in clientAddr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));
    int port = ntohs(clientAddr.sin_port);

    // 初始用户名设置
    char username[100];
    int len = recv(clientSock, username, sizeof(username) - 1, 0);
    if (len <= 0) {
        closesocket(clientSock);
        return;
    }

    username[len] = '\0';
    std::string userStr(username);

    // ==== 检查用户名合法性 ====
    if (!isValidChatMessage(userStr)) {
        closesocket(clientSock);
        AppendDebugLog(L"[" + CString(ip) + L"]非法用户,连接已拒绝.\n");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        clients.push_back(clientSock);
        clientUsernames[clientSock] = username;
    }

    SendHistory(clientSock);// 发送历史记录
    std::string welcome = "[" + std::string(ip) + "|" + std::string(username) + std::string(u8"加入聊天室\n");
    AppendDebugLog(CString(CA2W(welcome.c_str(), CP_UTF8)));

    //BroadcastMessage("[SERVER_MESSAGE]" + std::string(username) + " 加入聊天", clientSock);
    BroadcastOnlineCount();

    char buf[512];

    try {
        while (true) {
            len = recv(clientSock, buf, sizeof(buf) - 1, 0);
            if (len <= 0) break;

            buf[len] = '\0';
            std::string msg = sanitize(buf);

            CheckAndBroadcastTime();

            // ---- 速率限制 ----
            if (isRateLimited(clientSock) || isIpRateLimited(ip)) {
                std::string warn = "[SERVER_MESSAGE]Too many messages, please slow down.\n";
                send(clientSock, warn.c_str(), (int)warn.size(), 0);
                continue;
            }

            if (!isValidChatMessage(msg)) continue;

            std::string usernameCopy;
            {
                std::lock_guard<std::mutex> lock(mtx);
                usernameCopy = clientUsernames[clientSock];
            }

            std::string fullMsg = "[" + std::string(ip) + "|" + usernameCopy + "]" + msg;
            AppendDebugLog(CString(CA2W((fullMsg + "\n").c_str(), CP_UTF8)));

            BroadcastMessage(fullMsg, clientSock);
        }
    }
    catch (const std::exception& e) {
        AppendDebugLog(L"[ClientHandler]异常:" + CString(e.what()));
    }
    catch (...) {
        AppendDebugLog(L"[ClientHandler]未知异常!");
    }

    // ---- 客户端断开清理 ----
    {
        std::lock_guard<std::mutex> lock(mtx);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSock), clients.end());
        clientUsernames.erase(clientSock);
    }

    {
        std::lock_guard<std::mutex> rlock(rateMtx);
        rateMap.erase(clientSock);
    }

    {
        std::lock_guard<std::mutex> iplock(ipRateMutex);
        ipRateTable.erase(ip);
    }

    closesocket(clientSock);

    std::string leaveMsg = "[" + std::string(ip) + "|" + std::string(username) + std::string(u8"退出聊天室\n");
    AppendDebugLog(CString(CA2W(leaveMsg.c_str(), CP_UTF8)));
    //BroadcastMessage("[SERVER_MESSAGE]" + std::string(username) + " 退出聊天", clientSock);
    BroadcastOnlineCount();
}

bool CChatServerDlg::ServerInitial() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return false;
    }
    
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(23333);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSock);
        WSACleanup();
        return false;
    }

    listen(serverSock, 5);
    AppendDebugLog(L"聊天室启动成功.\n");

    while (serverRunning) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientSize);
        if (clientSock == INVALID_SOCKET)
            continue;

        {
            std::lock_guard<std::mutex> lock(mtx);
            if ((int)clients.size() >= MAX_CONNECTIONS) {
                std::string rejectMsg = "[SERVER_MESSAGE] Server is full, please try again later.\n";
                send(clientSock, rejectMsg.c_str(), (int)rejectMsg.size(), 0);
                closesocket(clientSock);
                continue;
            }
        }

        std::thread(&CChatServerDlg::ClientHandler, this, clientSock, clientAddr).detach();
    }

    closesocket(serverSock);
    WSACleanup();
    return true;
}