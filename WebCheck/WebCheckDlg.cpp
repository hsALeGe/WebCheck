
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

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

	// TODO:  �ڴ������ר�õĴ�������
	m_wndWebList.Create(NULL, TEXT("����1"), WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL, CRect(0,0,0,0), this, IDC_WNDWEBLIST_CTRL);
	m_wndWebList.ShowWindow(SW_SHOW);

	return 0;
}


void CWebCheckDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (m_wndWebList)
	{
	m_wndWebList.MoveWindow(CRect(10, 10, 440, 340));
	}
}
