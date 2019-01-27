#pragma once

#include "resource.h"


class CSettingsDlg : public CDialogImpl<CSettingsDlg>
{
public:
    enum { IDD = IDD_SETTINGSDLG };

    BEGIN_MSG_MAP_EX(CSettingsDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

    COMMAND_ID_HANDLER_EX(IDC_SET_PARAMETRIC1, OnSetParametric)
    COMMAND_ID_HANDLER_EX(IDC_SET_PARAMETRIC2, OnSetParametric)
    COMMAND_ID_HANDLER_EX(IDC_SET_PARAMETRIC3, OnSetParametric)
    COMMAND_ID_HANDLER_EX(IDC_SET_PARAMETRIC4, OnSetParametric)

    MSG_WM_HELP(OnHelp);

    MSG_WM_CTLCOLOREDIT(OnCtlColorEdit);
    COMMAND_CODE_HANDLER_EX(EN_CHANGE, OnChangeEdit);
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnSetParametric(UINT, int, HWND);

    LRESULT OnCtlColorEdit(HDC, HWND);
    LRESULT OnChangeEdit(UINT, int, HWND);

    LRESULT OnHelp(LPHELPINFO);

private:

    CBrush redbrush_;

};
