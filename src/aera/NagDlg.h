#pragma once

#include "resource.h"

class CNagDlg: public CDialogImpl<CNagDlg>
{
public:
    enum { IDD = IDD_NAG };

    BEGIN_MSG_MAP_EX(CNagDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)

    MSG_WM_TIMER(OnTimer);

    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnTimer(UINT, TIMERPROC=0);

    int timer_;
};
