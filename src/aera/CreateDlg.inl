
#include "createdlg.h"

#include "utilites/Localizator.h"
#include "utilites/tokenizer.h"
#include "wtladdons.h"

static std::string to_str(int x) 
{
  char buffer [30]; itoa(x, buffer, 10);
  return buffer;
}

LRESULT CCreateDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  _lw(m_hWnd, IDD_CREATE_PROJECT);

  CenterWindow(GetParent());

  CheckRadioButton(IDC_CR_DEFAULT, IDC_CR_OTHER, IDC_CR_DEFAULT);

  count_=10;
  SetTimer(0, 1000);
  OnTimer(0, 0);

  return TRUE;
}

LRESULT CCreateDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  EndDialog(
    IDCANCEL==wID ? IDCANCEL : 
    IsDlgButtonChecked(IDC_CR_CURRENT) ? IDC_CR_CURRENT : 
    IsDlgButtonChecked(IDC_CR_DEFAULT) ? IDC_CR_DEFAULT : 
    IDC_CR_OTHER
    );
  return 0;
}

void CCreateDlg::DrawTimer()
{
  std::string str=MakeWindowText( GetDlgItem(IDOK) );
  str=tokenizer::head(str, "(");
  if (!strlib::endswith(str, " ")) str+=" ";
  
  str = count_>0 
    ? str+"("+to_str(count_)+")"
    : str;
  MakeWindowText( GetDlgItem(IDOK) )=str;
}

LRESULT CCreateDlg::OnTimer(UINT, TIMERPROC)
{
  if (--count_==0) 
  {
    SendMessage(WM_COMMAND, IDOK, 0);
    return 0;
  }

  DrawTimer();
  return 0;
}

LRESULT CCreateDlg::OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  wID;
  count_=0; DrawTimer();

  KillTimer(0);
  return 0;
}

LRESULT CCreateDlg::OnDblClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  SendMessage(WM_COMMAND, IDOK, 0);
  return 0;
}


