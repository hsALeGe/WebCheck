
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
std::map<WORD, CString> CWebCheckDlg::m_mapErrorDes;


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
	LoadErrorDesc();
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
	KillTimer(IDI_TIME_CHECK_WEB);
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
	sess.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 4000);
	sess.SetOption(INTERNET_OPTION_SEND_TIMEOUT, 1000);
	sess.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT,6000);
	sess.SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT,1000);
	sess.SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,7000);
	sess.SetOption(INTERNET_OPTION_CONNECT_RETRIES,2);

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
					m_csUrlSection.Unlock();
					CString str = CWebCheckDlg::GetInstance()->m_webWnd.m_webListMap[index]->GetUrlString();
					pHttpFile = (CHttpFile*)sess.OpenURL(str);

					pHttpFile->QueryInfoStatusCode(dwStatusCode);
				}

			}
			catch (CInternetException * m_pException)
			{
				delete pHttpFile;
				pHttpFile = NULL;
				std::map<WORD, CString>::iterator itr = m_mapErrorDes.find(m_pException->m_dwError);
				if (itr==m_mapErrorDes.end())
				{
					CWebCheckDlg::GetInstance()->m_webWnd.m_webListMap[index]->SetCheckWebValue(0);
				}
				else {
					CWebCheckDlg::GetInstance()->m_webWnd.m_webListMap[index]->SetCheckWebValue(0,m_mapErrorDes[m_pException->m_dwError], m_pException->m_dwError);
				}
				
				m_pException->m_dwError;
				m_pException->Delete();
				return;
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

void CWebCheckDlg::LoadErrorDesc()
{
	m_mapErrorDes[1] = TEXT("功能错误");
	m_mapErrorDes[2] = TEXT("系统找不到指定的文件");
	m_mapErrorDes[3] = TEXT("系统找不到指定的路径");
	m_mapErrorDes[4] = TEXT("系统无法打开文件");
	m_mapErrorDes[5] = TEXT("拒绝访问");
	m_mapErrorDes[6] = TEXT("句柄无效");
	m_mapErrorDes[7] = TEXT("存储控制块被损坏");
	m_mapErrorDes[8] = TEXT("存储空间不足，无法处理此命令");
	m_mapErrorDes[9] = TEXT("存储控制块地址无效");
	m_mapErrorDes[10] = TEXT("环境错误");
	m_mapErrorDes[11] = TEXT("试图加载格式错误的程序");
	m_mapErrorDes[12] = TEXT("访问码无效");
	m_mapErrorDes[13] = TEXT("数据无效");
	m_mapErrorDes[14] = TEXT("存储器不足，无法完成此操作");
	m_mapErrorDes[15] = TEXT("系统找不到指定的驱动器");
	m_mapErrorDes[16] = TEXT("无法删除目录");
	m_mapErrorDes[17] = TEXT("系统无法将文件移到不同的驱动器");
	m_mapErrorDes[18] = TEXT("没有更多文件");
	m_mapErrorDes[19] = TEXT("介质受写入保护");
	m_mapErrorDes[20] = TEXT("系统找不到指定的设备");
	m_mapErrorDes[21] = TEXT("设备未就绪");
	m_mapErrorDes[22] = TEXT("设备不识别此命令");
	m_mapErrorDes[23] = TEXT("数据错误(循环冗余检查)");
	m_mapErrorDes[24] = TEXT("程序发出命令，但命令长度不正确");
	m_mapErrorDes[25] = TEXT("驱动器无法找出磁盘上特定区域或磁道的位置");
	m_mapErrorDes[26] = TEXT("无法访问指定的磁盘或软盘");
	m_mapErrorDes[27] = TEXT("驱动器找不到请求的扇区");
	m_mapErrorDes[28] = TEXT("打印机缺纸");
	m_mapErrorDes[29] = TEXT("系统无法写入指定的设备");
	m_mapErrorDes[30] = TEXT("系统无法从指定的设备上读取");
	m_mapErrorDes[31] = TEXT("连到系统上的设备没有发挥作用");
	m_mapErrorDes[32] = TEXT("进程无法访问文件，因为另一个程序正在使用此文件");
	m_mapErrorDes[33] = TEXT("进程无法访问文件，因为另一个程序已锁定文件的一部分");
	m_mapErrorDes[36] = TEXT("用来共享的打开文件过多");
	m_mapErrorDes[38] = TEXT("到达文件结尾");
	m_mapErrorDes[39] = TEXT("磁盘已满");
	m_mapErrorDes[50] = TEXT("不支持该请求");
	m_mapErrorDes[51] = TEXT("远程计算机不可用");
	m_mapErrorDes[52] = TEXT("在网络上已有重复的名称");
	m_mapErrorDes[53] = TEXT("找不到网络路径");
	m_mapErrorDes[54] = TEXT("网络忙");
	m_mapErrorDes[55] = TEXT("指定的网络资源或设备不再可用");
	m_mapErrorDes[56] = TEXT("已到达网络BIOS命令限制");
	m_mapErrorDes[57] = TEXT("网络适配器硬件出错");
	m_mapErrorDes[58] = TEXT("指定的服务器无法运行请求的操作");
	m_mapErrorDes[59] = TEXT("发生意外的网络错误");
	m_mapErrorDes[60] = TEXT("远程适配器不兼容");
	m_mapErrorDes[61] = TEXT("打印机队列已满");
	m_mapErrorDes[62] = TEXT("无法在服务器上获得用于保存待打印文件的空间");
	m_mapErrorDes[63] = TEXT("删除等候打印的文件");
	m_mapErrorDes[64] = TEXT("指定的网络名不再可用");
	m_mapErrorDes[65] = TEXT("拒绝网络访问");
	m_mapErrorDes[66] = TEXT("网络资源类型错误");
	m_mapErrorDes[67] = TEXT("找不到网络名");
	m_mapErrorDes[68] = TEXT("超过本地计算机网卡的名称限制");
	m_mapErrorDes[69] = TEXT("超出网络BIOS会话限制");
	m_mapErrorDes[70] = TEXT("远程服务器已暂停，或正在启动过程中");
	m_mapErrorDes[71] = TEXT("当前已无法再同此远程计算机连接，因为已达到计算机的连接数目极限");
	m_mapErrorDes[72] = TEXT("已暂停指定的打印机或磁盘设备");
	m_mapErrorDes[80] = TEXT("文件存在");
	m_mapErrorDes[82] = TEXT("无法创建目录或文件");
	m_mapErrorDes[83] = TEXT("INT24失败");
	m_mapErrorDes[84] = TEXT("无法取得处理此请求的存储空间");
	m_mapErrorDes[85] = TEXT("本地设备名已在使用中");
	m_mapErrorDes[86] = TEXT("指定的网络密码错误");
	m_mapErrorDes[87] = TEXT("参数错误");
	m_mapErrorDes[88] = TEXT("网络上发生写入错误");
	m_mapErrorDes[89] = TEXT("系统无法在此时启动另一个进程");
	m_mapErrorDes[100] = TEXT("无法创建另一个系统信号灯");
	m_mapErrorDes[101] = TEXT("另一个进程拥有独占的信号灯");
	m_mapErrorDes[102] = TEXT("已设置信号灯且无法关闭");
	m_mapErrorDes[103] = TEXT("无法再设置信号灯");
	m_mapErrorDes[104] = TEXT("无法在中断时请求独占的信号灯");
	m_mapErrorDes[105] = TEXT("此信号灯的前一个所有权已结束");
	m_mapErrorDes[107] = TEXT("程序停止，因为替代的软盘未插入");
	m_mapErrorDes[108] = TEXT("磁盘在使用中，或被另一个进程锁定");
	m_mapErrorDes[109] = TEXT("管道已结束");
	m_mapErrorDes[110] = TEXT("系统无法打开指定的设备或文件");
	m_mapErrorDes[111] = TEXT("文件名太长");
	m_mapErrorDes[112] = TEXT("磁盘空间不足");
	m_mapErrorDes[113] = TEXT("无法再获得内部文件的标识");
	m_mapErrorDes[114] = TEXT("目标内部文件的标识不正确");
	m_mapErrorDes[117] = TEXT("应用程序制作的IOCTL调用错误");
	m_mapErrorDes[118] = TEXT("验证写入的切换参数值错误");
	m_mapErrorDes[119] = TEXT("系统不支持请求的命令");
	m_mapErrorDes[120] = TEXT("此功能只被此系统支持");
	m_mapErrorDes[121] = TEXT("信号灯超时时间已到");
	m_mapErrorDes[122] = TEXT("传递到系统调用的数据区太小");
	m_mapErrorDes[123] = TEXT("文件名、目录名或卷标语法不正确");
	m_mapErrorDes[124] = TEXT("系统调用级别错误");
	m_mapErrorDes[125] = TEXT("磁盘没有卷标");
	m_mapErrorDes[126] = TEXT("找不到指定的模块");
	m_mapErrorDes[127] = TEXT("找不到指定的程序");
	m_mapErrorDes[128] = TEXT("没有等候的子进程");
	m_mapErrorDes[130] = TEXT("试图使用操作(而非原始磁盘I/O)的已打开磁盘分区的文件句柄");
	m_mapErrorDes[131] = TEXT("试图移动文件指针到文件开头之前");
	m_mapErrorDes[132] = TEXT("无法在指定的设备或文件上设置文件指针");
	m_mapErrorDes[133] = TEXT("包含先前加入驱动器的驱动器无法使用JOIN或SUBST命令");
	m_mapErrorDes[134] = TEXT("试图在已被合并的驱动器上使用JOIN或SUBST命令");
	m_mapErrorDes[135] = TEXT("试图在已被合并的驱动器上使用JOIN或SUBST命令");
	m_mapErrorDes[136] = TEXT("系统试图解除未合并驱动器的JOIN");
	m_mapErrorDes[137] = TEXT("系统试图解除未替代驱动器的SUBST");
	m_mapErrorDes[138] = TEXT("系统试图将驱动器合并到合并驱动器上的目录");
	m_mapErrorDes[139] = TEXT("系统试图将驱动器替代为替代驱动器上的目录");
	m_mapErrorDes[140] = TEXT("系统试图将驱动器合并到替代驱动器上的目录");
	m_mapErrorDes[141] = TEXT("系统试图替代驱动器为合并驱动器上的目录");
	m_mapErrorDes[142] = TEXT("系统无法在此时运行JOIN或SUBST");
	m_mapErrorDes[143] = TEXT("系统无法将驱动器合并到或替代为相同驱动器上的目录");
	m_mapErrorDes[144] = TEXT("目录并非根目录下的子目录");
	m_mapErrorDes[145] = TEXT("目录非空");
	m_mapErrorDes[146] = TEXT("指定的路径已在替代中使用");
	m_mapErrorDes[147] = TEXT("资源不足，无法处理此命令");
	m_mapErrorDes[148] = TEXT("指定的路径无法在此时使用");
	m_mapErrorDes[149] = TEXT("企图将驱动器合并或替代为驱动器上目录是上一个替代的目标的驱动器");
	m_mapErrorDes[150] = TEXT("系统跟踪信息未在CONFIG.SYS文件中指定，或不允许跟踪");
	m_mapErrorDes[151] = TEXT("为DosMuxSemWait指定的信号灯事件个数错误");
	m_mapErrorDes[152] = TEXT("DosMuxSemWait不可运行已设置过多的信号灯");
	m_mapErrorDes[153] = TEXT("DosMuxSemWait清单错误");
	m_mapErrorDes[154] = TEXT("输入的卷标超过目标文件系统的长度限制");
	m_mapErrorDes[155] = TEXT("无法创建另一个线程");
	m_mapErrorDes[156] = TEXT("接收进程已拒绝此信号");
	m_mapErrorDes[157] = TEXT("段已被放弃且无法锁定");
	m_mapErrorDes[158] = TEXT("段已解除锁定");
	m_mapErrorDes[159] = TEXT("线程标识的地址错误");
	m_mapErrorDes[160] = TEXT("传递到DosExecPgm的参数字符串错误");
	m_mapErrorDes[161] = TEXT("指定的路径无效");
	m_mapErrorDes[162] = TEXT("信号已暂停");
	m_mapErrorDes[164] = TEXT("无法在系统中创建更多的线程");
	m_mapErrorDes[167] = TEXT("无法锁定文件区域");
	m_mapErrorDes[170] = TEXT("请求的资源在使用中");
	m_mapErrorDes[173] = TEXT("对于提供取消区域进行锁定的请求不明显");
	m_mapErrorDes[174] = TEXT("文件系统不支持锁定类型的最小单元更改");
	m_mapErrorDes[180] = TEXT("系统检测出错误的段号");
	m_mapErrorDes[183] = TEXT("当文件已存在时，无法创建该文件");
	m_mapErrorDes[186] = TEXT("传递的标志错误");
	m_mapErrorDes[187] = TEXT("找不到指定的系统信号灯名称");
	m_mapErrorDes[196] = TEXT("操作系统无法运行此应用程序");
	m_mapErrorDes[197] = TEXT("操作系统当前的配置不能运行此应用程序");
	m_mapErrorDes[199] = TEXT("操作系统无法运行此应用程序");
	m_mapErrorDes[200] = TEXT("代码段不可大于或等于64K");
	m_mapErrorDes[203] = TEXT("操作系统找不到已输入的环境选项");
	m_mapErrorDes[205] = TEXT("命令子树中的进程没有信号处理程序");
	m_mapErrorDes[206] = TEXT("文件名或扩展名太长");
	m_mapErrorDes[207] = TEXT("第2环堆栈已被占用");
	m_mapErrorDes[208] = TEXT("没有正确输入文件名通配符*或?，或指定过多的文件名通配符");
	m_mapErrorDes[209] = TEXT("正在发送的信号错误");
	m_mapErrorDes[210] = TEXT("无法设置信号处理程序");
	m_mapErrorDes[212] = TEXT("段已锁定且无法重新分配");
	m_mapErrorDes[214] = TEXT("连到该程序或动态链接模块的动态链接模块太多");
	m_mapErrorDes[215] = TEXT("无法嵌套调用LoadModule");
	m_mapErrorDes[230] = TEXT("管道状态无效");
	m_mapErrorDes[231] = TEXT("所有的管道实例都在使用中");
	m_mapErrorDes[232] = TEXT("管道正在关闭中");
	m_mapErrorDes[233] = TEXT("管道的另一端上无任何进程");
	m_mapErrorDes[234] = TEXT("更多数据可用");
	m_mapErrorDes[240] = TEXT("取消会话");
	m_mapErrorDes[254] = TEXT("指定的扩展属性名无效");
	m_mapErrorDes[255] = TEXT("扩展属性不一致");
	m_mapErrorDes[258] = TEXT("等待的操作过时");
	m_mapErrorDes[259] = TEXT("没有可用的数据了");
	m_mapErrorDes[266] = TEXT("无法使用复制功能");
	m_mapErrorDes[267] = TEXT("目录名无效");
	m_mapErrorDes[275] = TEXT("扩展属性在缓冲区中不适用");
	m_mapErrorDes[276] = TEXT("装在文件系统上的扩展属性文件已损坏");
	m_mapErrorDes[277] = TEXT("扩展属性表格文件已满");
	m_mapErrorDes[278] = TEXT("指定的扩展属性句柄无效");
	m_mapErrorDes[282] = TEXT("装入的文件系统不支持扩展属性");
	m_mapErrorDes[288] = TEXT("企图释放并非呼叫方所拥有的多用户终端运行程序");
	m_mapErrorDes[298] = TEXT("发向信号灯的请求过多");
	m_mapErrorDes[299] = TEXT("仅完成部分的ReadProcessMemoty或WriteProcessMemory请求");
	m_mapErrorDes[300] = TEXT("操作锁定请求被拒绝");
	m_mapErrorDes[301] = TEXT("系统接收了一个无效的操作锁定确认");
	m_mapErrorDes[487] = TEXT("试图访问无效的地址");
	m_mapErrorDes[534] = TEXT("算术结果超过32位");
	m_mapErrorDes[535] = TEXT("管道的另一端有一进程");
	m_mapErrorDes[536] = TEXT("等候打开管道另一端的进程");
	m_mapErrorDes[994] = TEXT("拒绝访问扩展属性");
	m_mapErrorDes[995] = TEXT("由于线程退出或应用程序请求，已放弃I/O操作");
	m_mapErrorDes[996] = TEXT("重叠I/O事件不在信号状态中");
	m_mapErrorDes[997] = TEXT("重叠I/O操作在进行中");
	m_mapErrorDes[998] = TEXT("内存分配访问无效");
	m_mapErrorDes[999] = TEXT("错误运行页内操作");
	m_mapErrorDes[1001] = TEXT("递归太深；栈溢出");
	m_mapErrorDes[1002] = TEXT("窗口无法在已发送的消息上操作");
	m_mapErrorDes[1003] = TEXT("无法完成此功能");
	m_mapErrorDes[1004] = TEXT("无效标志");
	m_mapErrorDes[1005] = TEXT("此卷不包含可识别的文件系统 请确定所有请求的文件系统驱动程序已加载，且此卷未损坏");
	m_mapErrorDes[1006] = TEXT("文件所在的卷已被外部改变，因此打开的文件不再有效");
	m_mapErrorDes[1007] = TEXT("无法在全屏幕模式下运行请求的操作");
	m_mapErrorDes[1008] = TEXT("试图引用不存在的令牌");
	m_mapErrorDes[1009] = TEXT("配置注册表数据库损坏");
	m_mapErrorDes[1010] = TEXT("配置注册表项无效");
	m_mapErrorDes[1011] = TEXT("无法打开配置注册表项");
	m_mapErrorDes[1012] = TEXT("无法读取配置注册表项");
	m_mapErrorDes[1013] = TEXT("无法写入配置注册表项");
	m_mapErrorDes[1014] = TEXT("注册表数据库中的某一文件必须使用记录或替代复制来恢复恢复成功完成");
	m_mapErrorDes[1015] = TEXT("注册表损坏 包含注册表数据的某一文件结构损坏，或系统的文件内存映像损坏，或因为替代副本、日志缺少或损坏而无法恢复文件");
	m_mapErrorDes[1016] = TEXT("由注册表启动的I/O操作恢复失败 注册表无法读入、写出或清除任意一个包含注册表系统映像的文件");
	m_mapErrorDes[1017] = TEXT("系统试图加载或还原文件到注册表，但指定的文件并非注册表文件格式");
	m_mapErrorDes[1018] = TEXT("试图在标记为删除的注册表项上运行不合法的操作");
	m_mapErrorDes[1019] = TEXT("系统无法配置注册表日志中所请求的空间");
	m_mapErrorDes[1020] = TEXT("无法在已有子项或值的注册表项中创建符号链接");
	m_mapErrorDes[1021] = TEXT("无法在易变父项下创建稳定子项");
	m_mapErrorDes[1022] = TEXT("通知更改请求正在完成中，且信息并未返回到呼叫方的缓冲区中 当前呼叫方必须枚举文件来查找更改");
	m_mapErrorDes[1051] = TEXT("已发送停止控制到服务，该服务被其它正在运行的服务所依赖");
	m_mapErrorDes[1052] = TEXT("请求的控件对此服务无效");
	m_mapErrorDes[1053] = TEXT("服务并未及时响应启动或控制请求");
	m_mapErrorDes[1054] = TEXT("无法创建此服务的线程");
	m_mapErrorDes[1055] = TEXT("锁定服务数据库");
	m_mapErrorDes[1056] = TEXT("服务的实例已在运行中");
	m_mapErrorDes[1057] = TEXT("帐户名无效或不存在，或者密码对于指定的帐户名无效");
	m_mapErrorDes[1058] = TEXT("无法启动服务，原因可能是它被禁用或与它相关联的设备没有启动");
	m_mapErrorDes[1059] = TEXT("指定了循环服务依存");
	m_mapErrorDes[1060] = TEXT("指定的服务并未以已安装的服务存在");
	m_mapErrorDes[1061] = TEXT("服务无法在此时接受控制信息");
	m_mapErrorDes[1062] = TEXT("服务未启动");
	m_mapErrorDes[1063] = TEXT("服务进程无法连接到服务控制器上");
	m_mapErrorDes[1064] = TEXT("当处理控制请求时，在服务中发生异常");
	m_mapErrorDes[1065] = TEXT("指定的数据库不存在");
	m_mapErrorDes[1066] = TEXT("服务已返回特定的服务错误码");
	m_mapErrorDes[1067] = TEXT("进程意外终止");
	m_mapErrorDes[1068] = TEXT("依存服务或组无法启动");
	m_mapErrorDes[1069] = TEXT("由于登录失败而无法启动服务");
	m_mapErrorDes[1070] = TEXT("启动后，服务停留在启动暂停状态");
	m_mapErrorDes[1071] = TEXT("指定的服务数据库锁定无效");
	m_mapErrorDes[1072] = TEXT("指定的服务已标记为删除");
	m_mapErrorDes[1073] = TEXT("指定的服务已存在");
	m_mapErrorDes[1074] = TEXT("系统当前以最新的有效配置运行");
	m_mapErrorDes[1075] = TEXT("依存服务不存在，或已被标记为删除");
	m_mapErrorDes[1076] = TEXT("已接受使用当前引导作为最后的有效控制设置");
	m_mapErrorDes[1077] = TEXT("上次启动之后，仍未尝试引导服务");
	m_mapErrorDes[1078] = TEXT("名称已用作服务名或服务显示名");
	m_mapErrorDes[1079] = TEXT("此服务的帐户不同于运行于同一进程上的其它服务的帐户");
	m_mapErrorDes[1080] = TEXT("只能为Win32服务设置失败操作，不能为驱动程序设置");
	m_mapErrorDes[1081] = TEXT("这个服务所运行的处理和服务控制管理器相同 所以，如果服务处理程序意外中止的话，服务控制管理器无法进行任何操作");
	m_mapErrorDes[1082] = TEXT("这个服务尚未设置恢复程序");
	m_mapErrorDes[1083] = TEXT("配置成在该可执行程序中运行的这个服务不能执行该服务");
	m_mapErrorDes[1100] = TEXT("已达磁带的实际结尾");
	m_mapErrorDes[1101] = TEXT("磁带访问已达文件标记");
	m_mapErrorDes[1102] = TEXT("已达磁带或磁盘分区的开头");
	m_mapErrorDes[1103] = TEXT("磁带访问已达一组文件的结尾");
	m_mapErrorDes[1104] = TEXT("磁带上不再有任何数据");
	m_mapErrorDes[1105] = TEXT("磁带无法分区");
	m_mapErrorDes[1106] = TEXT("在访问多卷分区的新磁带时，当前的块大小不正确");
	m_mapErrorDes[1107] = TEXT("当加载磁带时，找不到分区信息");
	m_mapErrorDes[1108] = TEXT("无法锁定媒体弹出功能");
	m_mapErrorDes[1109] = TEXT("无法卸载介质");
	m_mapErrorDes[1110] = TEXT("驱动器中的介质可能已更改");
	m_mapErrorDes[1111] = TEXT("复位I/O总线");
	m_mapErrorDes[1112] = TEXT("驱动器中没有媒体");
	m_mapErrorDes[1113] = TEXT("在多字节的目标代码页中，没有此Unicode字符可以映射到的字符");
	m_mapErrorDes[1114] = TEXT("动态链接库(DLL)初始化例程失败");
	m_mapErrorDes[1115] = TEXT("系统关机正在进行");
	m_mapErrorDes[1116] = TEXT("因为没有任何进行中的关机过程，所以无法中断系统关机");
	m_mapErrorDes[1117] = TEXT("因为I/O设备错误，所以无法运行此项请求");
	m_mapErrorDes[1118] = TEXT("没有串行设备被初始化成功 串行驱动程序将卸载");
	m_mapErrorDes[1119] = TEXT("无法打开正在与其他设备共享中断请求(IRQ)的设备 至少有一个使用该IRQ的其他设备已打开");
	m_mapErrorDes[1120] = TEXT("序列I/O操作已由另一个串行口的写入完成(IOCTL_SERIAL_XOFF_COUNTER已达零");
	m_mapErrorDes[1121] = TEXT("因为已过超时时间，所以串行I/O操作完成IOCTL_SERIAL_XOFF_COUNTER未达零");
	m_mapErrorDes[1122] = TEXT("在软盘上找不到ID地址标记");
	m_mapErrorDes[1123] = TEXT("软盘扇区ID字符域与软盘控制器磁道地址不相符");
	m_mapErrorDes[1124] = TEXT("软盘控制器报告软盘驱动程序不能识别的错误");
	m_mapErrorDes[1125] = TEXT("软盘控制器返回与其寄存器中不一致的结果");
	m_mapErrorDes[1126] = TEXT("当访问硬盘时，重新校准操作失败，重试仍然失败");
	m_mapErrorDes[1127] = TEXT("当访问硬盘时，磁盘操作失败，重试仍然失败");
	m_mapErrorDes[1128] = TEXT("当访问硬盘时，即使失败，仍须复位磁盘控制器");
	m_mapErrorDes[1129] = TEXT("已达磁带结尾");
	m_mapErrorDes[1130] = TEXT("服务器存储空间不足，无法处理此命令");
	m_mapErrorDes[1131] = TEXT("检测出潜在的死锁状态");
	m_mapErrorDes[1132] = TEXT("指定的基址或文件偏移量没有适当对齐");
	m_mapErrorDes[1140] = TEXT("改变系统供电状态的尝试被另一应用程序或驱动程序否决");
	m_mapErrorDes[1141] = TEXT("系统BIOS改变系统供电状态的尝试失败");
	m_mapErrorDes[1142] = TEXT("试图在一文件上创建超过系统允许数额的链接");
	m_mapErrorDes[1150] = TEXT("指定程序要求更新的Windows版本");
	m_mapErrorDes[1151] = TEXT("指定程序不是Windows或MS=DOS程序");
	m_mapErrorDes[1152] = TEXT("只能启动该指定程序的一个实例");
	m_mapErrorDes[1153] = TEXT("该指定程序适用于旧的Windows版本");
	m_mapErrorDes[1154] = TEXT("执行该应用程序所需的库文件之一被损坏");
	m_mapErrorDes[1155] = TEXT("没有应用程序与此操作的指定文件有关联");
	m_mapErrorDes[1156] = TEXT("在输送指令到应用程序的过程中出现错误");
	m_mapErrorDes[1157] = TEXT("执行该应用程序所需的库文件之一无法找到");
	m_mapErrorDes[1158] = TEXT("当前程序已使用了Window管理器对象的系统允许的所有句柄");
	m_mapErrorDes[1159] = TEXT("消息只能与同步操作一起使用");
	m_mapErrorDes[1160] = TEXT("指出的源元素没有媒体");
	m_mapErrorDes[1161] = TEXT("指出的目标元素已包含媒体");
	m_mapErrorDes[1162] = TEXT("指出的元素不存在");
	m_mapErrorDes[1163] = TEXT("指出的元素是未显示的存储资源的一部分");
	m_mapErrorDes[1164] = TEXT("显示设备需要重新初始化，因为硬件有错误");
	m_mapErrorDes[1165] = TEXT("设备显示在尝试进一步操作之前需要清除");
	m_mapErrorDes[1166] = TEXT("设备显示它的门仍是打开状态");
	m_mapErrorDes[1167] = TEXT("设备没有连接");
	m_mapErrorDes[1168] = TEXT("找不到元素");
	m_mapErrorDes[1169] = TEXT("索引中没有同指定项相匹配的项");
	m_mapErrorDes[1170] = TEXT("在对象上不存在指定的属性集");
	m_mapErrorDes[1171] = TEXT("传递到GetMouseMovePoints的点不在缓冲区中");
	m_mapErrorDes[1172] = TEXT("跟踪(工作站)服务没运行");
	m_mapErrorDes[1173] = TEXT("找不到卷ID");
	m_mapErrorDes[1175] = TEXT("无法删除要被替换的文件");
	m_mapErrorDes[1176] = TEXT("无法将替换文件移到要被替换的文件 要被替换的文件保持原来的名称");
	m_mapErrorDes[1177] = TEXT("无法将替换文件移到要被替换的文件 要被替换的文件已被重新命名为备份名称");
	m_mapErrorDes[1178] = TEXT("卷更改记录被删除");
	m_mapErrorDes[1179] = TEXT("卷更改记录服务不处于活动中");
	m_mapErrorDes[1180] = TEXT("找到一份文件，但是可能不是正确的文件");
	m_mapErrorDes[1181] = TEXT("日志项从日志中被删除");
	m_mapErrorDes[1200] = TEXT("指定的设备名无效");
	m_mapErrorDes[1201] = TEXT("设备当前未连接上，但其为一个记录连接");
	m_mapErrorDes[1202] = TEXT("企图记录先前已被记录的设备");
	m_mapErrorDes[1203] = TEXT("无任何网络提供程序接受指定的网络路径");
	m_mapErrorDes[1204] = TEXT("指定的网络提供程序名称无效");
	m_mapErrorDes[1205] = TEXT("无法打开网络连接配置文件");
	m_mapErrorDes[1206] = TEXT("网络连接配置文件损坏");
	m_mapErrorDes[1207] = TEXT("无法枚举空载体");
	m_mapErrorDes[1208] = TEXT("发生扩展错误");
	m_mapErrorDes[1209] = TEXT("指定的组名格式无效");
	m_mapErrorDes[1210] = TEXT("指定的计算机名格式无效");
	m_mapErrorDes[1211] = TEXT("指定的事件名格式无效");
	m_mapErrorDes[1212] = TEXT("指定的域名格式无效");
	m_mapErrorDes[1213] = TEXT("指定的服务名格式无效");
	m_mapErrorDes[1214] = TEXT("指定的网络名格式无效");
	m_mapErrorDes[1215] = TEXT("指定的共享名格式无效");
	m_mapErrorDes[1216] = TEXT("指定的密码格式无效");
	m_mapErrorDes[1217] = TEXT("指定的消息名格式无效");
	m_mapErrorDes[1218] = TEXT("指定的消息目标格式无效");
	m_mapErrorDes[1219] = TEXT("提供的凭据与已存在的凭据集冲突");
	m_mapErrorDes[1220] = TEXT("企图创建网络服务器的会话，但已对该服务器创建过多的会话");
	m_mapErrorDes[1221] = TEXT("工作组或域名已由网络上的另一部计算机使用");
	m_mapErrorDes[1222] = TEXT("网络未连接或启动");
	m_mapErrorDes[1223] = TEXT("操作已被用户取消");
	m_mapErrorDes[1224] = TEXT("请求的操作无法在使用用户映射区域打开的文件上执行");
	m_mapErrorDes[1225] = TEXT("远程系统拒绝网络连接");
	m_mapErrorDes[1226] = TEXT("网络连接已被适当地关闭了");
	m_mapErrorDes[1227] = TEXT("网络传输终结点已有与其关联的地址");
	m_mapErrorDes[1228] = TEXT("地址仍未与网络终结点关联");
	m_mapErrorDes[1229] = TEXT("企图在不存在的网络连接上进行操作");
	m_mapErrorDes[1230] = TEXT("企图在使用中的网络连接上进行无效的操作");
	m_mapErrorDes[1231] = TEXT("不能访问网络位置 有关网络排除故障的信息，请参阅Windows帮助");
	m_mapErrorDes[1232] = TEXT("不能访问网络位置 有关网络排除故障的信息，请参阅Windows帮助");
	m_mapErrorDes[1233] = TEXT("不能访问网络位置 有关网络排除故障的信息，请参阅Windows帮助");
	m_mapErrorDes[1234] = TEXT("没有任何服务正在远程系统上的目标网络终结点上操作");
	m_mapErrorDes[1235] = TEXT("请求被终止");
	m_mapErrorDes[1236] = TEXT("由本地系统终止网络连接");
	m_mapErrorDes[1237] = TEXT("操作无法完成 应该重试");
	m_mapErrorDes[1238] = TEXT("因为已达到此帐户的最大同时连接数限制，所以无法连接服务器");
	m_mapErrorDes[1239] = TEXT("试图在这个帐户未被授权的时间内登录");
	m_mapErrorDes[1240] = TEXT("此帐户并未得到从这个工作站登录的授权");
	m_mapErrorDes[1241] = TEXT("请求的操作不能使用这个网络地址");
	m_mapErrorDes[1242] = TEXT("服务器已经注册");
	m_mapErrorDes[1243] = TEXT("指定的服务不存在");
	m_mapErrorDes[1244] = TEXT("因为用户还未被验证，不能执行所要求的操作");
	m_mapErrorDes[1245] = TEXT("因为用户还未登录网络，不能执行所要求的操作 指定的服务不存在");
	m_mapErrorDes[1246] = TEXT("正在继续工作");
	m_mapErrorDes[1247] = TEXT("试图进行初始操作，但是初始化已完成");
	m_mapErrorDes[1248] = TEXT("没有更多的本地设备");
	m_mapErrorDes[1249] = TEXT("指定的站点不存在");
	m_mapErrorDes[1250] = TEXT("具有指定名称的域控制器已经存在");
	m_mapErrorDes[1251] = TEXT("只有连接到服务器上时，该操作才受支持");
	m_mapErrorDes[1252] = TEXT("即使没有改动，组策略框架也应该调用扩展");
	m_mapErrorDes[1253] = TEXT("指定的用户没有一个有效的配置文件");
	m_mapErrorDes[1254] = TEXT("MicrosoftSmallBusinessServer不支持此操作");
	m_mapErrorDes[1300] = TEXT("并非所有被引用的特权都指派给呼叫方");
	m_mapErrorDes[1301] = TEXT("帐户名和安全标识间的某些映射未完成");
	m_mapErrorDes[1302] = TEXT("没有为该帐户特别设置系统配额限制");
	m_mapErrorDes[1303] = TEXT("没有可用的加密密钥 返回了一个已知加密密钥");
	m_mapErrorDes[1304] = TEXT("密码太复杂，无法转换成LANManager密码 返回的LANManager密码为空字符串");
	m_mapErrorDes[1305] = TEXT("修订级别未知");
	m_mapErrorDes[1306] = TEXT("表明两个修订级别是不兼容的");
	m_mapErrorDes[1307] = TEXT("这个安全标识不能指派为此对象的所有者");
	m_mapErrorDes[1308] = TEXT("这个安全标识不能指派为对象的主要组");
	m_mapErrorDes[1309] = TEXT("当前并未模拟客户的线程试图操作模拟令牌");
	m_mapErrorDes[1310] = TEXT("组可能未被禁用");
	m_mapErrorDes[1311] = TEXT("当前没有可用的登录服务器来服务登录请求");
	m_mapErrorDes[1312] = TEXT("指定的登录会话不存在 可能已被终止");
	m_mapErrorDes[1313] = TEXT("指定的特权不存在");
	m_mapErrorDes[1314] = TEXT("客户没有所需的特权");
	m_mapErrorDes[1315] = TEXT("提供的名称并非正确的帐户名形式");
	m_mapErrorDes[1316] = TEXT("指定的用户已存在");
	m_mapErrorDes[1317] = TEXT("指定的用户不存在");
	m_mapErrorDes[1318] = TEXT("指定的组已存在");
	m_mapErrorDes[1319] = TEXT("指定的组不存在");
	m_mapErrorDes[1320] = TEXT("指定的用户帐户已是指定组的成员，或是因为组包含成员所以无法删除指定的组");
	m_mapErrorDes[1321] = TEXT("指定的用户帐户不是指定组帐户的成员");
	m_mapErrorDes[1322] = TEXT("无法禁用或删除最后剩余的系统管理帐户");
	m_mapErrorDes[1323] = TEXT("无法更新密码 提供作为当前密码的值不正确");
	m_mapErrorDes[1324] = TEXT("无法更新密码 提供给新密码的值包含密码中不允许的值");
	m_mapErrorDes[1325] = TEXT("无法更新密码 为新密码提供的值不符合字符域的长度、复杂性或历史要求");
	m_mapErrorDes[1326] = TEXT("登录失败:未知的用户名或错误密码");
	m_mapErrorDes[1327] = TEXT("登录失败:用户帐户限制");
	m_mapErrorDes[1328] = TEXT("登录失败:违反帐户登录时间限制");
	m_mapErrorDes[1329] = TEXT("登录失败:不允许用户登录到此计算机");
	m_mapErrorDes[1330] = TEXT("登录失败:指定的帐户密码已过期");
	m_mapErrorDes[1331] = TEXT("登录失败:禁用当前的帐户");
	m_mapErrorDes[1332] = TEXT("帐户名与安全标识间无任何映射完成");
	m_mapErrorDes[1333] = TEXT("一次请求过多的本地用户标识符(LUIDs)");
	m_mapErrorDes[1334] = TEXT("无更多可用的本地用户标识符(LUIDs)");
	m_mapErrorDes[1335] = TEXT("对于该特别用法，安全ID的次级授权部分无效");
	m_mapErrorDes[1336] = TEXT("访问控制列表(ACL)结构无效");
	m_mapErrorDes[1337] = TEXT("安全ID结构无效");
	m_mapErrorDes[1338] = TEXT("安全描述符结构无效");
	m_mapErrorDes[1340] = TEXT("无法创建固有的访问控制列表(ACL)或访问控制项目(ACE)");
	m_mapErrorDes[1341] = TEXT("服务器当前已禁用");
	m_mapErrorDes[1342] = TEXT("服务器当前已启用");
	m_mapErrorDes[1343] = TEXT("提供给识别代号颁发机构的值为无效值");
	m_mapErrorDes[1344] = TEXT("无更多可用的内存以更新安全信息");
	m_mapErrorDes[1345] = TEXT("指定属性无效，或与整个群体的属性不兼容");
	m_mapErrorDes[1346] = TEXT("指定的模拟级别无效，或所提供的模拟级别无效");
	m_mapErrorDes[1347] = TEXT("无法打开匿名级安全令牌");
	m_mapErrorDes[1348] = TEXT("请求的验证信息类别无效");
	m_mapErrorDes[1349] = TEXT("令牌的类型对其尝试使用的方法不适当");
	m_mapErrorDes[1350] = TEXT("无法在与安全性无关联的对象上运行安全性操作");
	m_mapErrorDes[1351] = TEXT("未能从域控制器读取配置信息，或者是因为机器不可使用，或者是访问被拒绝");
	m_mapErrorDes[1352] = TEXT("安全帐户管理器(SAM)或本地安全颁发机构(LSA)服务器处于运行安全操作的错误状态");
	m_mapErrorDes[1353] = TEXT("域处于运行安全操作的错误状态");
	m_mapErrorDes[1354] = TEXT("此操作只对域的主要域控制器可行");
	m_mapErrorDes[1355] = TEXT("指定的域不存在，或无法联系");
	m_mapErrorDes[1356] = TEXT("指定的域已存在");
	m_mapErrorDes[1357] = TEXT("试图超出每服务器域个数的限制");
	m_mapErrorDes[1358] = TEXT("无法完成请求操作，因为磁盘上的严重介质失败或数据结构损坏");
	m_mapErrorDes[1359] = TEXT("出现了内部错误");
	m_mapErrorDes[1360] = TEXT("通用访问类型包含于已映射到非通用类型的访问掩码中");
	m_mapErrorDes[1361] = TEXT("安全描述符格式不正确(绝对或自相关的)");
	m_mapErrorDes[1362] = TEXT("请求操作只限制在登录进程中使用 调用进程未注册为一个登录进程");
	m_mapErrorDes[1363] = TEXT("无法使用已在使用中的标识启动新的会话");
	m_mapErrorDes[1364] = TEXT("未知的指定验证数据包");
	m_mapErrorDes[1365] = TEXT("登录会话并非处于与请求操作一致的状态中");
	m_mapErrorDes[1366] = TEXT("登录会话标识已在使用中");
	m_mapErrorDes[1367] = TEXT("登录请求包含无效的登录类型值");
	m_mapErrorDes[1368] = TEXT("在使用命名管道读取数据之前，无法经由该管道模拟");
	m_mapErrorDes[1369] = TEXT("注册表子树的事务处理状态与请求状态不一致");
	m_mapErrorDes[1370] = TEXT("安全性数据库内部出现损坏");
	m_mapErrorDes[1371] = TEXT("无法在内置帐户上运行此操作");
	m_mapErrorDes[1372] = TEXT("无法在内置特殊组上运行此操作");
	m_mapErrorDes[1373] = TEXT("无法在内置特殊用户上运行此操作");
	m_mapErrorDes[1374] = TEXT("无法从组中删除用户，因为当前组为用户的主要组");
	m_mapErrorDes[1375] = TEXT("令牌已作为主要令牌使用");
	m_mapErrorDes[1376] = TEXT("指定的本地组不存在");
	m_mapErrorDes[1377] = TEXT("指定的帐户名不是本地组的成员");
	m_mapErrorDes[1378] = TEXT("指定的帐户名已是本地组的成员");
	m_mapErrorDes[1379] = TEXT("指定的本地组已存在");
	m_mapErrorDes[1380] = TEXT("登录失败:未授予用户在此计算机上的请求登录类型");
	m_mapErrorDes[1381] = TEXT("已超过在单一系统中可保存机密的最大个数");
	m_mapErrorDes[1382] = TEXT("机密的长度超过允许的最大长度");
	m_mapErrorDes[1383] = TEXT("本地安全颁发机构数据库内部包含不一致性");
	m_mapErrorDes[1384] = TEXT("在尝试登录的过程中，用户的安全上下文积累了过多的安全标识");
	m_mapErrorDes[1385] = TEXT("登录失败:未授予用户在此计算机上的请求登录类型");
	m_mapErrorDes[1386] = TEXT("更改用户密码时需要交叉加密密码");
	m_mapErrorDes[1387] = TEXT("由于成员不存在，无法将成员添加到本地组中，也无法从本地组将其删除");
	m_mapErrorDes[1388] = TEXT("无法将新成员加入到本地组中，因为成员的帐户类型错误");
	m_mapErrorDes[1389] = TEXT("已指定过多的安全标识");
	m_mapErrorDes[1390] = TEXT("更改此用户密码时需要交叉加密密码");
	m_mapErrorDes[1391] = TEXT("表明ACL未包含任何可承继的组件");
	m_mapErrorDes[1392] = TEXT("文件或目录损坏且无法读取");
	m_mapErrorDes[1393] = TEXT("磁盘结构损坏且无法读取");
	m_mapErrorDes[1394] = TEXT("无任何指定登录会话的用户会话项");
	m_mapErrorDes[1395] = TEXT("正在访问的服务有连接数目标授权限制 这时候已经无法再连接，原因是已经到达可接受的连接数目上限");
	m_mapErrorDes[1396] = TEXT("登录失败:该目标帐户名称不正确");
	m_mapErrorDes[1397] = TEXT("相互身份验证失败 该服务器在域控制器的密码过期");
	m_mapErrorDes[1398] = TEXT("在客户机和服务器之间有一个时间差");
	m_mapErrorDes[12111] = TEXT("由于回话被终止，FTP操作未完成。");
	m_mapErrorDes[12112] = TEXT("被动模式在服务器上不可用。");
	m_mapErrorDes[12110] = TEXT("请求的操作不能在FTP会话句柄上进行，因为操作已在进行中。");
	m_mapErrorDes[12137] = TEXT("请求的属性找不到。");
	m_mapErrorDes[12132] = TEXT("从Gopher服务器接收数据时检测到错误。");
	m_mapErrorDes[12133] = TEXT("数据已经结束。");
	m_mapErrorDes[12135] = TEXT("该操作的定位器类型不正确。");
	m_mapErrorDes[12134] = TEXT("提供的定位器无效。");
	m_mapErrorDes[12131] = TEXT("该请求必须针对文件定位器进行。");
	m_mapErrorDes[12136] = TEXT("请求的操作只能针对Gopher +服务器或指定Gopher +操作的定位器。");
	m_mapErrorDes[12130] = TEXT("解析从Gopher服务器返回的数据时检测到错误。");
	m_mapErrorDes[12138] = TEXT("定位器类型是未知的。");
	m_mapErrorDes[12162] = TEXT("HTTP cookie被服务器拒绝。");
	m_mapErrorDes[12161] = TEXT("HTTP cookie需要确认。");
	m_mapErrorDes[12151] = TEXT("服务器没有返回任何标题。");
	m_mapErrorDes[12155] = TEXT("标题无法添加，因为它已经存在。");
	m_mapErrorDes[12150] = TEXT("请求的标题找不到。");
	m_mapErrorDes[12153] = TEXT("提供的标题无效。");
	m_mapErrorDes[12154] = TEXT("对HttpQueryInfo的请求 无效。");
	m_mapErrorDes[12152] = TEXT("服务器响应无法解析。");
	m_mapErrorDes[12160] = TEXT("HTTP请求未被重定向。");
	m_mapErrorDes[12456] = TEXT("重定向失败，因为方案更改（例如，HTTP到FTP）或所有尝试重定向尝试失败（默认为五次尝试）。");
	m_mapErrorDes[12168] = TEXT("重定向需要用户确认。");
	m_mapErrorDes[12047] = TEXT("应用程序无法启动异步线程。");
	m_mapErrorDes[12166] = TEXT("自动代理配置脚本中出现错误。");
	m_mapErrorDes[12010] = TEXT("提供给InternetQueryOption或 InternetSetOption的选项的长度 对于指定的选项类型不正确。");
	m_mapErrorDes[12022] = TEXT("找到了所需的注册表值，但是类型不正确或值无效。");
	m_mapErrorDes[12029] = TEXT("尝试连接到服务器失败。");
	m_mapErrorDes[12042] = TEXT("应用程序正在发布并尝试更改不安全的服务器上的多行文本。");
	m_mapErrorDes[12044] = TEXT("服务器正在请求客户端验证。");
	m_mapErrorDes[12046] = TEXT("此计算机上未设置客户端授权。");
	m_mapErrorDes[12030] = TEXT("与服务器的连接已终止。");
	m_mapErrorDes[12175] = TEXT("WinINet未能对响应执行内容解码。");
	m_mapErrorDes[12049] = TEXT("另一个线程正在进行密码对话框。");
	m_mapErrorDes[12163] = TEXT("互联网连接已经丢失。");
	m_mapErrorDes[12003] = TEXT("服务器返回了一个扩展错误。");
	m_mapErrorDes[12171] = TEXT("由于安全检查，该功能失败。");
	m_mapErrorDes[12032] = TEXT("该功能需要重做请求。");
	m_mapErrorDes[12054] = TEXT("所请求的资源需要Fortezza身份验证。");
	m_mapErrorDes[12036] = TEXT("请求失败，因为句柄已经存在。");
	m_mapErrorDes[12039] = TEXT("由于重定向，应用程序正从非SSL转换为SSL连接。");
	m_mapErrorDes[12052] = TEXT("提交给SSL连接的数据正被重定向到非SSL连接。");
	m_mapErrorDes[12040] = TEXT("由于重定向，应用程序正从SSL转移到非SSL连接。");
	m_mapErrorDes[12027] = TEXT("请求的格式无效。");
	m_mapErrorDes[12019] = TEXT("所请求的操作无法执行，因为所提供的手柄处于不正确的状态。");
	m_mapErrorDes[12018] = TEXT("此操作提供的句柄类型不正确。");

	m_mapErrorDes[12014] = TEXT("连接并登录到FTP服务器的请求无法完成，因为提供的密码不正确。");
	m_mapErrorDes[12013] = TEXT("连接并登录到FTP服务器的请求无法完成，因为提供的用户名不正确。");
	m_mapErrorDes[12053] = TEXT("该请求需要在CD-ROM驱动器中插入一张CD-ROM以查找请求的资源。");
	m_mapErrorDes[12004] = TEXT("发生内部错误。");
	m_mapErrorDes[12045] = TEXT("该功能不熟悉生成服务器证书的证书颁发机构。");
	m_mapErrorDes[12016] = TEXT("请求的操作无效。");
	m_mapErrorDes[12009] = TEXT("对InternetQueryOption或 InternetSetOption的请求 指定了无效的选项值。");
	m_mapErrorDes[12033] = TEXT("对代理的请求无效。");
	m_mapErrorDes[12005] = TEXT("该网址无效。");
	m_mapErrorDes[12028] = TEXT("请求的项目无法找到。");
	m_mapErrorDes[12015] = TEXT("连接并登录到FTP服务器的请求失败。");
	m_mapErrorDes[12174] = TEXT("MS-Logoff摘要标题已从网站返回。");
	m_mapErrorDes[12041] = TEXT("内容不完全安全。一些正在查看的内容可能来自不安全的服务器。");
	m_mapErrorDes[12007] = TEXT("服务器名称无法解析.");
	m_mapErrorDes[12173] = TEXT("目前尚未实施。");
	m_mapErrorDes[12034] = TEXT("用户界面或其他阻塞操作已被请求。");
	m_mapErrorDes[12025] = TEXT("由于尚未设置回调函数，因此无法执行异步请求。");
	m_mapErrorDes[12024] = TEXT("无法提供异步请求，因为提供了零上下文值。");
	m_mapErrorDes[12023] = TEXT("目前无法直接访问网络。");

	m_mapErrorDes[12172] = TEXT("没有发生WinINet API的初始化。");
	m_mapErrorDes[12020] = TEXT("该请求不能通过代理进行。");
	m_mapErrorDes[12017] = TEXT("该操作被取消，通常是因为在操作完成之前，请求所在的句柄已关闭。");
	m_mapErrorDes[12023] = TEXT("目前无法直接访问网络。");
	m_mapErrorDes[12011] = TEXT("请求的选项不能设置，只能查询。");
	m_mapErrorDes[12001] = TEXT("目前没有更多的句柄可以生成。");
	m_mapErrorDes[12043] = TEXT("该应用程序将数据发布到不安全的服务器。");
	m_mapErrorDes[12008] = TEXT("请求的协议无法找到。");
	m_mapErrorDes[12165] = TEXT("指定的代理服务器无法到达。");
	m_mapErrorDes[12048] = TEXT("该功能无法处理重定向，因为该方案已更改（例如，HTTP到FTP）。");
	m_mapErrorDes[12021] = TEXT("无法找到所需的注册表值。");
	m_mapErrorDes[12026] = TEXT("由于一个或多个请求正在等待，所需的操作无法完成。");
	m_mapErrorDes[12050] = TEXT("该对话框应该重试。");
	m_mapErrorDes[12038] = TEXT("SSL证书通用名称（主机名字段）不正确 ");
	m_mapErrorDes[12037] = TEXT("从服务器收到的SSL证书日期不正确。证书已过期。");

	m_mapErrorDes[12055] = TEXT("SSL证书包含错误。");
	m_mapErrorDes[12056] = TEXT("SSL证书未被吊销。");
	m_mapErrorDes[12057] = TEXT("撤消SSL证书失败。");
	m_mapErrorDes[12170] = TEXT("SSL证书已被吊销。");
	m_mapErrorDes[12169] = TEXT("SSL证书无效。");
	m_mapErrorDes[12157] = TEXT("应用程序在加载SSL库时遇到内部错误。");
	m_mapErrorDes[12164] = TEXT("指示的网站或服务器无法访问。");
	m_mapErrorDes[12012] = TEXT("WinINet支持正在关闭或卸载。");
	m_mapErrorDes[12159] = TEXT("所需的协议栈未加载，并且应用程序无法启动WinSock。");
	m_mapErrorDes[12002] = TEXT("请求已超时。");
	m_mapErrorDes[12158] = TEXT("该功能无法缓存文件。");
	m_mapErrorDes[12167] = TEXT("自动代理配置脚本无法下载。INTERNET_FLAG_MUST_CACHE_REQUEST标志被设置。");
	m_mapErrorDes[12006] = TEXT("URL方案无法识别或不受支持。");








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
