#pragma once
#include "WndWebList.h"

// CWebWnd

class CWebWnd : public CWnd
{
	DECLARE_DYNAMIC(CWebWnd)

public:
	CWebWnd();
	virtual ~CWebWnd();

public:
	std::map<WORD, CWndWebList*> m_webListMap;
	std::map<WORD,CString> m_strUrlRecord;
	static int m_nUrlRecordCount;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
	void UpdateScroll();
public:
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void OnInsertUrlRecord();
	void OnAddUrlRecord(CString strUrl);
	void OnDeleteWebList();
	void OnMoveWebListCtrl();

private:
	int		m_nYoldPos;
public:
//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};


