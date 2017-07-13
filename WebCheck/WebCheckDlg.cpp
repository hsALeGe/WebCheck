
// WebCheckDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WebCheck.h"
#include "WebCheckDlg.h"
#include "afxdialogex.h"
#include  <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//�ؼ���ʶ
#define IDC_WNDWEBLIST_CTRL				100

//ʱ���ʶ
#define IDI_TIME_CHECK_WEB				300
#define TIME_CHECK_WEB					5

CCriticalSection CWebCheckDlg::m_csUrlSection;
std::deque<WORD> CWebCheckDlg::m_urlIndexDeque;
CWebCheckDlg* CWebCheckDlg::_instance = NULL;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CWebCheckDlg �Ի���



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


// CWebCheckDlg ��Ϣ�������

BOOL CWebCheckDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	LoadConfigFun();
	m_webWnd.OnInsertUrlRecord();
	m_checkThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnCheckWebStatus, NULL, 0, 0);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CWebCheckDlg::LoadConfigFun()
{
	CString strFileDlgPath;
	TCHAR szModuleDirectory[MAX_PATH];	//ģ��Ŀ¼
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
	//����XML�Ƿ�ɹ�
	::CoInitialize(NULL);
	HRESULT HR = m_xmlDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60));
	if (!SUCCEEDED(HR))
	{
		ASSERT(false);
		return;
	}

	//����xml
	if (!m_xmlDoc->load(strPath.GetBuffer()))
	{
		//ASSERT(false);
		return;
	}

	MSXML2::IXMLDOMElementPtr xmlRoot;
	MSXML2::IXMLDOMNodeListPtr xmlNodes;//ĳ���ڵ�������ӽڵ�
	MSXML2::IXMLDOMNamedNodeMapPtr xmlNodeAtts;//ĳ���ڵ����������
	MSXML2::IXMLDOMNodePtr	  xmlNode;//ĳ�ӽڵ�	
	
	xmlRoot = m_xmlDoc->GetdocumentElement();		//��ȡ���ڵ�
	//��ȡ���и��ڵ���ӽڵ�
	xmlRoot->get_childNodes(&xmlNodes);

	LONG xmlNodeNum = 0, attsNum = 0;
	//�ӽڵ������
	xmlNodes->get_length(&xmlNodeNum);

	//�ڵ���Ϣ
	for (size_t i = 0; i < xmlNodeNum; i++)
	{
		//���ĳ���ӽڵ�
		xmlNodes->get_item(i, &xmlNode);
		//���ĳ���ڵ����������
		xmlNode->get_attributes(&xmlNodeAtts);
		//����������Եĸ���
		xmlNodeAtts->get_length(&attsNum);

		for (size_t j = 0; j < attsNum; j++)
		{
			//���ĳ������
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CWebCheckDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CWebCheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CWebCheckDlg::OnBnClickedBtnCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
		MessageBox(TEXT("��������Ч�����ӣ�"));
		return;
	}

	//��λ��
	if (m_uTimeElapse < 60) m_uTimeElapse = TIME_CHECK_WEB * 60;
	SetTimer(IDI_TIME_CHECK_WEB, m_uTimeElapse * 1000, NULL);

}


int CWebCheckDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	m_webWnd.Create(NULL, TEXT("WndWebList"), WS_VISIBLE | WS_CHILD | WS_VSCROLL, CRect(0,0,0,0), this, IDC_WNDWEBLIST_CTRL);

	return 0;
}


void CWebCheckDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (m_webWnd)
	{
		m_webWnd.MoveWindow(CRect(10, 18, 560, 300));
	}
}


void CWebCheckDlg::OnBnClickedBtnAdd()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
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
	// TODO: �ڴ����ר�ô����/����û���

	return CDialogEx::OnCommand(wParam, lParam);
}


void CWebCheckDlg::OnBnClickedBtnAbout()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CAboutDlg dlg;
	dlg.DoModal();
}
