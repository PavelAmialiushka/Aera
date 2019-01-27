#pragma once

#include "resource.h"

class CPropertyDlg : public CDialogImpl<CPropertyDlg>
{
public:
    enum { IDD = IDD_FILE_PROPERTY };

    BEGIN_MSG_MAP_EX(CPropertyDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

    MSG_WM_TIMER(OnTimer);

    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnTimer(UINT, TIMERPROC=0);

    void LoadData();

    CEdit		date_;
    CEdit		title_;
    CEdit		product_;
};
