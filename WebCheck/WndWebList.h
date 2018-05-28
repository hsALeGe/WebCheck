#pragma once
#include <vector>
#include <map>

//复选框定义
#define	IDC_CHECKBOX_INDEX		2000

//编辑框定义
#define IDC_EDIT_URL_INDEX		3000
#define IDC_EDIT_DESI_INDEX		4000
#define IDC_EDIT_STATU_INDEX	5000


//按钮定义
#define IDC_BTN_INDEX		6000

class CWndWebList : public CWnd
{
	DECLARE_DYNAMIC(CWndWebList)

public:
	CWndWebList();
	virtual ~CWndWebList();
	void SetWebListInfo(WORD m_wIndex, WORD wStatus, CString strDescribe, CString &strUrl);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void OnCreateWebCtrl();

private:
	CButton			*m_btnCheckBox;
	CButton			*m_btnOpenWeb;
	CEdit			*m_urlEdit;
	CString			m_strIndex;
	CString			m_strStatus;
	CString			m_strDescribe;
	CString			m_strUrl;
	CEdit			*m_editDes;
	CEdit			*m_editStatus;
	WORD			m_wIndex;

public:
	CButton* GetCheckBox() { return m_btnCheckBox; }
	WORD	 GetIndex() { return m_wIndex; }
	void OnDestroyWebListWnd();
	CString GetUrlString() { return m_strUrl; }

	virtual void DoDataExchange(CDataExchange* pDX);

	void SetCheckWebValue(DWORD dwStatus,CString strMsg=TEXT(""),DWORD dwErrorCode=0);
//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};


