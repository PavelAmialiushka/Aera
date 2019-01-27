#pragma once

#include "others/viskoe/PropertyGrid.h"

#include <atlframe.h>
#include <ATLWIN.H>
#include <atlcrack.h>

namespace nodes
{
    class filter;
    class grader;
}

class CPlotView;

class CFilterDialog
    : public CDialogImpl<CFilterDialog>
    , public WTL::CDialogResize<CFilterDialog>

{
public:

    enum {IDD = IDD_FILTER };

    BEGIN_MSG_MAP_EX(CFilterDialog)
        CHAIN_MSG_MAP(CDialogResize<CFilterDialog>)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        MSG_WM_HELP(OnHelp);

        COMMAND_ID_HANDLER(IDOK, OnIdClose)
        COMMAND_ID_HANDLER(IDCLOSE, OnIdClose)
        COMMAND_ID_HANDLER(IDCANCEL, OnIdClose)

        COMMAND_ID_HANDLER_EX(IDC_FL_RESTORE, OnRestore);

        NOTIFY_HANDLER_EX(IDC_FILTER_GRID, PIN_SELCHANGED, OnChangedFilter);

        MSG_WM_TIMER(OnTimer);

        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CFilterDialog)
        DLGRESIZE_CONTROL(IDCLOSE, DLSZ_MOVE_X|DLSZ_MOVE_Y)
        DLGRESIZE_CONTROL(IDC_FILTER_GRID, DLSZ_MOVE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_FL_RESTORE, DLSZ_MOVE_X|DLSZ_MOVE_Y)

        DLGRESIZE_CONTROL(IDC_STATIC1, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_FL_CHART3_2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()

    void DlgResize_UpdateLayout(int cx, int cy);

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnIdClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnRestore(UINT, int, HWND);

    LRESULT OnChangedFilter(LPNMHDR);

    LRESULT OnTimer(UINT, TIMERPROC=0);

    LRESULT OnHelp(LPHELPINFO);

private:

    void UpdateGridValues();

private:

    CPropertyGridCtrl grid_;

    shared_ptr<CPlotView> plot3_;
    shared_ptr<CPlotView> plot1_;
    shared_ptr<CPlotView> plot2_;
    shared_ptr<nodes::filter> node_;
    shared_ptr<nodes::filter> filter_;
    shared_ptr<nodes::grader> grader_;
};

