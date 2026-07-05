
// Chat ServerDlg.cpp: 实现文件
//

#include "framework.h"
#include "Chat Server.h"
#include "Chat ServerDlg.h"
#include "afxdialogex.h"
#include "Server/Server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChatServerDlg 对话框

void CChatServerDlg::AppendDebugLog(const CString& text)
{
	CString* pText = new CString(text);
	PostMessage(WM_USER + 100, 0, (LPARAM)pText);
}

LRESULT CChatServerDlg::OnAppendLogMessage(WPARAM wParam, LPARAM lParam)
{
	CString* pText = (CString*)lParam;
	if (pText)
	{
		int nLen = m_logEdit.GetWindowTextLength();
		m_logEdit.SetSel(nLen, nLen);
		m_logEdit.ReplaceSel(*pText);
		m_logEdit.LineScroll(m_logEdit.GetLineCount() - 1);
		delete pText;
	}
	return 0;
}

CChatServerDlg::CChatServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHAT_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChatServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_Log, m_logEdit);
	DDX_Control(pDX, IDC_EDIT1, m_editInput);
}

BEGIN_MESSAGE_MAP(CChatServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_USER + 100, &CChatServerDlg::OnAppendLogMessage)
	ON_BN_CLICKED(IDC_BUTTON1, &CChatServerDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CChatServerDlg 消息处理程序

BOOL CChatServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置图标
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 禁止拖拽和最大化，但允许最小化
	LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
	style &= ~WS_THICKFRAME;          // 去掉可拖动边框
	style &= ~WS_MAXIMIZEBOX;         // 去掉最大化按钮
	style |= WS_MINIMIZEBOX;          // 确保有最小化按钮
	SetWindowLong(m_hWnd, GWL_STYLE, style);

	// TODO: 在此添加额外的初始化代码
	std::thread server(&CChatServerDlg::ServerInitial, this);
	server.detach();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChatServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CChatServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatServerDlg::OnBnClickedButton1()
{
	CString strText;
	m_editInput.GetWindowText(strText);

	if (!strText.IsEmpty()) {
		USES_CONVERSION;
		std::string input = CW2A(strText);

		BroadcastMessage("[SERVER_MESSAGE]" + input, clientSock);

		m_editInput.SetWindowText(_T(""));
	}
}
