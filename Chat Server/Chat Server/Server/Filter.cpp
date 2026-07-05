#include "Server.h"

std::unordered_map<SOCKET, ClientRateInfo> rateMap;

// 每秒最多允许发送的消息数
bool isRateLimited(SOCKET client) {
	std::lock_guard<std::mutex> lock(rateMtx);
	auto& info = rateMap[client];
	auto now = steady_clock::now();

	while (!info.msgTimes.empty() && duration_cast<seconds>(now - info.msgTimes.front()).count() >= 1) {
		info.msgTimes.pop_front();
	}

	if (info.msgTimes.size() >= MAX_MSGS_PER_SECOND) {
		return true;
	}

	info.msgTimes.push_back(now);
	return false;
}

// 每个 IP 每秒最多发送 5 条消息（上限10）
std::unordered_map<std::string, IpRateInfo> ipRateTable;
bool isIpRateLimited(const std::string& ip) {
	std::lock_guard<std::mutex> lock(ipRateMutex);
	auto now = std::chrono::steady_clock::now();
	auto& info = ipRateTable[ip];

	// 初始化
	if (info.tokens == 0 && info.lastRefill.time_since_epoch().count() == 0)
		info = { 10, now };

	// 补充令牌（1秒5个）
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - info.lastRefill).count();
	if (elapsed >= 1) {
		int refill = (int)(elapsed * 5);
		info.tokens = (info.tokens + refill > 10) ? 10 : info.tokens + refill;
		info.lastRefill = now;
	}

	// 消耗令牌
	if (info.tokens > 0) {
		info.tokens--;
		return false; //
	}

	return true; //
}

// 快速子串检查
std::vector<std::string> suspiciousSubstrings = {
	"ftp://",
	"wget", "curl", "powershell", "pwsh", "cmd.exe",
	"exec(", "system(", "popen(", "CreateProcess", "ShellExecute",
	"chmod ", "rm ", "del ", "scp ", "ssh ", "nc ", "netcat",
	"base64", "curl -", "wget ", "Invoke-Expression", "IEX ",
	";", "&", "|", "`", "$(", "${", ">/dev/null", "\\x"
};

// 从 URL 提取域名的简单函数
std::string extractDomainFromUrl(const std::string& url) {
	auto pos = url.find("://");
	size_t start = (pos == std::string::npos) ? 0 : pos + 3;
	auto end = url.find_first_of("/: \t\r\n", start);
	std::string host = (end == std::string::npos) ? url.substr(start) : url.substr(start, end - start);
	if (host.rfind("www.", 0) == 0) host = host.substr(4);
	std::transform(host.begin(), host.end(), host.begin(), [](unsigned char c) { return std::tolower(c); });
	return host;
}

// 预编译正则，用于更严格的检测（忽略大小写）
std::vector<std::regex> suspiciousRegex = {
	//std::regex(R"((https?:\/\/)[^\s]+)", std::regex::icase),            // [0] URL
	std::regex(R"(\bwget\b|\bcurl\b)", std::regex::icase),             // [1] 下载工具
	std::regex(R"(\bpowershell\b|\bInvoke-Expression\b|\bIEX\b)", std::regex::icase), // [2]
	std::regex(R"(\b(cmd\.exe|powershell|sh|bash)\b)", std::regex::icase),            // [3]
	std::regex(R"([;|&`$\>\<])"),                                      // [4] shell 元字符
	std::regex(R"(\\x[0-9A-Fa-f]{2})"),                                // [5] 十六进制注入序列
	std::regex(R"(\b(base64)\s+\S{10,})", std::regex::icase)            // [6] 可疑 base64 数据（长串）
};

// 判断用户名是否合法
bool isValidChatMessage(const std::string& msg) {
	if (msg.empty()) return false;
	if (msg.size() > 4096) return false;

	bool hasPrintable = false;
	for (unsigned char c : msg) {
		if (c >= 0x20) { hasPrintable = true; break; }
	}
	if (!hasPrintable) return false;

	std::string lower = msg;
	std::transform(lower.begin(), lower.end(), lower.begin(),
		[](unsigned char c) { return std::tolower(c); });

	for (const auto& sub : suspiciousSubstrings) {
		if (lower.find(sub) != std::string::npos) {
			return false;
		}
	}

	for (const auto& r : suspiciousRegex) {
		std::smatch m;
		if (std::regex_search(msg, m, r)) {
			return false;
		}
	}

	// 拦截 HTTP 请求
	if (lower.rfind("get ", 0) == 0 ||
		lower.rfind("post ", 0) == 0 ||
		lower.rfind("put ", 0) == 0 ||
		lower.rfind("head ", 0) == 0 ||
		lower.rfind("delete ", 0) == 0 ||
		lower.find("http/1.") != std::string::npos ||
		lower.find("\r\nhost:") != std::string::npos ||
		lower.find("\nhost:") != std::string::npos)
	{
		return false;
	}

	const std::vector<std::string> blockedKeywords = { "CNXN", "OPENX", "Cookie ", "mstshash", "Administrator", "SSH", "MGLNDD", "1.95.182.82","SYSTEM","Unauthorized" };

	// 系统关键字禁止
	for (const auto& keyword : blockedKeywords) {
		if (lower.find(keyword) != std::string::npos) {
			return false;
		}
	}

	// 控制字符过滤
	for (unsigned char uc : msg) {
		if (uc < 32 && uc != '\t') return false;
		if (uc == 127) return false;
	}

	return true;
}