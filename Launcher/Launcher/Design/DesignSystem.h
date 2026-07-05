#pragma once
#include <QObject>
#include <QApplication>
#include <QWidget>

class DesignSystem : public QObject
{
	Q_OBJECT

public:
	static DesignSystem* instance()
	{
		if (!m_instance)
		{
			m_instance = new DesignSystem();
		}
		return m_instance;
	}

	// 获取主窗口指针
	void setMainWindow(QWidget* mainWindow) { m_mainWindow = mainWindow; }
	QWidget* getMainWindow() const { return m_mainWindow; }
	int dialogWidth() const
	{
		if (!m_mainWindow)
			return 0;
		return static_cast<int>(m_mainWindow->width() * 0.42);
	}

	int dialogHeight() const
	{
		if (!m_mainWindow)
			return 0;
		return static_cast<int>(m_mainWindow->height() * 0.43);
	}

	// 主题颜色
	struct Colors
	{
		QColor background = QColor(250, 250, 250);     // 主背景
		QColor sidePanel = QColor(240, 240, 240);      // 左侧面板
	} color;

	// 多媒体文件路径
	QString& homeVideoFilePath();
	QString& tkaImgFilePath();
	QString& wuwaImgFilePath();
	QString& ysImgFilePath();

	//Dll路径
	QString& tkaDllFilePath();
	QString& wuwaDllFilePath();
	QString& wuwamodDllFilePath();
	QString& ysDllFilePath();
	QString& ysmodDllFilePath();

	//文件夹路径
	QString& wuwamodPageFilePath();
	QString& ysmodPageFilePath();

private:
	DesignSystem();
	DesignSystem(const DesignSystem&) = delete;
	DesignSystem& operator=(const DesignSystem&) = delete;

	static DesignSystem* m_instance;
	QWidget* m_mainWindow = nullptr;		// 用于获取主窗口

	// 多媒体文件路径
	QString homeVideoPath;

	//外部图片路径
	QString tkaImgPath;
	QString wuwaImgPath;
	QString ysImgPath;

	//文件夹路径
	QString wuwamodPagePath;
	QString ysmodPagePath;

	//Dll路径
	QString tkaDllPath;
	QString wuwaDllPath;
	QString wuwamodDllPath;
	QString ysDllPath;
	QString ysmodDllPath;
};