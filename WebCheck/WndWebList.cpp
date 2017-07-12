// WndWebList.cpp : 实现文件
//

#include "stdafx.h"
#include "WebCheck.h"
#include "WndWebList.h"





//大小定义
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



// CWndWebList 消息处理程序



int CWndWebList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	return 0;
}


BOOL CWndWebList::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
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
	m_btnOpenWeb->Create(TEXT("打开"), WS_CHILD | WS_VISIBLE, CRect(CHECK_BOX_WIDTH + EDIT_WIDTH + 16, 0, CHECK_BOX_WIDTH + EDIT_WIDTH + BTN_WIDTH + 10, CHECK_BOX_HEIGHT), this, IDC_BTN_INDEX + m_wIndex);

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
	// TODO: 在此添加专用代码和/或调用基类

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
		m_strDescribe.Format(TEXT("请检查输入的网址是否有误"));
		break;
	case 200:
		m_strDescribe.Format(TEXT("正常"));
		break;
	case 400:
		m_strDescribe.Format(TEXT("错误请求，请手动打开网址查看是否正常！"));
		break;
	case 403:
		m_strDescribe.Format(TEXT("服务器拒绝你的请求"));
		break;
	case 404:
		m_strDescribe.Format(TEXT("服务器找不到请求的网页"));
		break;
	case 408:
		m_strDescribe.Format(TEXT("服务器等候请求超时，请手动打开连接查看是否正常"));
		break;
	case 409:
		m_strDescribe.Format(TEXT("服务器在完成请求时发生冲突"));
		break;
	case 410:
		m_strDescribe.Format(TEXT("请求的资源已永久删除"));
		break;
	case 500:
		m_strDescribe.Format(TEXT(" 服务器遇到错误，无法完成请求"));
		break;
	case 501:
		m_strDescribe.Format(TEXT(" 服务器不具备完成请求的功能"));
		break;
	case 502:
		m_strDescribe.Format(TEXT(" 服务器作为网关或代理，从上游服务器收到无效响应"));
		break;
	case 503:
		m_strDescribe.Format(TEXT(" 服务器目前无法使用"));
		break;
	case 504:
		m_strDescribe.Format(TEXT(" 服务器作为网关或代理，但是没有及时从上游服务器收到请求"));
		break;
	case 505:
		m_strDescribe.Format(TEXT("服务器不支持请求中所用的 HTTP 协议版本"));
		break;
	default:
		m_strDescribe.Format(TEXT("发生未知错误，请手动打开网址。"));
		break;
	}
	UpdateData(FALSE);
}

//void CWndWebList::OnLButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	CWnd::OnLButtonDown(nFlags, point);
//}


BOOL CWndWebList::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
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
