
// Chat ServerDlg.h: 头文件
//

#pragma once
#include "framework.h"

// CChatServerDlg 对话框
class CChatServerDlg : public CDialogEx
{
// 构造
public:
	CChatServerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHAT_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CEdit m_logEdit;

	// 生成的消息映射函数
	LRESULT OnAppendLogMessage(WPARAM wParam, LPARAM lParam);
	bool ServerInitial();
	void ClientHandler(SOCKET clientSock, sockaddr_in clientAddr);
	void AppendDebugLog(const CString& text);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();

	CEdit m_editInput;
};
