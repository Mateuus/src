// DlgSceneExport.cpp : implementation file
//

#include "stdafx.h"
#include "DlgSceneExport.h"
#include "Resource.h"
#include "../Common/Config.h"


// DlgSceneExport dialog

IMPLEMENT_DYNAMIC(DlgSceneExport, CDialog)

DlgSceneExport::DlgSceneExport(CWnd* pParent /*=NULL*/)
	: CDialog(DlgSceneExport::IDD, pParent)
{
}

DlgSceneExport::~DlgSceneExport()
{
}

void DlgSceneExport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_TEXCOPY, m_CheckTexCopy);
	DDX_Control(pDX, IDC_EXPORTPATH, m_EdExportPath);
	DDX_Control(pDX, IDC_COMPRESS_LIST, m_CompressList);
	DDX_Control(pDX, IDC_NRM_EPSILON, m_NrmEpsilon);
	DDX_Control(pDX, IDC_NRM_TEXT, m_NrmText);
	DDX_Control(pDX, IDC_CHECK_SAVEMATS, m_CheckSaveMats);
	DDX_Control(pDX, IDC_CHECK_VTXCOLOR, m_CheckVtxColors);
}


BEGIN_MESSAGE_MAP(DlgSceneExport, CDialog)
	ON_BN_CLICKED(IDC_SCENE_EXPORT, &DlgSceneExport::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK_TEXCOPY, &DlgSceneExport::OnBnClickedCheckTexcopy)
	ON_BN_CLICKED(IDCLOSE, &DlgSceneExport::OnBnClickedClose)
	ON_BN_CLICKED(IDC_BROWSE, &DlgSceneExport::OnBnClickedBrowse)
	ON_LBN_SELCHANGE(IDC_COMPRESS_LIST, &DlgSceneExport::OnLbnSelchangeCompressList)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// DlgSceneExport message handlers

void DlgSceneExport::ReadConfig()
{
  extern Interface* _ip;
  m_exportPath   = g_Config.GetStr("scene_path", _ip->GetDir(APP_EXPORT_DIR));
  m_expCopyTex   = g_Config.GetInt("scene_copytex", 0);
  m_expTexFormat = g_Config.GetInt("scene_texformat", 0);
  m_normalTolerance = g_Config.GetInt("scene_nrmeps", 5);
  m_expSaveMats  = 1;
  
  m_NrmEpsilon.SetRange(0, 20, TRUE);
  m_NrmEpsilon.SetPos(m_normalTolerance);
  m_NrmEpsilon.SetTicFreq(1);
  char buf[256];
  sprintf(buf, "%d degrees", m_normalTolerance);
  m_NrmText.SetWindowText(buf);
}

void DlgSceneExport::WriteConfig()
{
  m_expSaveMats = m_CheckSaveMats.GetCheck();
  m_expVtxColors = m_CheckVtxColors.GetCheck();

  g_Config.SetStr("scene_path",      m_exportPath.c_str());
  g_Config.SetInt("scene_copytex",   m_expCopyTex);
  g_Config.SetInt("scene_texformat", m_expTexFormat);
  g_Config.SetInt("scene_nrmeps",    m_normalTolerance);

  g_Config.SaveConfig();
  return;
}

BOOL DlgSceneExport::OnInitDialog()
{
  __super::OnInitDialog();
  
  ReadConfig();
  
  m_EdExportPath.SetWindowText(m_exportPath.c_str());
  m_CheckTexCopy.SetCheck(m_expCopyTex);
  m_CheckSaveMats.SetCheck(m_expSaveMats);
  m_CheckVtxColors.SetCheck(0);

  m_CompressList.AddString("-NONE-");
  m_CompressList.AddString("DXT1");
  m_CompressList.AddString("DXT5");
  m_CompressList.SetCurSel(m_expTexFormat);
  m_CompressList.EnableWindow(m_expCopyTex);

  return TRUE;
}

void DlgSceneExport::OnBnClickedButton1()
{
  WriteConfig();
  EndDialog(IDC_SCENE_EXPORT);
}

void DlgSceneExport::OnBnClickedCheckTexcopy()
{
  m_expCopyTex = m_CheckTexCopy.GetCheck();
  m_CompressList.EnableWindow(m_expCopyTex);
}

void DlgSceneExport::OnBnClickedCheckDxt1()
{
}

void DlgSceneExport::OnBnClickedClose()
{
  EndDialog(IDCANCEL);
}

void DlgSceneExport::OnBnClickedBrowse()
{
  char dir[MAX_PATH];
  if(!DirectoryPicker(GetSafeHwnd(), "Select Export Directory", m_exportPath.c_str(), dir, NULL))
    return;
    
  m_exportPath = dir;
  m_EdExportPath.SetWindowText(dir);
}

void DlgSceneExport::OnLbnSelchangeCompressList()
{
  m_expTexFormat = m_CompressList.GetCurSel();
}

void DlgSceneExport::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  if(pScrollBar == (CScrollBar*)&m_NrmEpsilon) 
  {
    m_normalTolerance = m_NrmEpsilon.GetPos();
    char buf[256];
    sprintf(buf, "%d degrees", m_normalTolerance);
    m_NrmText.SetWindowText(buf);
  }

  CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}
