
// WebCheckDlg.h : 头文件
//

#pragma once
#include "WebWnd.h"
#include <deque>
#import "msxml6.dll"
using namespace MSXML2;

// CWebCheckDlg 对话框
class CWebCheckDlg : public CDialogEx
{
// 构造
public:
	CWebCheckDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBCHECK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCheck();

	//变量定义
private:
	CWebWnd						m_webWnd;					//检测窗口
	MSXML2::IXMLDOMDocumentPtr	m_xmlDoc;						
	MSXML2::IXMLDOMElementPtr	m_xmlUrl;
	HANDLE						m_checkThread;

public:
	static CCriticalSection		m_csUrlSection;					//url
	static std::deque<WORD>		m_urlIndexDeque;			

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnAdd();

public:
	static CWebCheckDlg * _instance;
	static CWebCheckDlg* GetInstance();

private:
	void LoadConfigFun();
	void ReadXmlConfig(CString strPath);
	static void OnCheckWebStatus(LPVOID lparam);
	void LoadErrorDesc();

public:
	afx_msg void OnBnClickedBtnDel();
private:
	UINT m_uTimeElapse;
	static std::map<WORD, CString> m_mapErrorDes;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnAbout();
};
