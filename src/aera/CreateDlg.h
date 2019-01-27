#pragma once

#include "resource.h"


class CCreateDlg : public CDialogImpl<CCreateDlg>
{
public:
    enum { IDD = IDD_CREATE_PROJECT };

    BEGIN_MSG_MAP_EX(CCreateDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

    MSG_WM_TIMER(OnTimer);

    COMMAND_ID_HANDLER(IDOK, OnCloseCmd);
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

    COMMAND_CODE_HANDLER(BN_CLICKED, OnClicked);
    COMMAND_CODE_HANDLER(BN_DBLCLK, OnDblClicked);

    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnDblClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnTimer(UINT, TIMERPROC=0);
private:

    void DrawTimer();

private:

    unsigned count_;
};
