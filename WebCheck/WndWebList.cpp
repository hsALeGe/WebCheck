// WndWebList.cpp : 实现文件
//

#include "stdafx.h"
#include "WebCheck.h"
#include "WndWebList.h"


// CWndWebList

IMPLEMENT_DYNAMIC(CWndWebList, CWnd)

CWndWebList::CWndWebList()
{

}

CWndWebList::~CWndWebList()
{
}


BEGIN_MESSAGE_MAP(CWndWebList, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
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
