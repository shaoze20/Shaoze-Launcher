#include <winsock2.h>
#include <ws2tcpip.h> 
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <deque>
#include <string>     
#include <regex>
#include <thread>
#include <mutex>
#include <filesystem>
#include <type_traits>
#include <atomic>     
#include <QString>

extern std::atomic<int> onlineCount;
extern std::mutex msgMutex;
extern std::deque<std::string> messages;
extern SOCKET g_socket;
extern QString username;

class Chat {
public:
	static bool InitSocket(const char* ip, int port = 23333);
	static void ShutdownSocket();
	static void InitSocket_();
};