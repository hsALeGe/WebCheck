
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

//时间标识
#define IDI_TIME_CHECK_WEB				300
#define TIME_CHECK_WEB					5

CCriticalSection CWebCheckDlg::m_csUrlSection;
std::deque<WORD> CWebCheckDlg::m_urlIndexDeque;
CWebCheckDlg* CWebCheckDlg::_instance = NULL;


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
	, m_uTimeElapse(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	if (_instance == NULL) _instance = this;
}

void CWebCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TIME, m_uTimeElapse);
}

BEGIN_MESSAGE_MAP(CWebCheckDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CHECK, &CWebCheckDlg::OnBnClickedBtnCheck)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_ADD, &CWebCheckDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CWebCheckDlg::OnBnClickedBtnDel)
	ON_WM_TIMER()
//	ON_WM_LBUTTONDOWN()
ON_BN_CLICKED(IDC_BTN_ABOUT, &CWebCheckDlg::OnBnClickedBtnAbout)
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
	LoadConfigFun();
	m_webWnd.OnInsertUrlRecord();
	m_checkThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnCheckWebStatus, NULL, 0, 0);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CWebCheckDlg::LoadConfigFun()
{
	CString strFileDlgPath;
	TCHAR szModuleDirectory[MAX_PATH];	//模块目录
	GetModuleFileName(AfxGetInstanceHandle(), szModuleDirectory, sizeof(szModuleDirectory));
	int nModuleLen = lstrlen(szModuleDirectory);
	int nProcessLen = lstrlen(AfxGetApp()->m_pszExeName) + lstrlen(TEXT(".EXE")) + 1;
	if (nModuleLen <= nProcessLen)
		return ;
	szModuleDirectory[nModuleLen - nProcessLen] = 0;
	strFileDlgPath = szModuleDirectory;
	
	strFileDlgPath += TEXT("\\url_record.xml");
	ReadXmlConfig(strFileDlgPath);
	m_webWnd.ShowWindow(SW_SHOW);

}

void CWebCheckDlg::ReadXmlConfig(CString strPath)
{
	//创建XML是否成功
	::CoInitialize(NULL);
	HRESULT HR = m_xmlDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (!SUCCEEDED(HR))
	{
		ASSERT(false);
		return;
	}

	//加载xml
	if (!m_xmlDoc->load(strPath.GetBuffer()))
	{
		//ASSERT(false);
		return;
	}

	MSXML2::IXMLDOMElementPtr xmlRoot;
	MSXML2::IXMLDOMNodeListPtr xmlNodes;//某个节点的所有子节点
	MSXML2::IXMLDOMNamedNodeMapPtr xmlNodeAtts;//某个节点的所有属性
	MSXML2::IXMLDOMNodePtr	  xmlNode;//某子节点	
	
	xmlRoot = m_xmlDoc->GetdocumentElement();		//获取根节点
	//获取所有根节点的子节点
	xmlRoot->get_childNodes(&xmlNodes);

	LONG xmlNodeNum = 0, attsNum = 0;
	//子节点的数量
	xmlNodes->get_length(&xmlNodeNum);

	//节点信息
	for (size_t i = 0; i < xmlNodeNum; i++)
	{
		//获得某个子节点
		xmlNodes->get_item(i, &xmlNode);
		//获得某个节点的所有属性
		xmlNode->get_attributes(&xmlNodeAtts);
		//获得所有属性的个数
		xmlNodeAtts->get_length(&attsNum);

		for (size_t j = 0; j < attsNum; j++)
		{
			//获得某个属性
			xmlNodeAtts->get_item(j, &xmlNode);
			CString str1 = xmlNode->nodeName;
			CString str2 = xmlNode->text;
			m_webWnd.m_strUrlRecord.insert(std::make_pair(j, str2));
		}
	}

	m_xmlDoc->save(TEXT("url_record.xml"));
	xmlNodes->Release();
	xmlNode->Release();
	xmlRoot->Release();
	m_xmlDoc.Release();
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
	UpdateData(TRUE);

	int index = 0;
	std::map<WORD, CWndWebList*>::iterator itr = m_webWnd.m_webListMap.begin();
	for (; itr != m_webWnd.m_webListMap.end();)
	{
		itr->second->UpdateData(TRUE);
		CEdit *edit = (CEdit*)itr->second->GetDlgItem(IDC_EDIT_URL_INDEX + itr->second->GetIndex());
		CString str=TEXT("");
		edit->GetWindowText(str);
		str.TrimLeft();
		str.TrimRight();

		if (str.IsEmpty())
		{
			++index;
		}
		else
		{
			m_csUrlSection.Lock();
			m_urlIndexDeque.push_back(itr->second->GetIndex());
			m_csUrlSection.Unlock();
		}
		++itr;
	}

	if (index == m_webWnd.m_webListMap.size())
	{
		MessageBox(TEXT("请输入有效的连接！"));
		return;
	}

	//单位秒
	if (m_uTimeElapse < 60) m_uTimeElapse = TIME_CHECK_WEB * 60;
	SetTimer(IDI_TIME_CHECK_WEB, m_uTimeElapse * 1000, NULL);

}


int CWebCheckDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_webWnd.Create(NULL, TEXT("WndWebList"), WS_VISIBLE | WS_CHILD | WS_VSCROLL, CRect(0,0,0,0), this, IDC_WNDWEBLIST_CTRL);

	return 0;
}


void CWebCheckDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (m_webWnd)
	{
		m_webWnd.MoveWindow(CRect(10, 18, 560, 300));
	}
}


void CWebCheckDlg::OnBnClickedBtnAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	m_webWnd.OnAddUrlRecord(TEXT(""));

	std::map<WORD, CWndWebList*>::iterator itr = m_webWnd.m_webListMap.begin();
	for (; itr != m_webWnd.m_webListMap.end(); )
	{
		CEdit *edit = (CEdit*)itr->second->GetDlgItem(IDC_EDIT_URL_INDEX + itr->second->GetIndex());
		edit->SetLimitText(256);
		++itr;
	}
}

CWebCheckDlg *CWebCheckDlg::GetInstance()
{
	if (NULL == _instance)
	{
		_instance = new CWebCheckDlg();
	}
	
	return _instance;
}

void CWebCheckDlg::OnBnClickedBtnDel()
{
	// TODO: 在此添加控件通知处理程序代码
	std::map<WORD, CWndWebList*>::iterator itr = m_webWnd.m_webListMap.begin();
	if (itr == m_webWnd.m_webListMap.end()) KillTimer(IDI_TIME_CHECK_WEB);
	for (; itr != m_webWnd.m_webListMap.end(); )
	{
		CButton *bt = (CButton*)itr->second->GetDlgItem(IDC_CHECKBOX_INDEX + itr->second->GetIndex());
		if(1 == bt->GetCheck())
		{	
			itr->second->OnDestroyWebListWnd();
			delete itr->second;
			itr->second = 0;
			m_webWnd.m_webListMap.erase(itr++);
		}
		else {
			itr++;
		}
	}
	Invalidate();

	m_webWnd.OnMoveWebListCtrl();

}


void CWebCheckDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case IDI_TIME_CHECK_WEB:
	{
		std::map<WORD, CWndWebList*>::iterator itr = m_webWnd.m_webListMap.begin();
		for (; itr != m_webWnd.m_webListMap.end() ;)
		{
			CEdit *edit = (CEdit*)itr->second->GetDlgItem(IDC_EDIT_URL_INDEX + itr->second->GetIndex());
			CString str = TEXT("");
			edit->GetWindowText(str);
			str.TrimLeft();
			str.TrimRight();
			if (!str.IsEmpty())
			{
				m_csUrlSection.Lock();
				m_urlIndexDeque.push_back(itr->second->GetIndex());
				m_csUrlSection.Unlock();
			}
			++itr;
		}
	}
		break;
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CWebCheckDlg::OnCheckWebStatus(LPVOID lparam)
{

	CInternetSession sess;
	int index = -1;
	WORD c = 0;
	CHttpFile *pHttpFile = NULL;
	CString strHtml = TEXT("");
	DWORD dwStatusCode;
	while (true)
	{
		try
		{
			try
			{
				if (!m_urlIndexDeque.empty())
				{
					m_csUrlSection.Lock();
					index = m_urlIndexDeque.front();
					m_urlIndexDeque.pop_front();
					CString str = CWebCheckDlg::GetInstance()->m_webWnd.m_webListMap[index]->GetUrlString();
					pHttpFile = (CHttpFile*)sess.OpenURL(str);
					m_csUrlSection.Unlock();

					pHttpFile->QueryInfoStatusCode(dwStatusCode);
				}

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
				while (pHttpFile->ReadString(strLine) != NULL)
				{
					strHtml += strLine;
				}

				if (strHtml)
				{
					int len = MultiByteToWideChar(CP_UTF8, 0, (char*)strHtml.GetBuffer(), -1, NULL, 0);
					pszMsg = new TCHAR[len];
					MultiByteToWideChar(CP_UTF8, 0, (char*)strHtml.GetBuffer(), -1, pszMsg, len);
				}

			}
			else {
				if (index != -1)
				{
					CWebCheckDlg::GetInstance()->m_webWnd.m_webListMap[index]->SetCheckWebValue(0);
					index = -1;
				}
				continue;
			}
			DWORD dwStatus = 0;
			pHttpFile->QueryInfoStatusCode(dwStatus);
	
			delete pHttpFile;
			pHttpFile = NULL;



			if (index != -1)
			{
				CWebCheckDlg::GetInstance()->m_webWnd.m_webListMap[index]->SetCheckWebValue(dwStatus);
				index = -1;
			}
		}
		catch (const std::exception&)
		{

		}
	}
	sess.Close();
	pHttpFile->Close();
}

BOOL CWebCheckDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CDialogEx::OnCommand(wParam, lParam);
}


void CWebCheckDlg::OnBnClickedBtnAbout()
{
	// TODO: 在此添加控件通知处理程序代码
	CAboutDlg dlg;
	dlg.DoModal();
}
