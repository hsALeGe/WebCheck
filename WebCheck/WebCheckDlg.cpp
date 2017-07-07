
// WebCheckDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "WebCheck.h"
#include "WebCheckDlg.h"
#include "afxdialogex.h"
#include  <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//控件标识
#define IDC_WNDWEBLIST_CTRL				100


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWebCheckDlg 对话框



CWebCheckDlg::CWebCheckDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_WEBCHECK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWebCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWebCheckDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CHECK, &CWebCheckDlg::OnBnClickedBtnCheck)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CWebCheckDlg 消息处理程序

BOOL CWebCheckDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWebCheckDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWebCheckDlg::OnPaint()
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
HCURSOR CWebCheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWebCheckDlg::OnBnClickedBtnCheck()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strUrl = TEXT("http://baidu.com");

	CInternetSession sess;

	CHttpFile *pHttpFile = NULL;
	CString strHtml = TEXT("");
	DWORD dwStatusCode;
	try
	{
		pHttpFile = (CHttpFile*)sess.OpenURL(strUrl);
		
		pHttpFile->QueryInfoStatusCode(dwStatusCode);
	}
	catch (CInternetException * m_pException)
	{
		pHttpFile = NULL;
		m_pException->m_dwError;
		m_pException->Delete();
		sess.Close();
	}
	CString strLine;
	TCHAR *pszMsg;
	if (pHttpFile)
	{
		while (pHttpFile->ReadString(strLine)!=NULL)
		{
			strHtml += strLine;
		}

		if (strHtml)
		{
			int len =MultiByteToWideChar(CP_UTF8, 0, (char*)strHtml.GetBuffer(), -1, NULL, 0);
			pszMsg = new TCHAR[len];
			MultiByteToWideChar(CP_UTF8, 0, (char*)strHtml.GetBuffer(), -1, pszMsg, len);
		}

	}
	else {
		return;
	}
	sess.Close();
	pHttpFile->Close();
	delete pHttpFile;
	pHttpFile = NULL;

}


int CWebCheckDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_wndWebList.Create(NULL, TEXT("窗口1"), WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL, CRect(0,0,0,0), this, IDC_WNDWEBLIST_CTRL);
	m_wndWebList.ShowWindow(SW_SHOW);

	return 0;
}


void CWebCheckDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (m_wndWebList)
	{
	m_wndWebList.MoveWindow(CRect(10, 10, 440, 340));
	}
}
