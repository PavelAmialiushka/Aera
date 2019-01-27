#pragma once

#include "resource.h"

#include <atlwin.h>
#include <atlcrack.h>

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
    enum { IDD = IDD_ABOUTBOX };

    BEGIN_MSG_MAP_EX(CAboutDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        COMMAND_ID_HANDLER_EX(IDC_REGISTER, OnRegistration);

        NOTIFY_HANDLER_EX(IDC_AERA_ICON, STN_DBLCLK, OnIconDblClk);

        MSG_WM_KEYDOWN(OnChar);
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    void OnRegistration(UINT, int, HWND);
    void UpdateRegistrationInfo();
    LRESULT OnIconDblClk(LPNMHDR);

    LRESULT OnChar(TCHAR, UINT, UINT);

private:

    CHyperLink url_;
    CHyperLink url2_;
    CStatic    reg1_;
    CStatic    reg2_;
};
