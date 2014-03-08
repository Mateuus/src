#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// DlgSceneExport dialog

class DlgSceneExport : public CDialog
{
	DECLARE_DYNAMIC(DlgSceneExport)

public:
	int		m_expCopyTex;
	int		m_expTexFormat;
	int		m_normalTolerance;
	std::string	m_exportPath;
	void		ReadConfig();
	void		WriteConfig();

public:
	DlgSceneExport(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgSceneExport();

// Dialog Data
	enum { IDD = IDD_SCENE_EXPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCheckTexcopy();
	afx_msg void OnBnClickedCheckDxt1();
	CButton m_CheckTexCopy;
	CEdit m_EdExportPath;
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedBrowse();
	CListBox m_CompressList;
	afx_msg void OnLbnSelchangeCompressList();
	CSliderCtrl m_NrmEpsilon;
	CStatic m_NrmText;
};
