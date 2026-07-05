#include "DesignSystem.h"

DesignSystem* DesignSystem::m_instance = nullptr;

DesignSystem::DesignSystem()
{
	//程序运行路径
	QString appPath = QCoreApplication::applicationDirPath();

	//多媒体路径
	homeVideoPath = appPath + "/resources/Home.png";
	tkaImgPath = appPath + "/resources/TKA.png";
	wuwaImgPath = appPath + "/resources/Wuwa.png";
	ysImgPath = appPath + "/resources/YS.png";

	//dll路径
	tkaDllPath = appPath + "/lib/TKA/shaoze.dll";
	wuwaDllPath = appPath + "/lib/Wuwa/shaoze.dll";
	ysDllPath = appPath + "/lib/YS/shaoze.dll";

	//MOD
	wuwamodDllPath = appPath + "/lib/Wuwa/loadmod.dll";
	ysmodDllPath = appPath + "/lib/YS/loadmod.dll";

	//文件夹路径
	wuwamodPagePath = appPath + "/lib/Wuwa/WWMI/Mods";
	ysmodPagePath = appPath + "/lib/YS/GIMI/Mods";
}

QString& DesignSystem::homeVideoFilePath()
{
	return homeVideoPath;
}

QString& DesignSystem::tkaImgFilePath()
{
	return tkaImgPath;
}

QString& DesignSystem::tkaDllFilePath()
{
	return tkaDllPath;
}

QString& DesignSystem::wuwaDllFilePath()
{
	return wuwaDllPath;
}

QString& DesignSystem::wuwamodDllFilePath()
{
	return wuwamodDllPath;
}

QString& DesignSystem::wuwamodPageFilePath()
{
	return wuwamodPagePath;
}

QString& DesignSystem::wuwaImgFilePath()
{
	return wuwaImgPath;
}

QString& DesignSystem::ysDllFilePath()
{
	return ysDllPath;
}

QString& DesignSystem::ysmodDllFilePath()
{
	return ysmodDllPath;
}

QString& DesignSystem::ysmodPageFilePath()
{
	return ysmodPagePath;
}

QString& DesignSystem::ysImgFilePath()
{
	return ysImgPath;
}