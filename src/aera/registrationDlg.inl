
#include "registrationDlg.h"

#include "utilites/tokenizer.h"
#include "utilites/security.h"
#include "wtladdons.h"

LRESULT CRegistrationDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL&)
{
  _lw(m_hWnd, IDD_REGISTRATION_CODE);
  CenterWindow();
  invert_mode_=false;

  ::EnableWindow(GetDlgItem(IDC_REGISTRATION_INFO), false);

  if (::IsClipboardFormatAvailable(CF_TEXT) && ::OpenClipboard(m_hWnd)) 
  {
    HANDLE data=::GetClipboardData(CF_TEXT);
    if (data) 
    {
      const char* ptr=(const char*)::GlobalLock(data);
      std::string contents(ptr, ::GlobalSize(data));

      if (security::manager().is_serial_number(contents))
      {
        MakeWindowText( GetDlgItem(IDC_REGISTRATION_CODE) )=contents;
        OnChange(0,0,0);
      }

      ::GlobalUnlock(data);     
    }

    ::CloseClipboard();
  }
  
  return 0;
}

LRESULT CRegistrationDlg::OnCloseCmd(WORD , WORD wID, HWND , BOOL&)
{
  if (!invert_mode_)
  {
    std::string serial=MakeWindowText( GetDlgItem(IDC_REGISTRATION_CODE) );
    security::manager().accept_serial_number(serial);
  }
  return EndDialog(wID);
}

void CRegistrationDlg::OnChange(UINT, int, HWND)
{
  if (!invert_mode_)
  {
    std::string info;
    std::string serial=MakeWindowText( GetDlgItem(IDC_REGISTRATION_CODE) );

#ifndef NDEBUG
    if (serial=="generate") {
      ::EnableWindow(GetDlgItem(IDC_REGISTRATION_INFO), true);
      SendMessage(WM_NEXTDLGCTL, (WPARAM)(HWND)GetDlgItem(IDC_REGISTRATION_INFO), (LPARAM)1);
      invert_mode_=true;
    } else if (serial=="unregister") {
      security::manager().remove_serial_number();
      EndDialog(IDOK);
    } else
#endif
    if (security::manager().is_serial_number(serial))  {
      std::string b=security::manager().get_registration_info(serial);
      info+=tokenizer::head(b, "\n")+"\n";
      info+=tokenizer::head(b, "\n")+"\n";
    } 

    MakeWindowText( GetDlgItem(IDC_REGISTRATION_INFO) )=info;
  }
}

void CRegistrationDlg::OnChangeInfo(UINT, int, HWND)
{
  if (invert_mode_)
  {
    std::string info=MakeWindowText( GetDlgItem(IDC_REGISTRATION_INFO) );

    MakeWindowText( GetDlgItem(IDC_REGISTRATION_CODE) )=
      security::manager().create_serial_number(info);
  }
}
