#include "stdafx.h"

#include "aboutdlg.h"

#include "utilites/Localizator.h"
#include "utilites/tokenizer.h"
#include "wtladdons.h"

#include "registrationDlg.h"

#include "utilites/security.h"

#include "FileVersionInfo.h"

LRESULT CAboutDlg::OnChar(TCHAR c, UINT, UINT)
{ 
  SetMsgHandled(false);
  return 0;
}

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  _lw(m_hWnd, IDD_ABOUTBOX);

  url_.SubclassWindow( GetDlgItem(IDC_ABOUT_URL1) );
  url2_.SubclassWindow( GetDlgItem(IDC_ABOUT_URL2) );

  reg1_.Attach( GetDlgItem(IDC_ABOUT_REGINFO1) );
  reg2_.Attach( GetDlgItem(IDC_ABOUT_REGINFO2) );
  UpdateRegistrationInfo();

  FileVersionInfo vi(utils::get_exefile_name());
  std::string ver0=vi["ProductVersion"];
  ver0=strlib::replace(ver0, " ", "");

  std::string ver=tokenizer::head(CWindowText(GetDlgItem(IDC_ABOUT_STATIC1)).string(), "@");
  ver+=tokenizer::head(ver0, ",");
  ver+="."+tokenizer::head(ver0, ",");
  tokenizer::head(ver0, ",");
  ver+=" (build:"+tokenizer::head(ver0, ",")+")";

  CWindowText(GetDlgItem(IDC_ABOUT_STATIC1))=ver;

  CFontHandle fnt;
  LOGFONT lf={0};
  CDC dc=::GetWindowDC(GetDesktopWindow());
  lf.lfHeight=-MulDiv(8, GetDeviceCaps(dc, LOGPIXELSY), 72);
  lf.lfWeight=FW_BOLD;
  strcpy(lf.lfFaceName, "Tahoma");
  fnt.CreateFontIndirect(&lf);
  reg1_.SetFont( fnt );
  reg2_.SetFont( fnt );

  CenterWindow(GetParent());
  return TRUE;
}

void CAboutDlg::UpdateRegistrationInfo()
{
	std::string info=security::manager().get_registration_info();
	std::string info1=tokenizer::head(info, "\n");
	std::string info2=tokenizer::head(info, "\n");

	MakeWindowText(reg1_)=info1;
	MakeWindowText(reg2_)=info2;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  EndDialog(wID);
  return 0;
}

void CAboutDlg::OnRegistration(UINT, int, HWND)
{	
#ifndef DEMO
	CRegistrationDlg dlg;

	ShowWindow(SW_HIDE);
	dlg.DoModal();

	ShowWindow(SW_SHOW);
	UpdateRegistrationInfo();	
#else
    ShellExecute(0, 0, "http://aetestingtools.com/demo_registration", 0, 0 , SW_SHOW );
#endif
}

LRESULT CAboutDlg::OnIconDblClk(LPNMHDR)
{
	return 0;
}
