
#include "NagDlg.h"

#include "utilites/tokenizer.h"
#include "utilites/security.h"

#include "wtladdons.h"

LRESULT CNagDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL&)
{
  _lw(m_hWnd, IDD_NAG);
  CenterWindow();

  static int counter=0; counter++;

  if ((counter < 30 || counter % 19 > 0) && counter != 1)
    timer_ = 0;
  else
    timer_ = 5 + counter/100;

  ::EnableWindow(GetDlgItem(IDOK), false);

#ifndef DEMO
  if (!security::manager().is_unregistered() || timer_ == 0)
    return EndDialog(0);
#endif

  SetTimer(0, 1000);
  OnTimer(0, 0);
  
  return 0;
}

LRESULT CNagDlg::OnCloseCmd(WORD , WORD wID, HWND , BOOL&)
{
#ifndef DEMO
    if (--timer_<=0) 
    {
      EndDialog(0);
    }
#endif
    return 0;
}

LRESULT CNagDlg::OnTimer(UINT, TIMERPROC)
{
  if (--timer_<=0) 
  {
    ::EnableWindow(GetDlgItem(IDOK), true);
  }

  std::string string=MakeWindowText( GetDlgItem(IDOK) );
  std::string start=tokenizer::head(string, " ");
  string = start + (timer_ > 0 ? strlib::strf(" (%d)", timer_+1) : "");
  MakeWindowText( GetDlgItem(IDOK) ) = string;
  return 0;
}
