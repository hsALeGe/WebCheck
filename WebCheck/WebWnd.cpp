// WebWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "WebCheck.h"
#include "WebWnd.h"
#include <winuser.h>


#define	WEB_LIST_CTRL_WIDTH			525			//列表宽度
#define WEB_LIST_CTRL_HEIGHT		20			//列表高度
#define WEB_LIST_CTRL_TOP			2			//列表上边距
#define WEB_LIST_CTRL_LEFT			5			//列表左边距

#define IDC_WEB_LIST_CTRL_INDEX		1000

int CWebWnd::m_nUrlRecordCount = 0;

// CWebWnd

IMPLEMENT_DYNAMIC(CWebWnd, CWnd)

CWebWnd::CWebWnd()
{
	m_nYoldPos = 0;
}

CWebWnd::~CWebWnd()
{
	m_nYoldPos = 0;
}


BEGIN_MESSAGE_MAP(CWebWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
//	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CWebWnd 消息处理程序




int CWebWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	std::map<WORD,CString>::iterator itr = m_strUrlRecord.begin();
	if (itr == m_strUrlRecord.end()) return 0;
	
	int index = 0;
	for (; itr != m_strUrlRecord.end(); itr++)
	{
		CWndWebList * web_list = new CWndWebList();
		web_list->SetWebListInfo(++index, 0, TEXT(""), itr->second);
		m_webListMap.insert(std::make_pair(index, web_list));
	}
	return 0;
}


void CWebWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}


BOOL CWebWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd::OnEraseBkgnd(pDC);
	return true;
}

//更新滚动条
void CWebWnd::UpdateScroll()
{
	CRect rect;
	GetClientRect(&rect);

	int nTotalHeight = WEB_LIST_CTRL_TOP*(m_webListMap.size() -1) + WEB_LIST_CTRL_HEIGHT*(m_webListMap.size());

	SCROLLINFO scrllinfo;
	scrllinfo.cbSize = sizeof(scrllinfo);
	scrllinfo.fMask = SIF_PAGE | SIF_RANGE;
	scrllinfo.nMax = nTotalHeight;
	scrllinfo.nMin = 0;
	scrllinfo.nPage = rect.Height();
	scrllinfo.nPos = 0;
	SetScrollInfo(SB_VERT, &scrllinfo);
}

//调整滚动条
void CWebWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	static int oldpos = 0;
	int minpos = 0;
	int maxpos = 0;
	GetScrollRange(SB_HORZ, &minpos, &maxpos);
	maxpos = GetScrollLimit(SB_VERT);

	int curpos = GetScrollPos(SB_VERT);
	switch (nSBCode)
	{
	case SB_TOP:
		curpos = minpos;
		break;
	case SB_BOTTOM:
		curpos = maxpos;
		break;
	case SB_ENDSCROLL:
		break;
	case SB_LINEUP:
		if (curpos > minpos)
			--curpos;
		break;
	case SB_LINEDOWN:
		if (curpos < maxpos)
			++curpos;
		break;
	case SB_PAGEUP:
	{
		SCROLLINFO info;
		GetScrollInfo(SB_VERT, &info);
		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;
	case SB_PAGEDOWN:
	{
		SCROLLINFO info;
		GetScrollInfo(SB_VERT, &info);
		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;
	case SB_THUMBPOSITION:
		curpos = nPos;
		break;
	case SB_THUMBTRACK:
		curpos = nPos;
		break;

	}

	SetScrollPos(SB_VERT, curpos);
	m_nYoldPos = curpos;
	oldpos = curpos;
	/*Invalidate();*/
	OnMoveWebListCtrl();

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

//增加url记录
void CWebWnd::OnInsertUrlRecord()
{
	std::map<WORD,CString>::iterator itr1 = m_strUrlRecord.begin();
	if (itr1 == m_strUrlRecord.end()) return;

	int index = 0;
	for (; itr1 != m_strUrlRecord.end(); itr1++)
	{
		CWndWebList * web_list = new CWndWebList();
		web_list->Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, IDC_WEB_LIST_CTRL_INDEX + index);

		web_list->SetWebListInfo(index, 0, TEXT(""), itr1->second);
		m_webListMap.insert(std::make_pair(index, web_list));
		++index;
	}
	m_nUrlRecordCount = index;
	OnMoveWebListCtrl();
}

//添加单项记录
void CWebWnd::OnAddUrlRecord(CString strUrl)
{
	m_strUrlRecord.insert(std::make_pair(m_nUrlRecordCount, TEXT("")));

	CWndWebList * web_list = new CWndWebList();
	web_list->Create(NULL, NULL, WS_VISIBLE | WS_CHILD , CRect(0, 0, 0, 0), this, IDC_WEB_LIST_CTRL_INDEX + m_nUrlRecordCount);
	web_list->SetWebListInfo(m_nUrlRecordCount, 0, TEXT(""), strUrl);

	m_webListMap.insert(std::make_pair(m_nUrlRecordCount, web_list));
	++m_nUrlRecordCount;

	OnMoveWebListCtrl();

}

//删除单项记录
void CWebWnd::OnDeleteWebList()
{
	RedrawWindow();
}

void CWebWnd::OnMoveWebListCtrl()
{
	int index = 0;
	std::map<WORD, CWndWebList*>::iterator itr = m_webListMap.begin();
	for (; itr != m_webListMap.end(); itr++)
	{
		if (!itr->second) continue;
		/*itr->second->MoveWindow(WEB_LIST_CTRL_LEFT, (WEB_LIST_CTRL_TOP)*index - m_nYoldPos + WEB_LIST_CTRL_HEIGHT*(index++) , WEB_LIST_CTRL_WIDTH, WEB_LIST_CTRL_HEIGHT);
		itr->second->ShowWindow(SW_SHOW);*/
		//移动准备
		HDWP hDwp = BeginDeferWindowPos(64);
		UINT uFlags = SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOZORDER;

		DeferWindowPos(hDwp, itr->second->m_hWnd, NULL, WEB_LIST_CTRL_LEFT, (WEB_LIST_CTRL_TOP)*index - m_nYoldPos + WEB_LIST_CTRL_HEIGHT*(index++), WEB_LIST_CTRL_WIDTH, WEB_LIST_CTRL_HEIGHT, uFlags);
		EndDeferWindowPos(hDwp);
	}
	UpdateScroll();
}



//void CWebWnd::OnLButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	CWnd::OnLButtonDown(nFlags, point);
//}
