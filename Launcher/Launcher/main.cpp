#include <Windows.h>
#include "Gui/LoginWindow.h"
#include <QFontDatabase>
#include <QProcessEnvironment>
#include <QtWidgets/QApplication>
#include "Utils/Encrypt.h"

std::string g_version, currentversion;
std::vector<std::string> members;

int main(int argc, char* argv[])
{
	//初始化
	//miao::StartConsole();
	//DebuggerDetectEnhancedSafe::Yz4AbX(500, 3);//开启反调试
	currentversion = "Version: 2.0.0.29";

	// 检查是否重复
	HWND hWnd = FindWindowW(L"Qt683QWindowIcon", L"shaoze");
	if (hWnd) {
		MessageBoxW(NULL, L"程序正在运行!", L"Erorr", MB_OK | MB_ICONERROR);
		return 0;
	}

	QApplication app(argc, argv);

	// 加载LOG
	QPixmap pixmap(":/Launcher/Imgs/loading.png");
	QLabel* label = new QLabel();
	label->setPixmap(pixmap);
	label->setFixedSize(300, 300);
	label->setScaledContents(true);
	label->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
	label->setAttribute(Qt::WA_TranslucentBackground);
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	label->move((screenGeometry.width() - 300) / 2, (screenGeometry.height() - 300) / 2);
	label->show();
	QApplication::processEvents();

	//加载字体
	int fontId = QFontDatabase::addApplicationFont(":/Launcher/fonts/NotoSansSC-VariableFont_wght.ttf");
	if (fontId != -1) {
		QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
		QFont appFont(family);
		appFont.setPointSize(12);
		appFont.setHintingPreference(QFont::PreferNoHinting);
		appFont.setStyleStrategy(QFont::PreferAntialias);
		appFont.setKerning(true);
		QApplication::setFont(appFont);
	}
	else {
		MessageBoxW(NULL, L"字体加载失败!", L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	// 连接服务器
	//if (!Edun::验证_初始化("1.14.60.9", 30001, "Shaoze Launcher", 1)) {
	//	MessageBoxW(NULL, L"服务器连接失败!", L"Erorr", MB_OK | MB_ICONERROR);
	//	label->close();
	//	return 0;
	//}

	// 文件下载
	//auto Data = Edun::文件_下载服务器文件("Version.txt");
	//if (!Data.empty()) {
	//	// 获取版本
	//	g_version = std::string(Data.begin(), Data.end());
	//	if (g_version.size() >= 3 && (unsigned char)
	//		g_version[0] == 0xEF && (unsigned char)
	//		g_version[1] == 0xBB && (unsigned char)
	//		g_version[2] == 0xBF)g_version.erase(0, 3);
	//}
	//else {
	//	MessageBoxW(NULL, L"公告获取失败!", L"Erorr", MB_OK | MB_ICONERROR);
	//	label->close();
	//	return 0;
	//}

	//版本对比
	//if (currentversion != g_version) {
	//	int ret = MessageBoxW(NULL, L"发现有新版本,是否更新?", L"Erorr", MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
	//	if (ret == IDOK) {
	//		ShellExecuteA(NULL, "open", "https://github.com/shaoze20/Shaoze-Launcher", NULL, NULL, SW_SHOWNORMAL);
	//		label->close();
	//		return 0;
	//	}
	//}

	//读取配置文件
	//miao::LoadData();

	//登录窗口
	LoginWindow window;
	window.setWindowIcon(QIcon(":/Launcher/Imgs/Logo.ico"));
	window.setWindowTitle(QStringLiteral("shaoze"));
	window.show();

	label->close();
	return app.exec();
}