// WndWebList.cpp : ʵ���ļ�
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
