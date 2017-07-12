// WndWebList.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "WebCheck.h"
#include "WndWebList.h"





//��С����
#define CHECK_BOX_WIDTH		25
#define CHECK_BOX_HEIGHT	20
#define EDIT_WIDTH			190
#define BTN_WIDTH			44
#define BTN_HEIGHT			15
#define TOP_MARGINE			5
#define LEFT_MARGINE		10


// CWndWebList

IMPLEMENT_DYNAMIC(CWndWebList, CWnd)

CWndWebList::CWndWebList()
{
	m_strIndex = TEXT("");
	m_strDescribe = TEXT("");
	m_strStatus = TEXT("");
	m_wIndex = 0;
}

void CWndWebList::SetWebListInfo(WORD wIndex, WORD wStatus, CString strDescribe, CString &strUrl)
{
	m_strIndex.Format(TEXT("%d"), wIndex);
	m_strStatus.Format(TEXT("%d"),wStatus);
	m_strDescribe = strDescribe;
	m_strUrl = strUrl;
	m_wIndex = wIndex;

	OnCreateWebCtrl();
}

CWndWebList::~CWndWebList()
{
	delete			m_btnCheckBox;
	delete			m_btnOpenWeb;
	delete			m_urlEdit;
	delete			m_editDes;
	delete			m_editStatus;

	m_btnCheckBox = NULL;
	m_btnOpenWeb = NULL;
	m_urlEdit = NULL;
	m_editDes = NULL;
	m_editStatus = NULL;
	m_strIndex = TEXT("");
	m_strStatus = TEXT("");
	m_strDescribe = TEXT("");
	m_strUrl = TEXT("");

	m_wIndex = 0;
}


BEGIN_MESSAGE_MAP(CWndWebList, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
//	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CWndWebList ��Ϣ�������



int CWndWebList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	return 0;
}


BOOL CWndWebList::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	return CWnd::OnEraseBkgnd(pDC);
}

void CWndWebList::OnCreateWebCtrl()
{
	m_btnCheckBox = new CButton();
	m_btnCheckBox->Create(m_strIndex, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX  , CRect(0, 0, CHECK_BOX_WIDTH+5, CHECK_BOX_HEIGHT), this, IDC_CHECKBOX_INDEX + m_wIndex);

	m_urlEdit = new CEdit();
	m_urlEdit->Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, CRect(CHECK_BOX_WIDTH + 8, 0, CHECK_BOX_WIDTH + 5 + EDIT_WIDTH, CHECK_BOX_HEIGHT), this, IDC_EDIT_URL_INDEX + m_wIndex);
	m_urlEdit->SetLimitText(256);

	m_urlEdit->SetWindowText(TEXT("http://"));

	m_btnOpenWeb = new CButton();
	m_btnOpenWeb->Create(TEXT("��"), WS_CHILD | WS_VISIBLE, CRect(CHECK_BOX_WIDTH + EDIT_WIDTH + 16, 0, CHECK_BOX_WIDTH + EDIT_WIDTH + BTN_WIDTH + 10, CHECK_BOX_HEIGHT), this, IDC_BTN_INDEX + m_wIndex);

	m_editStatus = new CEdit();
	m_editStatus->Create(WS_CHILD | WS_VISIBLE , CRect(CHECK_BOX_WIDTH + EDIT_WIDTH + BTN_WIDTH + 24, 0, CHECK_BOX_WIDTH + EDIT_WIDTH + BTN_WIDTH + 46, CHECK_BOX_HEIGHT), this, IDC_EDIT_STATU_INDEX + m_wIndex);
	m_editStatus->SetReadOnly(TRUE);

	m_editDes = new CEdit();
	m_editDes->Create(WS_CHILD | WS_VISIBLE , CRect(CHECK_BOX_WIDTH + EDIT_WIDTH + BTN_WIDTH * 2 + 16, 0, CHECK_BOX_WIDTH + EDIT_WIDTH + BTN_WIDTH * 2 + EDIT_WIDTH + 26, CHECK_BOX_HEIGHT), this, IDC_EDIT_DESI_INDEX + m_wIndex);
	m_editDes->SetReadOnly(TRUE);

}

void CWndWebList::OnDestroyWebListWnd()
{
	if (m_btnCheckBox)
		m_btnCheckBox->DestroyWindow();

	if (m_urlEdit)
		m_urlEdit->DestroyWindow();

	if (m_btnOpenWeb)
		m_btnOpenWeb->DestroyWindow();

	if (m_editStatus)
		m_editStatus->DestroyWindow();

	if (m_editDes)
		m_editDes->DestroyWindow();
}


void CWndWebList::DoDataExchange(CDataExchange* pDX)
{
	// TODO: �ڴ����ר�ô����/����û���

	CWnd::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_URL_INDEX + m_wIndex, m_strUrl);
	DDX_Text(pDX, IDC_EDIT_STATU_INDEX + m_wIndex, m_strStatus);
	DDX_Text(pDX, IDC_EDIT_DESI_INDEX + m_wIndex, m_strDescribe);

}

void CWndWebList::SetCheckWebValue(DWORD dwStatus)
{
	m_strStatus.Format(TEXT("%ld"), dwStatus);
	switch (dwStatus)
	{
	case 0:
		m_strDescribe.Format(TEXT("�����������ַ�Ƿ�����"));
		break;
	case 200:
		m_strDescribe.Format(TEXT("����"));
		break;
	case 400:
		m_strDescribe.Format(TEXT("�����������ֶ�����ַ�鿴�Ƿ�������"));
		break;
	case 403:
		m_strDescribe.Format(TEXT("�������ܾ��������"));
		break;
	case 404:
		m_strDescribe.Format(TEXT("�������Ҳ����������ҳ"));
		break;
	case 408:
		m_strDescribe.Format(TEXT("�������Ⱥ�����ʱ�����ֶ������Ӳ鿴�Ƿ�����"));
		break;
	case 409:
		m_strDescribe.Format(TEXT("���������������ʱ������ͻ"));
		break;
	case 410:
		m_strDescribe.Format(TEXT("�������Դ������ɾ��"));
		break;
	case 500:
		m_strDescribe.Format(TEXT(" ���������������޷��������"));
		break;
	case 501:
		m_strDescribe.Format(TEXT(" ���������߱��������Ĺ���"));
		break;
	case 502:
		m_strDescribe.Format(TEXT(" ��������Ϊ���ػ���������η������յ���Ч��Ӧ"));
		break;
	case 503:
		m_strDescribe.Format(TEXT(" ������Ŀǰ�޷�ʹ��"));
		break;
	case 504:
		m_strDescribe.Format(TEXT(" ��������Ϊ���ػ��������û�м�ʱ�����η������յ�����"));
		break;
	case 505:
		m_strDescribe.Format(TEXT("��������֧�����������õ� HTTP Э��汾"));
		break;
	default:
		m_strDescribe.Format(TEXT("����δ֪�������ֶ�����ַ��"));
		break;
	}
	UpdateData(FALSE);
}

//void CWndWebList::OnLButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//
//	CWnd::OnLButtonDown(nFlags, point);
//}


BOOL CWndWebList::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: �ڴ����ר�ô����/����û���
	UINT nCommandID = LOWORD(wParam);
	
	if (nCommandID == (IDC_BTN_INDEX + m_wIndex))
	{
		CString strUrl = m_strUrl;
		strUrl.TrimLeft();
		strUrl.TrimRight();
		if(TEXT("") != strUrl)
			ShellExecute(NULL, TEXT("open"), strUrl, NULL, NULL, SW_NORMAL);
	}
	return CWnd::OnCommand(wParam, lParam);
}
