#include "Chat.h"
#pragma comment(lib, "ws2_32.lib")

SOCKET g_socket = INVALID_SOCKET;
std::thread recvThread;
std::mutex msgMutex;
std::deque<std::string> messages;
std::atomic<int> onlineCount = 0;
bool running = true;
char inputBuf[256] = "";
QString username = "";
bool isSelf = false;
bool Isconnect = false;
bool b_usercmemu = false;

void ClearMessages() {
	std::lock_guard<std::mutex> lock(msgMutex);
	messages.clear();
}

void RecvThread() {
	char buf[512];
	std::string recvBuffer;

	while (running) {
		int len = recv(g_socket, buf, sizeof(buf), 0);
		if (len > 0) {
			recvBuffer.append(buf, len);

			size_t pos;
			while ((pos = recvBuffer.find('\n')) != std::string::npos) {
				std::string message = recvBuffer.substr(0, pos);
				recvBuffer.erase(0, pos + 1);

				if (message.rfind("[SERVER_COUNT]", 0) == 0) {
					try {
						onlineCount = std::stoi(message.substr(strlen("[SERVER_COUNT]")));
					}
					catch (...) {
						onlineCount = 0;
					}

					continue;
				}

				{
					std::lock_guard<std::mutex> lock(msgMutex);
					messages.push_back(message);
					if (messages.size() > 500) {
						messages.pop_front();
					}
				}
			}
		}
		else {
			break;
		}
	}
}

bool Chat::InitSocket(const char* ip, int port) {
	static bool wsaInitialized = false;
	if (!wsaInitialized) {
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa)) return false;
		wsaInitialized = true;
	}

	g_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_socket == INVALID_SOCKET) return false;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (InetPtonA(AF_INET, ip, &addr.sin_addr) != 1) return false;

	u_long mode = 1;
	ioctlsocket(g_socket, FIONBIO, &mode);

	int ret = connect(g_socket, (sockaddr*)&addr, sizeof(addr));
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) return false;
	}

	fd_set writeSet;
	FD_ZERO(&writeSet);
	FD_SET(g_socket, &writeSet);

	timeval timeout{};
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	ret = select(0, nullptr, &writeSet, nullptr, &timeout);
	if (ret <= 0) {
		closesocket(g_socket);
		return false;
	}

	mode = 0;
	ioctlsocket(g_socket, FIONBIO, &mode);

	running = true;
	recvThread = std::thread(RecvThread);

	//设置用户名
	QByteArray bytes = username.toUtf8();
	send(g_socket, bytes.constData(), bytes.size(), 0);
	return true;
}

void Chat::ShutdownSocket() {
	running = false;
	shutdown(g_socket, SD_BOTH);
	closesocket(g_socket);
	if (recvThread.joinable()) recvThread.join();
}

void Chat::InitSocket_() {
	ShutdownSocket();
	ClearMessages();
	Sleep(200);

	if (Chat::InitSocket("127.0.0.1")) {
		Isconnect = true;
	}
	else {
		Isconnect = false;
	}
}