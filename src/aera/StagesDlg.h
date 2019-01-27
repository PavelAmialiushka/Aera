#pragma once

#include "resource.h"
#include "data/stage.h"
#include "data/node.h"

#include "others/wtl/atlframe.h"


class stage;

class CStagesDlg
        : public CDialogImpl<CStagesDlg>
        , public WTL::CDialogResize<CStagesDlg>
{
public:
    enum { IDD = IDD_STAGES };

    BEGIN_MSG_MAP_EX(CStagesDlg)
        CHAIN_MSG_MAP(CDialogResize<CStagesDlg>)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        if (!edit_label_)
        {
            COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
            COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        }

        COMMAND_ID_HANDLER_EX(ID_APPEND_STAGE, OnAppendStage);
        COMMAND_ID_HANDLER_EX(ID_DELETE_STAGE, OnDeleteStage);
        COMMAND_ID_HANDLER_EX(ID_RENAME_STAGE, OnRenameStage);

        COMMAND_HANDLER_EX(IDC_STAGE_NAME2, EN_CHANGE, OnEditChange);
        COMMAND_HANDLER_EX(IDC_STAGE_START, EN_CHANGE, OnEditChange);
        COMMAND_HANDLER_EX(IDC_STAGE_END,   EN_CHANGE, OnEditChange);

        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, NM_RCLICK, OnTreeRClick);
        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, NM_DBLCLK, OnTreeDblClk);

        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, TVN_SELCHANGED, OnSelChanged);
        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, TVN_BEGINLABELEDIT, OnBeginLabelEdit);
        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, TVN_ENDLABELEDIT, OnEndLabelEdit);
        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, TVN_ITEMEXPANDING, OnTreeExpanding);
        NOTIFY_HANDLER_EX(IDC_STAGE_TREE, TVN_KEYDOWN, OnKeyDown);

        MESSAGE_HANDLER_EX(WM_USER+1, OnUpdateRange);
        MESSAGE_HANDLER_EX(WM_USER+2, OnUpdateRange2);

        MSG_WM_TIMER(OnTimer);

        MSG_WM_HELP(OnHelp);
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CStagesDlg)
        DLGRESIZE_CONTROL(IDC_STAGE_TREE, DLSZ_SIZE_Y)

        DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_STATIC2, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_STATIC3, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_STATIC5, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_NAME2, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_START, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_END, DLSZ_MOVE_X|DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_STAGE_STATIC0, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_PLOTPOS, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_STAGE_PLOTPOS2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:

    LRESULT OnUpdateRange(UINT, WPARAM, LPARAM);
    LRESULT OnUpdateRange2(UINT, WPARAM, LPARAM);

    // command handlers
    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnAppendStage(UINT, int, HWND);
    LRESULT OnDeleteStage(UINT, int, HWND);
    LRESULT OnRenameStage(UINT, int, HWND);

    LRESULT OnEditChange(UINT, int, HWND);

    // tree notification
    LRESULT OnTreeRClick(LPNMHDR);
    LRESULT OnTreeDblClk(LPNMHDR);

    LRESULT OnKeyDown(LPNMHDR);
    LRESULT OnSelChanged(LPNMHDR);
    LRESULT OnBeginLabelEdit(LPNMHDR);
    LRESULT OnEndLabelEdit(LPNMHDR);
    LRESULT OnTreeExpanding(LPNMHDR);

    void    OnTimer(UINT, TIMERPROC=0);
    LRESULT OnHelp(LPHELPINFO=0);

private:

    int  GetLevel(CTreeItem=0);
    stage &GetStage(CTreeItem=0);
    stage &GetRealStage(CTreeItem=0);

    void SyncEditCtrls();

    void CreateTree();
    void BuildTreeFromStage(CTreeItem item, const stage &stg);
    void BuildTreeFromGraph();

    CTreeItem GetTreeItemByStage(const stage &pattern);

    void TryToGetResult();
    double GetMaxTime();

private:

    nodes::presult  result_;
    stage           current_stage_;

    CTreeViewCtrlEx tree_;
    CTreeItem  root_item_;
    CTreeItem  current_item_;

    CEdit stage_name_;
    CEdit start_time_;
    CEdit end_time_;
    bool edit_label_;
    bool	updating_;

    class CPlotView *view_;
};
