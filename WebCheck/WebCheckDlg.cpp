
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
std::map<WORD, CString> CWebCheckDlg::m_mapErrorDes;


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
	LoadErrorDesc();
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
	KillTimer(IDI_TIME_CHECK_WEB);
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
	m_mapErrorDes[1] = TEXT("���ܴ���");
	m_mapErrorDes[2] = TEXT("ϵͳ�Ҳ���ָ�����ļ�");
	m_mapErrorDes[3] = TEXT("ϵͳ�Ҳ���ָ����·��");
	m_mapErrorDes[4] = TEXT("ϵͳ�޷����ļ�");
	m_mapErrorDes[5] = TEXT("�ܾ�����");
	m_mapErrorDes[6] = TEXT("�����Ч");
	m_mapErrorDes[7] = TEXT("�洢���ƿ鱻��");
	m_mapErrorDes[8] = TEXT("�洢�ռ䲻�㣬�޷����������");
	m_mapErrorDes[9] = TEXT("�洢���ƿ��ַ��Ч");
	m_mapErrorDes[10] = TEXT("��������");
	m_mapErrorDes[11] = TEXT("��ͼ���ظ�ʽ����ĳ���");
	m_mapErrorDes[12] = TEXT("��������Ч");
	m_mapErrorDes[13] = TEXT("������Ч");
	m_mapErrorDes[14] = TEXT("�洢�����㣬�޷���ɴ˲���");
	m_mapErrorDes[15] = TEXT("ϵͳ�Ҳ���ָ����������");
	m_mapErrorDes[16] = TEXT("�޷�ɾ��Ŀ¼");
	m_mapErrorDes[17] = TEXT("ϵͳ�޷����ļ��Ƶ���ͬ��������");
	m_mapErrorDes[18] = TEXT("û�и����ļ�");
	m_mapErrorDes[19] = TEXT("������д�뱣��");
	m_mapErrorDes[20] = TEXT("ϵͳ�Ҳ���ָ�����豸");
	m_mapErrorDes[21] = TEXT("�豸δ����");
	m_mapErrorDes[22] = TEXT("�豸��ʶ�������");
	m_mapErrorDes[23] = TEXT("���ݴ���(ѭ��������)");
	m_mapErrorDes[24] = TEXT("���򷢳����������Ȳ���ȷ");
	m_mapErrorDes[25] = TEXT("�������޷��ҳ��������ض������ŵ���λ��");
	m_mapErrorDes[26] = TEXT("�޷�����ָ���Ĵ��̻�����");
	m_mapErrorDes[27] = TEXT("�������Ҳ������������");
	m_mapErrorDes[28] = TEXT("��ӡ��ȱֽ");
	m_mapErrorDes[29] = TEXT("ϵͳ�޷�д��ָ�����豸");
	m_mapErrorDes[30] = TEXT("ϵͳ�޷���ָ�����豸�϶�ȡ");
	m_mapErrorDes[31] = TEXT("����ϵͳ�ϵ��豸û�з�������");
	m_mapErrorDes[32] = TEXT("�����޷������ļ�����Ϊ��һ����������ʹ�ô��ļ�");
	m_mapErrorDes[33] = TEXT("�����޷������ļ�����Ϊ��һ�������������ļ���һ����");
	m_mapErrorDes[36] = TEXT("��������Ĵ��ļ�����");
	m_mapErrorDes[38] = TEXT("�����ļ���β");
	m_mapErrorDes[39] = TEXT("��������");
	m_mapErrorDes[50] = TEXT("��֧�ָ�����");
	m_mapErrorDes[51] = TEXT("Զ�̼����������");
	m_mapErrorDes[52] = TEXT("�������������ظ�������");
	m_mapErrorDes[53] = TEXT("�Ҳ�������·��");
	m_mapErrorDes[54] = TEXT("����æ");
	m_mapErrorDes[55] = TEXT("ָ����������Դ���豸���ٿ���");
	m_mapErrorDes[56] = TEXT("�ѵ�������BIOS��������");
	m_mapErrorDes[57] = TEXT("����������Ӳ������");
	m_mapErrorDes[58] = TEXT("ָ���ķ������޷���������Ĳ���");
	m_mapErrorDes[59] = TEXT("����������������");
	m_mapErrorDes[60] = TEXT("Զ��������������");
	m_mapErrorDes[61] = TEXT("��ӡ����������");
	m_mapErrorDes[62] = TEXT("�޷��ڷ������ϻ�����ڱ������ӡ�ļ��Ŀռ�");
	m_mapErrorDes[63] = TEXT("ɾ���Ⱥ��ӡ���ļ�");
	m_mapErrorDes[64] = TEXT("ָ�������������ٿ���");
	m_mapErrorDes[65] = TEXT("�ܾ��������");
	m_mapErrorDes[66] = TEXT("������Դ���ʹ���");
	m_mapErrorDes[67] = TEXT("�Ҳ���������");
	m_mapErrorDes[68] = TEXT("�������ؼ������������������");
	m_mapErrorDes[69] = TEXT("��������BIOS�Ự����");
	m_mapErrorDes[70] = TEXT("Զ�̷���������ͣ������������������");
	m_mapErrorDes[71] = TEXT("��ǰ���޷���ͬ��Զ�̼�������ӣ���Ϊ�Ѵﵽ�������������Ŀ����");
	m_mapErrorDes[72] = TEXT("����ָͣ���Ĵ�ӡ��������豸");
	m_mapErrorDes[80] = TEXT("�ļ�����");
	m_mapErrorDes[82] = TEXT("�޷�����Ŀ¼���ļ�");
	m_mapErrorDes[83] = TEXT("INT24ʧ��");
	m_mapErrorDes[84] = TEXT("�޷�ȡ�ô��������Ĵ洢�ռ�");
	m_mapErrorDes[85] = TEXT("�����豸������ʹ����");
	m_mapErrorDes[86] = TEXT("ָ���������������");
	m_mapErrorDes[87] = TEXT("��������");
	m_mapErrorDes[88] = TEXT("�����Ϸ���д�����");
	m_mapErrorDes[89] = TEXT("ϵͳ�޷��ڴ�ʱ������һ������");
	m_mapErrorDes[100] = TEXT("�޷�������һ��ϵͳ�źŵ�");
	m_mapErrorDes[101] = TEXT("��һ������ӵ�ж�ռ���źŵ�");
	m_mapErrorDes[102] = TEXT("�������źŵ����޷��ر�");
	m_mapErrorDes[103] = TEXT("�޷��������źŵ�");
	m_mapErrorDes[104] = TEXT("�޷����ж�ʱ�����ռ���źŵ�");
	m_mapErrorDes[105] = TEXT("���źŵƵ�ǰһ������Ȩ�ѽ���");
	m_mapErrorDes[107] = TEXT("����ֹͣ����Ϊ���������δ����");
	m_mapErrorDes[108] = TEXT("������ʹ���У�����һ����������");
	m_mapErrorDes[109] = TEXT("�ܵ��ѽ���");
	m_mapErrorDes[110] = TEXT("ϵͳ�޷���ָ�����豸���ļ�");
	m_mapErrorDes[111] = TEXT("�ļ���̫��");
	m_mapErrorDes[112] = TEXT("���̿ռ䲻��");
	m_mapErrorDes[113] = TEXT("�޷��ٻ���ڲ��ļ��ı�ʶ");
	m_mapErrorDes[114] = TEXT("Ŀ���ڲ��ļ��ı�ʶ����ȷ");
	m_mapErrorDes[117] = TEXT("Ӧ�ó���������IOCTL���ô���");
	m_mapErrorDes[118] = TEXT("��֤д����л�����ֵ����");
	m_mapErrorDes[119] = TEXT("ϵͳ��֧�����������");
	m_mapErrorDes[120] = TEXT("�˹���ֻ����ϵͳ֧��");
	m_mapErrorDes[121] = TEXT("�źŵƳ�ʱʱ���ѵ�");
	m_mapErrorDes[122] = TEXT("���ݵ�ϵͳ���õ�������̫С");
	m_mapErrorDes[123] = TEXT("�ļ�����Ŀ¼�������﷨����ȷ");
	m_mapErrorDes[124] = TEXT("ϵͳ���ü������");
	m_mapErrorDes[125] = TEXT("����û�о��");
	m_mapErrorDes[126] = TEXT("�Ҳ���ָ����ģ��");
	m_mapErrorDes[127] = TEXT("�Ҳ���ָ���ĳ���");
	m_mapErrorDes[128] = TEXT("û�еȺ���ӽ���");
	m_mapErrorDes[130] = TEXT("��ͼʹ�ò���(����ԭʼ����I/O)���Ѵ򿪴��̷������ļ����");
	m_mapErrorDes[131] = TEXT("��ͼ�ƶ��ļ�ָ�뵽�ļ���ͷ֮ǰ");
	m_mapErrorDes[132] = TEXT("�޷���ָ�����豸���ļ��������ļ�ָ��");
	m_mapErrorDes[133] = TEXT("������ǰ�������������������޷�ʹ��JOIN��SUBST����");
	m_mapErrorDes[134] = TEXT("��ͼ���ѱ��ϲ�����������ʹ��JOIN��SUBST����");
	m_mapErrorDes[135] = TEXT("��ͼ���ѱ��ϲ�����������ʹ��JOIN��SUBST����");
	m_mapErrorDes[136] = TEXT("ϵͳ��ͼ���δ�ϲ���������JOIN");
	m_mapErrorDes[137] = TEXT("ϵͳ��ͼ���δ�����������SUBST");
	m_mapErrorDes[138] = TEXT("ϵͳ��ͼ���������ϲ����ϲ��������ϵ�Ŀ¼");
	m_mapErrorDes[139] = TEXT("ϵͳ��ͼ�����������Ϊ����������ϵ�Ŀ¼");
	m_mapErrorDes[140] = TEXT("ϵͳ��ͼ���������ϲ�������������ϵ�Ŀ¼");
	m_mapErrorDes[141] = TEXT("ϵͳ��ͼ���������Ϊ�ϲ��������ϵ�Ŀ¼");
	m_mapErrorDes[142] = TEXT("ϵͳ�޷��ڴ�ʱ����JOIN��SUBST");
	m_mapErrorDes[143] = TEXT("ϵͳ�޷����������ϲ��������Ϊ��ͬ�������ϵ�Ŀ¼");
	m_mapErrorDes[144] = TEXT("Ŀ¼���Ǹ�Ŀ¼�µ���Ŀ¼");
	m_mapErrorDes[145] = TEXT("Ŀ¼�ǿ�");
	m_mapErrorDes[146] = TEXT("ָ����·�����������ʹ��");
	m_mapErrorDes[147] = TEXT("��Դ���㣬�޷����������");
	m_mapErrorDes[148] = TEXT("ָ����·���޷��ڴ�ʱʹ��");
	m_mapErrorDes[149] = TEXT("��ͼ���������ϲ������Ϊ��������Ŀ¼����һ�������Ŀ���������");
	m_mapErrorDes[150] = TEXT("ϵͳ������Ϣδ��CONFIG.SYS�ļ���ָ�������������");
	m_mapErrorDes[151] = TEXT("ΪDosMuxSemWaitָ�����źŵ��¼���������");
	m_mapErrorDes[152] = TEXT("DosMuxSemWait�������������ù�����źŵ�");
	m_mapErrorDes[153] = TEXT("DosMuxSemWait�嵥����");
	m_mapErrorDes[154] = TEXT("����ľ�곬��Ŀ���ļ�ϵͳ�ĳ�������");
	m_mapErrorDes[155] = TEXT("�޷�������һ���߳�");
	m_mapErrorDes[156] = TEXT("���ս����Ѿܾ����ź�");
	m_mapErrorDes[157] = TEXT("���ѱ��������޷�����");
	m_mapErrorDes[158] = TEXT("���ѽ������");
	m_mapErrorDes[159] = TEXT("�̱߳�ʶ�ĵ�ַ����");
	m_mapErrorDes[160] = TEXT("���ݵ�DosExecPgm�Ĳ����ַ�������");
	m_mapErrorDes[161] = TEXT("ָ����·����Ч");
	m_mapErrorDes[162] = TEXT("�ź�����ͣ");
	m_mapErrorDes[164] = TEXT("�޷���ϵͳ�д���������߳�");
	m_mapErrorDes[167] = TEXT("�޷������ļ�����");
	m_mapErrorDes[170] = TEXT("�������Դ��ʹ����");
	m_mapErrorDes[173] = TEXT("�����ṩȡ�����������������������");
	m_mapErrorDes[174] = TEXT("�ļ�ϵͳ��֧���������͵���С��Ԫ����");
	m_mapErrorDes[180] = TEXT("ϵͳ��������Ķκ�");
	m_mapErrorDes[183] = TEXT("���ļ��Ѵ���ʱ���޷��������ļ�");
	m_mapErrorDes[186] = TEXT("���ݵı�־����");
	m_mapErrorDes[187] = TEXT("�Ҳ���ָ����ϵͳ�źŵ�����");
	m_mapErrorDes[196] = TEXT("����ϵͳ�޷����д�Ӧ�ó���");
	m_mapErrorDes[197] = TEXT("����ϵͳ��ǰ�����ò������д�Ӧ�ó���");
	m_mapErrorDes[199] = TEXT("����ϵͳ�޷����д�Ӧ�ó���");
	m_mapErrorDes[200] = TEXT("����β��ɴ��ڻ����64K");
	m_mapErrorDes[203] = TEXT("����ϵͳ�Ҳ���������Ļ���ѡ��");
	m_mapErrorDes[205] = TEXT("���������еĽ���û���źŴ������");
	m_mapErrorDes[206] = TEXT("�ļ�������չ��̫��");
	m_mapErrorDes[207] = TEXT("��2����ջ�ѱ�ռ��");
	m_mapErrorDes[208] = TEXT("û����ȷ�����ļ���ͨ���*��?����ָ��������ļ���ͨ���");
	m_mapErrorDes[209] = TEXT("���ڷ��͵��źŴ���");
	m_mapErrorDes[210] = TEXT("�޷������źŴ������");
	m_mapErrorDes[212] = TEXT("�����������޷����·���");
	m_mapErrorDes[214] = TEXT("�����ó����̬����ģ��Ķ�̬����ģ��̫��");
	m_mapErrorDes[215] = TEXT("�޷�Ƕ�׵���LoadModule");
	m_mapErrorDes[230] = TEXT("�ܵ�״̬��Ч");
	m_mapErrorDes[231] = TEXT("���еĹܵ�ʵ������ʹ����");
	m_mapErrorDes[232] = TEXT("�ܵ����ڹر���");
	m_mapErrorDes[233] = TEXT("�ܵ�����һ�������κν���");
	m_mapErrorDes[234] = TEXT("�������ݿ���");
	m_mapErrorDes[240] = TEXT("ȡ���Ự");
	m_mapErrorDes[254] = TEXT("ָ������չ��������Ч");
	m_mapErrorDes[255] = TEXT("��չ���Բ�һ��");
	m_mapErrorDes[258] = TEXT("�ȴ��Ĳ�����ʱ");
	m_mapErrorDes[259] = TEXT("û�п��õ�������");
	m_mapErrorDes[266] = TEXT("�޷�ʹ�ø��ƹ���");
	m_mapErrorDes[267] = TEXT("Ŀ¼����Ч");
	m_mapErrorDes[275] = TEXT("��չ�����ڻ������в�����");
	m_mapErrorDes[276] = TEXT("װ���ļ�ϵͳ�ϵ���չ�����ļ�����");
	m_mapErrorDes[277] = TEXT("��չ���Ա���ļ�����");
	m_mapErrorDes[278] = TEXT("ָ������չ���Ծ����Ч");
	m_mapErrorDes[282] = TEXT("װ����ļ�ϵͳ��֧����չ����");
	m_mapErrorDes[288] = TEXT("��ͼ�ͷŲ��Ǻ��з���ӵ�еĶ��û��ն����г���");
	m_mapErrorDes[298] = TEXT("�����źŵƵ��������");
	m_mapErrorDes[299] = TEXT("����ɲ��ֵ�ReadProcessMemoty��WriteProcessMemory����");
	m_mapErrorDes[300] = TEXT("�����������󱻾ܾ�");
	m_mapErrorDes[301] = TEXT("ϵͳ������һ����Ч�Ĳ�������ȷ��");
	m_mapErrorDes[487] = TEXT("��ͼ������Ч�ĵ�ַ");
	m_mapErrorDes[534] = TEXT("�����������32λ");
	m_mapErrorDes[535] = TEXT("�ܵ�����һ����һ����");
	m_mapErrorDes[536] = TEXT("�Ⱥ�򿪹ܵ���һ�˵Ľ���");
	m_mapErrorDes[994] = TEXT("�ܾ�������չ����");
	m_mapErrorDes[995] = TEXT("�����߳��˳���Ӧ�ó��������ѷ���I/O����");
	m_mapErrorDes[996] = TEXT("�ص�I/O�¼������ź�״̬��");
	m_mapErrorDes[997] = TEXT("�ص�I/O�����ڽ�����");
	m_mapErrorDes[998] = TEXT("�ڴ���������Ч");
	m_mapErrorDes[999] = TEXT("��������ҳ�ڲ���");
	m_mapErrorDes[1001] = TEXT("�ݹ�̫�ջ���");
	m_mapErrorDes[1002] = TEXT("�����޷����ѷ��͵���Ϣ�ϲ���");
	m_mapErrorDes[1003] = TEXT("�޷���ɴ˹���");
	m_mapErrorDes[1004] = TEXT("��Ч��־");
	m_mapErrorDes[1005] = TEXT("�˾�������ʶ����ļ�ϵͳ ��ȷ������������ļ�ϵͳ���������Ѽ��أ��Ҵ˾�δ��");
	m_mapErrorDes[1006] = TEXT("�ļ����ڵľ��ѱ��ⲿ�ı䣬��˴򿪵��ļ�������Ч");
	m_mapErrorDes[1007] = TEXT("�޷���ȫ��Ļģʽ����������Ĳ���");
	m_mapErrorDes[1008] = TEXT("��ͼ���ò����ڵ�����");
	m_mapErrorDes[1009] = TEXT("����ע������ݿ���");
	m_mapErrorDes[1010] = TEXT("����ע�������Ч");
	m_mapErrorDes[1011] = TEXT("�޷�������ע�����");
	m_mapErrorDes[1012] = TEXT("�޷���ȡ����ע�����");
	m_mapErrorDes[1013] = TEXT("�޷�д������ע�����");
	m_mapErrorDes[1014] = TEXT("ע������ݿ��е�ĳһ�ļ�����ʹ�ü�¼������������ָ��ָ��ɹ����");
	m_mapErrorDes[1015] = TEXT("ע����� ����ע������ݵ�ĳһ�ļ��ṹ�𻵣���ϵͳ���ļ��ڴ�ӳ���𻵣�����Ϊ�����������־ȱ�ٻ��𻵶��޷��ָ��ļ�");
	m_mapErrorDes[1016] = TEXT("��ע���������I/O�����ָ�ʧ�� ע����޷����롢д�����������һ������ע���ϵͳӳ����ļ�");
	m_mapErrorDes[1017] = TEXT("ϵͳ��ͼ���ػ�ԭ�ļ���ע�����ָ�����ļ�����ע����ļ���ʽ");
	m_mapErrorDes[1018] = TEXT("��ͼ�ڱ��Ϊɾ����ע����������в��Ϸ��Ĳ���");
	m_mapErrorDes[1019] = TEXT("ϵͳ�޷�����ע�����־��������Ŀռ�");
	m_mapErrorDes[1020] = TEXT("�޷������������ֵ��ע������д�����������");
	m_mapErrorDes[1021] = TEXT("�޷����ױ丸���´����ȶ�����");
	m_mapErrorDes[1022] = TEXT("֪ͨ����������������У�����Ϣ��δ���ص����з��Ļ������� ��ǰ���з�����ö���ļ������Ҹ���");
	m_mapErrorDes[1051] = TEXT("�ѷ���ֹͣ���Ƶ����񣬸÷��������������еķ���������");
	m_mapErrorDes[1052] = TEXT("����Ŀؼ��Դ˷�����Ч");
	m_mapErrorDes[1053] = TEXT("����δ��ʱ��Ӧ�������������");
	m_mapErrorDes[1054] = TEXT("�޷������˷�����߳�");
	m_mapErrorDes[1055] = TEXT("�����������ݿ�");
	m_mapErrorDes[1056] = TEXT("�����ʵ������������");
	m_mapErrorDes[1057] = TEXT("�ʻ�����Ч�򲻴��ڣ������������ָ�����ʻ�����Ч");
	m_mapErrorDes[1058] = TEXT("�޷���������ԭ��������������û�������������豸û������");
	m_mapErrorDes[1059] = TEXT("ָ����ѭ����������");
	m_mapErrorDes[1060] = TEXT("ָ���ķ���δ���Ѱ�װ�ķ������");
	m_mapErrorDes[1061] = TEXT("�����޷��ڴ�ʱ���ܿ�����Ϣ");
	m_mapErrorDes[1062] = TEXT("����δ����");
	m_mapErrorDes[1063] = TEXT("��������޷����ӵ������������");
	m_mapErrorDes[1064] = TEXT("�������������ʱ���ڷ����з����쳣");
	m_mapErrorDes[1065] = TEXT("ָ�������ݿⲻ����");
	m_mapErrorDes[1066] = TEXT("�����ѷ����ض��ķ��������");
	m_mapErrorDes[1067] = TEXT("����������ֹ");
	m_mapErrorDes[1068] = TEXT("�����������޷�����");
	m_mapErrorDes[1069] = TEXT("���ڵ�¼ʧ�ܶ��޷���������");
	m_mapErrorDes[1070] = TEXT("�����󣬷���ͣ����������ͣ״̬");
	m_mapErrorDes[1071] = TEXT("ָ���ķ������ݿ�������Ч");
	m_mapErrorDes[1072] = TEXT("ָ���ķ����ѱ��Ϊɾ��");
	m_mapErrorDes[1073] = TEXT("ָ���ķ����Ѵ���");
	m_mapErrorDes[1074] = TEXT("ϵͳ��ǰ�����µ���Ч��������");
	m_mapErrorDes[1075] = TEXT("������񲻴��ڣ����ѱ����Ϊɾ��");
	m_mapErrorDes[1076] = TEXT("�ѽ���ʹ�õ�ǰ������Ϊ������Ч��������");
	m_mapErrorDes[1077] = TEXT("�ϴ�����֮����δ������������");
	m_mapErrorDes[1078] = TEXT("�����������������������ʾ��");
	m_mapErrorDes[1079] = TEXT("�˷�����ʻ���ͬ��������ͬһ�����ϵ�����������ʻ�");
	m_mapErrorDes[1080] = TEXT("ֻ��ΪWin32��������ʧ�ܲ���������Ϊ������������");
	m_mapErrorDes[1081] = TEXT("������������еĴ���ͷ�����ƹ�������ͬ ���ԣ�������������������ֹ�Ļ���������ƹ������޷������κβ���");
	m_mapErrorDes[1082] = TEXT("���������δ���ûָ�����");
	m_mapErrorDes[1083] = TEXT("���ó��ڸÿ�ִ�г��������е����������ִ�и÷���");
	m_mapErrorDes[1100] = TEXT("�Ѵ�Ŵ���ʵ�ʽ�β");
	m_mapErrorDes[1101] = TEXT("�Ŵ������Ѵ��ļ����");
	m_mapErrorDes[1102] = TEXT("�Ѵ�Ŵ�����̷����Ŀ�ͷ");
	m_mapErrorDes[1103] = TEXT("�Ŵ������Ѵ�һ���ļ��Ľ�β");
	m_mapErrorDes[1104] = TEXT("�Ŵ��ϲ������κ�����");
	m_mapErrorDes[1105] = TEXT("�Ŵ��޷�����");
	m_mapErrorDes[1106] = TEXT("�ڷ��ʶ��������´Ŵ�ʱ����ǰ�Ŀ��С����ȷ");
	m_mapErrorDes[1107] = TEXT("�����شŴ�ʱ���Ҳ���������Ϣ");
	m_mapErrorDes[1108] = TEXT("�޷�����ý�嵯������");
	m_mapErrorDes[1109] = TEXT("�޷�ж�ؽ���");
	m_mapErrorDes[1110] = TEXT("�������еĽ��ʿ����Ѹ���");
	m_mapErrorDes[1111] = TEXT("��λI/O����");
	m_mapErrorDes[1112] = TEXT("��������û��ý��");
	m_mapErrorDes[1113] = TEXT("�ڶ��ֽڵ�Ŀ�����ҳ�У�û�д�Unicode�ַ�����ӳ�䵽���ַ�");
	m_mapErrorDes[1114] = TEXT("��̬���ӿ�(DLL)��ʼ������ʧ��");
	m_mapErrorDes[1115] = TEXT("ϵͳ�ػ����ڽ���");
	m_mapErrorDes[1116] = TEXT("��Ϊû���κν����еĹػ����̣������޷��ж�ϵͳ�ػ�");
	m_mapErrorDes[1117] = TEXT("��ΪI/O�豸���������޷����д�������");
	m_mapErrorDes[1118] = TEXT("û�д����豸����ʼ���ɹ� ������������ж��");
	m_mapErrorDes[1119] = TEXT("�޷��������������豸�����ж�����(IRQ)���豸 ������һ��ʹ�ø�IRQ�������豸�Ѵ�");
	m_mapErrorDes[1120] = TEXT("����I/O����������һ�����пڵ�д�����(IOCTL_SERIAL_XOFF_COUNTER�Ѵ���");
	m_mapErrorDes[1121] = TEXT("��Ϊ�ѹ���ʱʱ�䣬���Դ���I/O�������IOCTL_SERIAL_XOFF_COUNTERδ����");
	m_mapErrorDes[1122] = TEXT("���������Ҳ���ID��ַ���");
	m_mapErrorDes[1123] = TEXT("��������ID�ַ��������̿������ŵ���ַ�����");
	m_mapErrorDes[1124] = TEXT("���̿�����������������������ʶ��Ĵ���");
	m_mapErrorDes[1125] = TEXT("���̿�������������Ĵ����в�һ�µĽ��");
	m_mapErrorDes[1126] = TEXT("������Ӳ��ʱ������У׼����ʧ�ܣ�������Ȼʧ��");
	m_mapErrorDes[1127] = TEXT("������Ӳ��ʱ�����̲���ʧ�ܣ�������Ȼʧ��");
	m_mapErrorDes[1128] = TEXT("������Ӳ��ʱ����ʹʧ�ܣ����븴λ���̿�����");
	m_mapErrorDes[1129] = TEXT("�Ѵ�Ŵ���β");
	m_mapErrorDes[1130] = TEXT("�������洢�ռ䲻�㣬�޷����������");
	m_mapErrorDes[1131] = TEXT("����Ǳ�ڵ�����״̬");
	m_mapErrorDes[1132] = TEXT("ָ���Ļ�ַ���ļ�ƫ����û���ʵ�����");
	m_mapErrorDes[1140] = TEXT("�ı�ϵͳ����״̬�ĳ��Ա���һӦ�ó��������������");
	m_mapErrorDes[1141] = TEXT("ϵͳBIOS�ı�ϵͳ����״̬�ĳ���ʧ��");
	m_mapErrorDes[1142] = TEXT("��ͼ��һ�ļ��ϴ�������ϵͳ�������������");
	m_mapErrorDes[1150] = TEXT("ָ������Ҫ����µ�Windows�汾");
	m_mapErrorDes[1151] = TEXT("ָ��������Windows��MS=DOS����");
	m_mapErrorDes[1152] = TEXT("ֻ��������ָ�������һ��ʵ��");
	m_mapErrorDes[1153] = TEXT("��ָ�����������ھɵ�Windows�汾");
	m_mapErrorDes[1154] = TEXT("ִ�и�Ӧ�ó�������Ŀ��ļ�֮һ����");
	m_mapErrorDes[1155] = TEXT("û��Ӧ�ó�����˲�����ָ���ļ��й���");
	m_mapErrorDes[1156] = TEXT("������ָ�Ӧ�ó���Ĺ����г��ִ���");
	m_mapErrorDes[1157] = TEXT("ִ�и�Ӧ�ó�������Ŀ��ļ�֮һ�޷��ҵ�");
	m_mapErrorDes[1158] = TEXT("��ǰ������ʹ����Window�����������ϵͳ��������о��");
	m_mapErrorDes[1159] = TEXT("��Ϣֻ����ͬ������һ��ʹ��");
	m_mapErrorDes[1160] = TEXT("ָ����ԴԪ��û��ý��");
	m_mapErrorDes[1161] = TEXT("ָ����Ŀ��Ԫ���Ѱ���ý��");
	m_mapErrorDes[1162] = TEXT("ָ����Ԫ�ز�����");
	m_mapErrorDes[1163] = TEXT("ָ����Ԫ����δ��ʾ�Ĵ洢��Դ��һ����");
	m_mapErrorDes[1164] = TEXT("��ʾ�豸��Ҫ���³�ʼ������ΪӲ���д���");
	m_mapErrorDes[1165] = TEXT("�豸��ʾ�ڳ��Խ�һ������֮ǰ��Ҫ���");
	m_mapErrorDes[1166] = TEXT("�豸��ʾ���������Ǵ�״̬");
	m_mapErrorDes[1167] = TEXT("�豸û������");
	m_mapErrorDes[1168] = TEXT("�Ҳ���Ԫ��");
	m_mapErrorDes[1169] = TEXT("������û��ָͬ������ƥ�����");
	m_mapErrorDes[1170] = TEXT("�ڶ����ϲ�����ָ�������Լ�");
	m_mapErrorDes[1171] = TEXT("���ݵ�GetMouseMovePoints�ĵ㲻�ڻ�������");
	m_mapErrorDes[1172] = TEXT("����(����վ)����û����");
	m_mapErrorDes[1173] = TEXT("�Ҳ�����ID");
	m_mapErrorDes[1175] = TEXT("�޷�ɾ��Ҫ���滻���ļ�");
	m_mapErrorDes[1176] = TEXT("�޷����滻�ļ��Ƶ�Ҫ���滻���ļ� Ҫ���滻���ļ�����ԭ��������");
	m_mapErrorDes[1177] = TEXT("�޷����滻�ļ��Ƶ�Ҫ���滻���ļ� Ҫ���滻���ļ��ѱ���������Ϊ��������");
	m_mapErrorDes[1178] = TEXT("����ļ�¼��ɾ��");
	m_mapErrorDes[1179] = TEXT("����ļ�¼���񲻴��ڻ��");
	m_mapErrorDes[1180] = TEXT("�ҵ�һ���ļ������ǿ��ܲ�����ȷ���ļ�");
	m_mapErrorDes[1181] = TEXT("��־�����־�б�ɾ��");
	m_mapErrorDes[1200] = TEXT("ָ�����豸����Ч");
	m_mapErrorDes[1201] = TEXT("�豸��ǰδ�����ϣ�����Ϊһ����¼����");
	m_mapErrorDes[1202] = TEXT("��ͼ��¼��ǰ�ѱ���¼���豸");
	m_mapErrorDes[1203] = TEXT("���κ������ṩ�������ָ��������·��");
	m_mapErrorDes[1204] = TEXT("ָ���������ṩ����������Ч");
	m_mapErrorDes[1205] = TEXT("�޷����������������ļ�");
	m_mapErrorDes[1206] = TEXT("�������������ļ���");
	m_mapErrorDes[1207] = TEXT("�޷�ö�ٿ�����");
	m_mapErrorDes[1208] = TEXT("������չ����");
	m_mapErrorDes[1209] = TEXT("ָ����������ʽ��Ч");
	m_mapErrorDes[1210] = TEXT("ָ���ļ��������ʽ��Ч");
	m_mapErrorDes[1211] = TEXT("ָ�����¼�����ʽ��Ч");
	m_mapErrorDes[1212] = TEXT("ָ����������ʽ��Ч");
	m_mapErrorDes[1213] = TEXT("ָ���ķ�������ʽ��Ч");
	m_mapErrorDes[1214] = TEXT("ָ������������ʽ��Ч");
	m_mapErrorDes[1215] = TEXT("ָ���Ĺ�������ʽ��Ч");
	m_mapErrorDes[1216] = TEXT("ָ���������ʽ��Ч");
	m_mapErrorDes[1217] = TEXT("ָ������Ϣ����ʽ��Ч");
	m_mapErrorDes[1218] = TEXT("ָ������ϢĿ���ʽ��Ч");
	m_mapErrorDes[1219] = TEXT("�ṩ��ƾ�����Ѵ��ڵ�ƾ�ݼ���ͻ");
	m_mapErrorDes[1220] = TEXT("��ͼ��������������ĻỰ�����ѶԸ÷�������������ĻỰ");
	m_mapErrorDes[1221] = TEXT("��������������������ϵ���һ�������ʹ��");
	m_mapErrorDes[1222] = TEXT("����δ���ӻ�����");
	m_mapErrorDes[1223] = TEXT("�����ѱ��û�ȡ��");
	m_mapErrorDes[1224] = TEXT("����Ĳ����޷���ʹ���û�ӳ������򿪵��ļ���ִ��");
	m_mapErrorDes[1225] = TEXT("Զ��ϵͳ�ܾ���������");
	m_mapErrorDes[1226] = TEXT("���������ѱ��ʵ��عر���");
	m_mapErrorDes[1227] = TEXT("���紫���ս��������������ĵ�ַ");
	m_mapErrorDes[1228] = TEXT("��ַ��δ�������ս�����");
	m_mapErrorDes[1229] = TEXT("��ͼ�ڲ����ڵ����������Ͻ��в���");
	m_mapErrorDes[1230] = TEXT("��ͼ��ʹ���е����������Ͻ�����Ч�Ĳ���");
	m_mapErrorDes[1231] = TEXT("���ܷ�������λ�� �й������ų����ϵ���Ϣ�������Windows����");
	m_mapErrorDes[1232] = TEXT("���ܷ�������λ�� �й������ų����ϵ���Ϣ�������Windows����");
	m_mapErrorDes[1233] = TEXT("���ܷ�������λ�� �й������ų����ϵ���Ϣ�������Windows����");
	m_mapErrorDes[1234] = TEXT("û���κη�������Զ��ϵͳ�ϵ�Ŀ�������ս���ϲ���");
	m_mapErrorDes[1235] = TEXT("������ֹ");
	m_mapErrorDes[1236] = TEXT("�ɱ���ϵͳ��ֹ��������");
	m_mapErrorDes[1237] = TEXT("�����޷���� Ӧ������");
	m_mapErrorDes[1238] = TEXT("��Ϊ�Ѵﵽ���ʻ������ͬʱ���������ƣ������޷����ӷ�����");
	m_mapErrorDes[1239] = TEXT("��ͼ������ʻ�δ����Ȩ��ʱ���ڵ�¼");
	m_mapErrorDes[1240] = TEXT("���ʻ���δ�õ����������վ��¼����Ȩ");
	m_mapErrorDes[1241] = TEXT("����Ĳ�������ʹ����������ַ");
	m_mapErrorDes[1242] = TEXT("�������Ѿ�ע��");
	m_mapErrorDes[1243] = TEXT("ָ���ķ��񲻴���");
	m_mapErrorDes[1244] = TEXT("��Ϊ�û���δ����֤������ִ����Ҫ��Ĳ���");
	m_mapErrorDes[1245] = TEXT("��Ϊ�û���δ��¼���磬����ִ����Ҫ��Ĳ��� ָ���ķ��񲻴���");
	m_mapErrorDes[1246] = TEXT("���ڼ�������");
	m_mapErrorDes[1247] = TEXT("��ͼ���г�ʼ���������ǳ�ʼ�������");
	m_mapErrorDes[1248] = TEXT("û�и���ı����豸");
	m_mapErrorDes[1249] = TEXT("ָ����վ�㲻����");
	m_mapErrorDes[1250] = TEXT("����ָ�����Ƶ���������Ѿ�����");
	m_mapErrorDes[1251] = TEXT("ֻ�����ӵ���������ʱ���ò�������֧��");
	m_mapErrorDes[1252] = TEXT("��ʹû�иĶ�������Կ��ҲӦ�õ�����չ");
	m_mapErrorDes[1253] = TEXT("ָ�����û�û��һ����Ч�������ļ�");
	m_mapErrorDes[1254] = TEXT("MicrosoftSmallBusinessServer��֧�ִ˲���");
	m_mapErrorDes[1300] = TEXT("�������б����õ���Ȩ��ָ�ɸ����з�");
	m_mapErrorDes[1301] = TEXT("�ʻ����Ͱ�ȫ��ʶ���ĳЩӳ��δ���");
	m_mapErrorDes[1302] = TEXT("û��Ϊ���ʻ��ر�����ϵͳ�������");
	m_mapErrorDes[1303] = TEXT("û�п��õļ�����Կ ������һ����֪������Կ");
	m_mapErrorDes[1304] = TEXT("����̫���ӣ��޷�ת����LANManager���� ���ص�LANManager����Ϊ���ַ���");
	m_mapErrorDes[1305] = TEXT("�޶�����δ֪");
	m_mapErrorDes[1306] = TEXT("���������޶������ǲ����ݵ�");
	m_mapErrorDes[1307] = TEXT("�����ȫ��ʶ����ָ��Ϊ�˶����������");
	m_mapErrorDes[1308] = TEXT("�����ȫ��ʶ����ָ��Ϊ�������Ҫ��");
	m_mapErrorDes[1309] = TEXT("��ǰ��δģ��ͻ����߳���ͼ����ģ������");
	m_mapErrorDes[1310] = TEXT("�����δ������");
	m_mapErrorDes[1311] = TEXT("��ǰû�п��õĵ�¼�������������¼����");
	m_mapErrorDes[1312] = TEXT("ָ���ĵ�¼�Ự������ �����ѱ���ֹ");
	m_mapErrorDes[1313] = TEXT("ָ������Ȩ������");
	m_mapErrorDes[1314] = TEXT("�ͻ�û���������Ȩ");
	m_mapErrorDes[1315] = TEXT("�ṩ�����Ʋ�����ȷ���ʻ�����ʽ");
	m_mapErrorDes[1316] = TEXT("ָ�����û��Ѵ���");
	m_mapErrorDes[1317] = TEXT("ָ�����û�������");
	m_mapErrorDes[1318] = TEXT("ָ�������Ѵ���");
	m_mapErrorDes[1319] = TEXT("ָ�����鲻����");
	m_mapErrorDes[1320] = TEXT("ָ�����û��ʻ�����ָ����ĳ�Ա��������Ϊ�������Ա�����޷�ɾ��ָ������");
	m_mapErrorDes[1321] = TEXT("ָ�����û��ʻ�����ָ�����ʻ��ĳ�Ա");
	m_mapErrorDes[1322] = TEXT("�޷����û�ɾ�����ʣ���ϵͳ�����ʻ�");
	m_mapErrorDes[1323] = TEXT("�޷��������� �ṩ��Ϊ��ǰ�����ֵ����ȷ");
	m_mapErrorDes[1324] = TEXT("�޷��������� �ṩ���������ֵ���������в������ֵ");
	m_mapErrorDes[1325] = TEXT("�޷��������� Ϊ�������ṩ��ֵ�������ַ���ĳ��ȡ������Ի���ʷҪ��");
	m_mapErrorDes[1326] = TEXT("��¼ʧ��:δ֪���û������������");
	m_mapErrorDes[1327] = TEXT("��¼ʧ��:�û��ʻ�����");
	m_mapErrorDes[1328] = TEXT("��¼ʧ��:Υ���ʻ���¼ʱ������");
	m_mapErrorDes[1329] = TEXT("��¼ʧ��:�������û���¼���˼����");
	m_mapErrorDes[1330] = TEXT("��¼ʧ��:ָ�����ʻ������ѹ���");
	m_mapErrorDes[1331] = TEXT("��¼ʧ��:���õ�ǰ���ʻ�");
	m_mapErrorDes[1332] = TEXT("�ʻ����밲ȫ��ʶ�����κ�ӳ�����");
	m_mapErrorDes[1333] = TEXT("һ���������ı����û���ʶ��(LUIDs)");
	m_mapErrorDes[1334] = TEXT("�޸�����õı����û���ʶ��(LUIDs)");
	m_mapErrorDes[1335] = TEXT("���ڸ��ر��÷�����ȫID�Ĵμ���Ȩ������Ч");
	m_mapErrorDes[1336] = TEXT("���ʿ����б�(ACL)�ṹ��Ч");
	m_mapErrorDes[1337] = TEXT("��ȫID�ṹ��Ч");
	m_mapErrorDes[1338] = TEXT("��ȫ�������ṹ��Ч");
	m_mapErrorDes[1340] = TEXT("�޷��������еķ��ʿ����б�(ACL)����ʿ�����Ŀ(ACE)");
	m_mapErrorDes[1341] = TEXT("��������ǰ�ѽ���");
	m_mapErrorDes[1342] = TEXT("��������ǰ������");
	m_mapErrorDes[1343] = TEXT("�ṩ��ʶ����Ű䷢������ֵΪ��Чֵ");
	m_mapErrorDes[1344] = TEXT("�޸�����õ��ڴ��Ը��°�ȫ��Ϣ");
	m_mapErrorDes[1345] = TEXT("ָ��������Ч����������Ⱥ������Բ�����");
	m_mapErrorDes[1346] = TEXT("ָ����ģ�⼶����Ч�������ṩ��ģ�⼶����Ч");
	m_mapErrorDes[1347] = TEXT("�޷�����������ȫ����");
	m_mapErrorDes[1348] = TEXT("�������֤��Ϣ�����Ч");
	m_mapErrorDes[1349] = TEXT("���Ƶ����Ͷ��䳢��ʹ�õķ������ʵ�");
	m_mapErrorDes[1350] = TEXT("�޷����밲ȫ���޹����Ķ��������а�ȫ�Բ���");
	m_mapErrorDes[1351] = TEXT("δ�ܴ����������ȡ������Ϣ����������Ϊ��������ʹ�ã������Ƿ��ʱ��ܾ�");
	m_mapErrorDes[1352] = TEXT("��ȫ�ʻ�������(SAM)�򱾵ذ�ȫ�䷢����(LSA)�������������а�ȫ�����Ĵ���״̬");
	m_mapErrorDes[1353] = TEXT("�������а�ȫ�����Ĵ���״̬");
	m_mapErrorDes[1354] = TEXT("�˲���ֻ�������Ҫ�����������");
	m_mapErrorDes[1355] = TEXT("ָ�����򲻴��ڣ����޷���ϵ");
	m_mapErrorDes[1356] = TEXT("ָ�������Ѵ���");
	m_mapErrorDes[1357] = TEXT("��ͼ����ÿ�����������������");
	m_mapErrorDes[1358] = TEXT("�޷���������������Ϊ�����ϵ����ؽ���ʧ�ܻ����ݽṹ��");
	m_mapErrorDes[1359] = TEXT("�������ڲ�����");
	m_mapErrorDes[1360] = TEXT("ͨ�÷������Ͱ�������ӳ�䵽��ͨ�����͵ķ���������");
	m_mapErrorDes[1361] = TEXT("��ȫ��������ʽ����ȷ(���Ի�����ص�)");
	m_mapErrorDes[1362] = TEXT("�������ֻ�����ڵ�¼������ʹ�� ���ý���δע��Ϊһ����¼����");
	m_mapErrorDes[1363] = TEXT("�޷�ʹ������ʹ���еı�ʶ�����µĻỰ");
	m_mapErrorDes[1364] = TEXT("δ֪��ָ����֤���ݰ�");
	m_mapErrorDes[1365] = TEXT("��¼�Ự���Ǵ������������һ�µ�״̬��");
	m_mapErrorDes[1366] = TEXT("��¼�Ự��ʶ����ʹ����");
	m_mapErrorDes[1367] = TEXT("��¼���������Ч�ĵ�¼����ֵ");
	m_mapErrorDes[1368] = TEXT("��ʹ�������ܵ���ȡ����֮ǰ���޷����ɸùܵ�ģ��");
	m_mapErrorDes[1369] = TEXT("ע���������������״̬������״̬��һ��");
	m_mapErrorDes[1370] = TEXT("��ȫ�����ݿ��ڲ�������");
	m_mapErrorDes[1371] = TEXT("�޷��������ʻ������д˲���");
	m_mapErrorDes[1372] = TEXT("�޷������������������д˲���");
	m_mapErrorDes[1373] = TEXT("�޷������������û������д˲���");
	m_mapErrorDes[1374] = TEXT("�޷�������ɾ���û�����Ϊ��ǰ��Ϊ�û�����Ҫ��");
	m_mapErrorDes[1375] = TEXT("��������Ϊ��Ҫ����ʹ��");
	m_mapErrorDes[1376] = TEXT("ָ���ı����鲻����");
	m_mapErrorDes[1377] = TEXT("ָ�����ʻ������Ǳ�����ĳ�Ա");
	m_mapErrorDes[1378] = TEXT("ָ�����ʻ������Ǳ�����ĳ�Ա");
	m_mapErrorDes[1379] = TEXT("ָ���ı������Ѵ���");
	m_mapErrorDes[1380] = TEXT("��¼ʧ��:δ�����û��ڴ˼�����ϵ������¼����");
	m_mapErrorDes[1381] = TEXT("�ѳ����ڵ�һϵͳ�пɱ�����ܵ�������");
	m_mapErrorDes[1382] = TEXT("���ܵĳ��ȳ����������󳤶�");
	m_mapErrorDes[1383] = TEXT("���ذ�ȫ�䷢�������ݿ��ڲ�������һ����");
	m_mapErrorDes[1384] = TEXT("�ڳ��Ե�¼�Ĺ����У��û��İ�ȫ�����Ļ����˹���İ�ȫ��ʶ");
	m_mapErrorDes[1385] = TEXT("��¼ʧ��:δ�����û��ڴ˼�����ϵ������¼����");
	m_mapErrorDes[1386] = TEXT("�����û�����ʱ��Ҫ�����������");
	m_mapErrorDes[1387] = TEXT("���ڳ�Ա�����ڣ��޷�����Ա��ӵ��������У�Ҳ�޷��ӱ����齫��ɾ��");
	m_mapErrorDes[1388] = TEXT("�޷����³�Ա���뵽�������У���Ϊ��Ա���ʻ����ʹ���");
	m_mapErrorDes[1389] = TEXT("��ָ������İ�ȫ��ʶ");
	m_mapErrorDes[1390] = TEXT("���Ĵ��û�����ʱ��Ҫ�����������");
	m_mapErrorDes[1391] = TEXT("����ACLδ�����κοɳм̵����");
	m_mapErrorDes[1392] = TEXT("�ļ���Ŀ¼�����޷���ȡ");
	m_mapErrorDes[1393] = TEXT("���̽ṹ�����޷���ȡ");
	m_mapErrorDes[1394] = TEXT("���κ�ָ����¼�Ự���û��Ự��");
	m_mapErrorDes[1395] = TEXT("���ڷ��ʵķ�����������Ŀ����Ȩ���� ��ʱ���Ѿ��޷������ӣ�ԭ�����Ѿ�����ɽ��ܵ�������Ŀ����");
	m_mapErrorDes[1396] = TEXT("��¼ʧ��:��Ŀ���ʻ����Ʋ���ȷ");
	m_mapErrorDes[1397] = TEXT("�໥�����֤ʧ�� �÷�����������������������");
	m_mapErrorDes[1398] = TEXT("�ڿͻ����ͷ�����֮����һ��ʱ���");
	m_mapErrorDes[12111] = TEXT("���ڻػ�����ֹ��FTP����δ��ɡ�");
	m_mapErrorDes[12112] = TEXT("����ģʽ�ڷ������ϲ����á�");
	m_mapErrorDes[12110] = TEXT("����Ĳ���������FTP�Ự����Ͻ��У���Ϊ�������ڽ����С�");
	m_mapErrorDes[12137] = TEXT("����������Ҳ�����");
	m_mapErrorDes[12132] = TEXT("��Gopher��������������ʱ��⵽����");
	m_mapErrorDes[12133] = TEXT("�����Ѿ�������");
	m_mapErrorDes[12135] = TEXT("�ò����Ķ�λ�����Ͳ���ȷ��");
	m_mapErrorDes[12134] = TEXT("�ṩ�Ķ�λ����Ч��");
	m_mapErrorDes[12131] = TEXT("�������������ļ���λ�����С�");
	m_mapErrorDes[12136] = TEXT("����Ĳ���ֻ�����Gopher +��������ָ��Gopher +�����Ķ�λ����");
	m_mapErrorDes[12130] = TEXT("������Gopher���������ص�����ʱ��⵽����");
	m_mapErrorDes[12138] = TEXT("��λ��������δ֪�ġ�");
	m_mapErrorDes[12162] = TEXT("HTTP cookie���������ܾ���");
	m_mapErrorDes[12161] = TEXT("HTTP cookie��Ҫȷ�ϡ�");
	m_mapErrorDes[12151] = TEXT("������û�з����κα��⡣");
	m_mapErrorDes[12155] = TEXT("�����޷���ӣ���Ϊ���Ѿ����ڡ�");
	m_mapErrorDes[12150] = TEXT("����ı����Ҳ�����");
	m_mapErrorDes[12153] = TEXT("�ṩ�ı�����Ч��");
	m_mapErrorDes[12154] = TEXT("��HttpQueryInfo������ ��Ч��");
	m_mapErrorDes[12152] = TEXT("��������Ӧ�޷�������");
	m_mapErrorDes[12160] = TEXT("HTTP����δ���ض���");
	m_mapErrorDes[12456] = TEXT("�ض���ʧ�ܣ���Ϊ�������ģ����磬HTTP��FTP�������г����ض�����ʧ�ܣ�Ĭ��Ϊ��γ��ԣ���");
	m_mapErrorDes[12168] = TEXT("�ض�����Ҫ�û�ȷ�ϡ�");
	m_mapErrorDes[12047] = TEXT("Ӧ�ó����޷������첽�̡߳�");
	m_mapErrorDes[12166] = TEXT("�Զ��������ýű��г��ִ���");
	m_mapErrorDes[12010] = TEXT("�ṩ��InternetQueryOption�� InternetSetOption��ѡ��ĳ��� ����ָ����ѡ�����Ͳ���ȷ��");
	m_mapErrorDes[12022] = TEXT("�ҵ��������ע���ֵ���������Ͳ���ȷ��ֵ��Ч��");
	m_mapErrorDes[12029] = TEXT("�������ӵ�������ʧ�ܡ�");
	m_mapErrorDes[12042] = TEXT("Ӧ�ó������ڷ��������Ը��Ĳ���ȫ�ķ������ϵĶ����ı���");
	m_mapErrorDes[12044] = TEXT("��������������ͻ�����֤��");
	m_mapErrorDes[12046] = TEXT("�˼������δ���ÿͻ�����Ȩ��");
	m_mapErrorDes[12030] = TEXT("�����������������ֹ��");
	m_mapErrorDes[12175] = TEXT("WinINetδ�ܶ���Ӧִ�����ݽ��롣");
	m_mapErrorDes[12049] = TEXT("��һ���߳����ڽ�������Ի���");
	m_mapErrorDes[12163] = TEXT("�����������Ѿ���ʧ��");
	m_mapErrorDes[12003] = TEXT("������������һ����չ����");
	m_mapErrorDes[12171] = TEXT("���ڰ�ȫ��飬�ù���ʧ�ܡ�");
	m_mapErrorDes[12032] = TEXT("�ù�����Ҫ��������");
	m_mapErrorDes[12054] = TEXT("���������Դ��ҪFortezza�����֤��");
	m_mapErrorDes[12036] = TEXT("����ʧ�ܣ���Ϊ����Ѿ����ڡ�");
	m_mapErrorDes[12039] = TEXT("�����ض���Ӧ�ó������ӷ�SSLת��ΪSSL���ӡ�");
	m_mapErrorDes[12052] = TEXT("�ύ��SSL���ӵ����������ض��򵽷�SSL���ӡ�");
	m_mapErrorDes[12040] = TEXT("�����ض���Ӧ�ó�������SSLת�Ƶ���SSL���ӡ�");
	m_mapErrorDes[12027] = TEXT("����ĸ�ʽ��Ч��");
	m_mapErrorDes[12019] = TEXT("������Ĳ����޷�ִ�У���Ϊ���ṩ���ֱ����ڲ���ȷ��״̬��");
	m_mapErrorDes[12018] = TEXT("�˲����ṩ�ľ�����Ͳ���ȷ��");

	m_mapErrorDes[12014] = TEXT("���Ӳ���¼��FTP�������������޷���ɣ���Ϊ�ṩ�����벻��ȷ��");
	m_mapErrorDes[12013] = TEXT("���Ӳ���¼��FTP�������������޷���ɣ���Ϊ�ṩ���û�������ȷ��");
	m_mapErrorDes[12053] = TEXT("��������Ҫ��CD-ROM�������в���һ��CD-ROM�Բ����������Դ��");
	m_mapErrorDes[12004] = TEXT("�����ڲ�����");
	m_mapErrorDes[12045] = TEXT("�ù��ܲ���Ϥ���ɷ�����֤���֤��䷢������");
	m_mapErrorDes[12016] = TEXT("����Ĳ�����Ч��");
	m_mapErrorDes[12009] = TEXT("��InternetQueryOption�� InternetSetOption������ ָ������Ч��ѡ��ֵ��");
	m_mapErrorDes[12033] = TEXT("�Դ����������Ч��");
	m_mapErrorDes[12005] = TEXT("����ַ��Ч��");
	m_mapErrorDes[12028] = TEXT("�������Ŀ�޷��ҵ���");
	m_mapErrorDes[12015] = TEXT("���Ӳ���¼��FTP������������ʧ�ܡ�");
	m_mapErrorDes[12174] = TEXT("MS-LogoffժҪ�����Ѵ���վ���ء�");
	m_mapErrorDes[12041] = TEXT("���ݲ���ȫ��ȫ��һЩ���ڲ鿴�����ݿ������Բ���ȫ�ķ�������");
	m_mapErrorDes[12007] = TEXT("�����������޷�����.");
	m_mapErrorDes[12173] = TEXT("Ŀǰ��δʵʩ��");
	m_mapErrorDes[12034] = TEXT("�û�������������������ѱ�����");
	m_mapErrorDes[12025] = TEXT("������δ���ûص�����������޷�ִ���첽����");
	m_mapErrorDes[12024] = TEXT("�޷��ṩ�첽������Ϊ�ṩ����������ֵ��");
	m_mapErrorDes[12023] = TEXT("Ŀǰ�޷�ֱ�ӷ������硣");

	m_mapErrorDes[12172] = TEXT("û�з���WinINet API�ĳ�ʼ����");
	m_mapErrorDes[12020] = TEXT("��������ͨ��������С�");
	m_mapErrorDes[12017] = TEXT("�ò�����ȡ����ͨ������Ϊ�ڲ������֮ǰ���������ڵľ���ѹرա�");
	m_mapErrorDes[12023] = TEXT("Ŀǰ�޷�ֱ�ӷ������硣");
	m_mapErrorDes[12011] = TEXT("�����ѡ������ã�ֻ�ܲ�ѯ��");
	m_mapErrorDes[12001] = TEXT("Ŀǰû�и���ľ���������ɡ�");
	m_mapErrorDes[12043] = TEXT("��Ӧ�ó������ݷ���������ȫ�ķ�������");
	m_mapErrorDes[12008] = TEXT("�����Э���޷��ҵ���");
	m_mapErrorDes[12165] = TEXT("ָ���Ĵ���������޷����");
	m_mapErrorDes[12048] = TEXT("�ù����޷������ض�����Ϊ�÷����Ѹ��ģ����磬HTTP��FTP����");
	m_mapErrorDes[12021] = TEXT("�޷��ҵ������ע���ֵ��");
	m_mapErrorDes[12026] = TEXT("����һ�������������ڵȴ�������Ĳ����޷���ɡ�");
	m_mapErrorDes[12050] = TEXT("�öԻ���Ӧ�����ԡ�");
	m_mapErrorDes[12038] = TEXT("SSL֤��ͨ�����ƣ��������ֶΣ�����ȷ ");
	m_mapErrorDes[12037] = TEXT("�ӷ������յ���SSL֤�����ڲ���ȷ��֤���ѹ��ڡ�");

	m_mapErrorDes[12055] = TEXT("SSL֤���������");
	m_mapErrorDes[12056] = TEXT("SSL֤��δ��������");
	m_mapErrorDes[12057] = TEXT("����SSL֤��ʧ�ܡ�");
	m_mapErrorDes[12170] = TEXT("SSL֤���ѱ�������");
	m_mapErrorDes[12169] = TEXT("SSL֤����Ч��");
	m_mapErrorDes[12157] = TEXT("Ӧ�ó����ڼ���SSL��ʱ�����ڲ�����");
	m_mapErrorDes[12164] = TEXT("ָʾ����վ��������޷����ʡ�");
	m_mapErrorDes[12012] = TEXT("WinINet֧�����ڹرջ�ж�ء�");
	m_mapErrorDes[12159] = TEXT("�����Э��ջδ���أ�����Ӧ�ó����޷�����WinSock��");
	m_mapErrorDes[12002] = TEXT("�����ѳ�ʱ��");
	m_mapErrorDes[12158] = TEXT("�ù����޷������ļ���");
	m_mapErrorDes[12167] = TEXT("�Զ��������ýű��޷����ء�INTERNET_FLAG_MUST_CACHE_REQUEST��־�����á�");
	m_mapErrorDes[12006] = TEXT("URL�����޷�ʶ�����֧�֡�");








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
