
// WebCheckDlg.h : ͷ�ļ�
//

#pragma once
#include "WebWnd.h"
#import "msxml6.dll"
using namespace MSXML2;

// CWebCheckDlg �Ի���
class CWebCheckDlg : public CDialogEx
{
// ����
public:
	CWebCheckDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WEBCHECK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCheck();

	//��������
private:
	CWebWnd						m_webWnd;					//��ⴰ��
	MSXML2::IXMLDOMDocumentPtr	m_xmlDoc;						
	MSXML2::IXMLDOMElementPtr	m_xmlUrl;
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnAdd();

private:
	void LoadConfigFun();
	void ReadXmlConfig(CString strPath);
	DWORD OnCheckWebStatus(CString strUrl);

public:
	afx_msg void OnBnClickedBtnDel();
private:
	UINT m_uTimeElapse;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnAbout();
};
