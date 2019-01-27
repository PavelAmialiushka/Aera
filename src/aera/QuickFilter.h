#pragma once

#include "others\viskoe\RtfScriptEditor.h"
#include "others/EditNumber.h"

class CQuickFilter
    : public CDialogImpl<CQuickFilter>
{
public:

    enum {IDD = IDD_QUICK_FILTER};

    BEGIN_MSG_MAP_EX(CQuickFilter)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        MSG_WM_HELP(OnHelp);

        MSG_WM_TIMER(OnTimer);

        COMMAND_ID_HANDLER(IDCLOSE, OnIdClose)
        COMMAND_ID_HANDLER(IDCANCEL, OnIdClose)

        COMMAND_ID_HANDLER_EX(IDC_QF_SAVE, OnSave);
        COMMAND_ID_HANDLER_EX(IDC_QF_LOAD, OnLoad);

        COMMAND_RANGE_HANDLER_EX(IDC_QF_MRU_FIRST, IDC_QF_MRU_FIRST+20, OnMRU);
        COMMAND_RANGE_HANDLER_EX(IDC_QF_MRUV_FIRST, IDC_QF_MRUV_FIRST+20, OnMRUV);

        COMMAND_ID_HANDLER_EX(IDC_QF_TIME, OnTimeChanged)

        COMMAND_HANDLER_EX(IDC_QF_TEXT, EN_CHANGE, OnTextUpdate);

        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()


    void    FillInVariables();
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);

    void    OnTimer(UINT, TIMERPROC=0);

    LRESULT OnIdClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnTimeChanged(UINT, int, HWND);

    LRESULT OnSave(UINT, int, HWND);
    LRESULT OnLoad(UINT, int, HWND);

    LRESULT OnMRU(UINT, int, HWND);
    LRESULT OnMRUV(UINT, int, HWND);

    LRESULT OnTextUpdate(UINT, int, HWND);

    LRESULT OnHelp(LPHELPINFO);

private:

    std::string Store();
    void Restore(std::string);

private:

    void ExportText();
    void SetRTFEdit();

private:

    CRtfScriptEditorCtrl text_;
    CNumberEdit          before_;
    CNumberEdit          after_;

    std::vector<std::string>  serl_array_;
};

