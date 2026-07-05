#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <regex>
#include <unordered_set>
#include <cctype>
#include <chrono>
#include <deque>
#include <unordered_map>
#include <atlconv.h>
#include "../Chat ServerDlg.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std::chrono;

struct ClientRateInfo {
	std::deque<steady_clock::time_point> msgTimes;
};

struct IpRateInfo {
	int tokens = 0;
	std::chrono::steady_clock::time_point lastRefill;
};

extern std::unordered_map<SOCKET, ClientRateInfo> rateMap;
extern std::unordered_map<std::string, IpRateInfo> ipRateTable;
extern SOCKET clientSock;
extern std::mutex mtx, logMtx, timeMtx, ipRateMutex, rateMtx;

constexpr int MAX_MSGS_PER_SECOND = 5;
constexpr int MAX_CONNECTIONS = 100; // 最大允许的连接数

bool isIpRateLimited(const std::string& ip);
bool isRateLimited(SOCKET client);
void BroadcastMessage(const std::string& msg, SOCKET sender);
bool isValidChatMessage(const std::string& msg);