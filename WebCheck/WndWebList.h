#pragma once


// CWndWebList

class CWndWebList : public CWnd
{
	DECLARE_DYNAMIC(CWndWebList)

public:
	CWndWebList();
	virtual ~CWndWebList();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


