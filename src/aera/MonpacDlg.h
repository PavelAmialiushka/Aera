#pragma once

#include "aera\resource.h"
#include "data\stage.h"

#include "data\grader.h"
#include "data\criterion.h"

#include "data\monpac.h"

//////////////////////////////////////////////////////////////////////////

class CMonpacDialog
    : public CDialogImpl<CMonpacDialog>
    , public CCustomDraw<CMonpacDialog>
    , process::host_listener
{
public:
    enum { IDD = IDD_MONPAC};

    ~CMonpacDialog();

    BEGIN_MSG_MAP_EX(CMonpacDialog)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

    COMMAND_ID_HANDLER(ID_MP_COPY, CopyCmd)

    COMMAND_HANDLER_EX(IDC_MP_STAGES, CBN_SELCHANGE, OnChangedStage)
    COMMAND_HANDLER_EX(IDC_MP_VESSEL, CBN_SELCHANGE, OnChanged)

    COMMAND_HANDLER_EX(IDC_MP_RESULTTYPE, CBN_SELCHANGE, OnChanged)

    COMMAND_ID_HANDLER_EX(IDC_MP_EVAL,   OnChangedMode);
    COMMAND_ID_HANDLER_EX(IDC_MP_INTENS, OnChangedMode);
    COMMAND_ID_HANDLER_EX(IDC_MP_GRAPH,  OnChangedMode);

    COMMAND_ID_HANDLER_EX(IDC_MP_TYPE1, OnChanged);
    COMMAND_ID_HANDLER_EX(IDC_MP_TYPE2, OnChanged);
    COMMAND_ID_HANDLER_EX(IDC_MP_TYPE3, OnChanged);

    COMMAND_ID_HANDLER_EX(IDC_MP_IT_EVAL, OnChanged);
    COMMAND_ID_HANDLER_EX(IDC_MP_IT_TEST, OnChanged);

    COMMAND_ID_HANDLER_EX(IDC_MP_UPDATE, OnUpdate);

    COMMAND_HANDLER_EX(IDC_MP_LOCKOUT, EN_CHANGE,   OnChanged)

    MSG_WM_TIMER(OnTimer);

    MSG_WM_DRAWITEM(OnDrawItem);

    MSG_WM_HELP(OnHelp);
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT CopyCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    void OnUpdate(...);

    void OnChangedStage(...);
    void OnChanged(...);
    void OnChangedMode(...);

    void safe_on_finish(process::prslt);

    LRESULT OnTimer(UINT, TIMERPROC=0);

    LRESULT OnDrawItem(UINT, LPDRAWITEMSTRUCT);

    LRESULT OnHelp(LPHELPINFO=0);

private:

    void Apply();
    void DelayedApply();

    void ExportCriterion();
    void ImportCriterion();

    void SetMonpacMode(int);
    void DrawItem(CDCHandle dc, int i, int si, std::string str, bool clr);

private:
    // params
    CComboBox stages_;
    CComboBox location_;

    // result
    CListViewCtrl	grid_;

    int		intens_mode_;
    COLORREF textcolor_;

    bool					lock_changes_;
    bool					update_needed_;
    UINT					timer_;
    int           selected_item_;

    shared_ptr<monpac::classifier> monpac_;
    shared_ptr<monpac::monpac_result> result_;

    monpac::criterion criterion_;
    object_id   id_;

    shared_ptr<class nodes::grader>   grader_;

    class CMonpacWindow *view_;

};

////////////////////////////////////////////////////////////////////////////
