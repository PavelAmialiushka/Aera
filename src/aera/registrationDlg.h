#pragma once

#include "resource.h"

class CRegistrationDlg : public CDialogImpl<CRegistrationDlg >
{
public:
    enum { IDD = IDD_REGISTRATION_CODE };

    BEGIN_MSG_MAP_EX(CNagDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

    COMMAND_HANDLER_EX(IDC_REGISTRATION_CODE, EN_CHANGE, OnChange);
    COMMAND_HANDLER_EX(IDC_REGISTRATION_INFO, EN_CHANGE, OnChangeInfo);

    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    void OnChange(UINT, int, HWND);
    void OnChangeInfo(UINT, int, HWND);


    bool invert_mode_;
};
