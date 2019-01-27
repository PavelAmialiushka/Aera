#pragma once

#include "controls\PlotModel.h"

#include "aera\resource.h"

class CPlotPropertyDlg
    :  public CDialogImpl<CPlotPropertyDlg>
    ,  public process::host_listener
{
public:
    enum { IDD=IDD_PLOTPPCOMMON };

    CPlotPropertyDlg(plot_model *protocol, HWND);
    ~CPlotPropertyDlg();

    typedef CDialogImpl<CPlotPropertyDlg> baseClass;
    BEGIN_MSG_MAP_EX(CPlotPropertyDlg)
        MSG_WM_INITDIALOG(OnInitDialog)

        COMMAND_HANDLER_EX(IDC_PP_USE_LOCATION, BN_CLICKED, OnLocationChanged);
        COMMAND_HANDLER_EX(IDC_PP_LOCATION, CBN_SELCHANGE, OnLocationChanged);
        COMMAND_ID_HANDLER_EX(IDC_SHOW_TDD, OnTddChanged);

        COMMAND_HANDLER_EX(IDC_PLOT_XAXE_AUTO, BN_CLICKED, OnXAutoClick);
        COMMAND_HANDLER_EX(IDC_PLOT_YAXE_AUTO, BN_CLICKED, OnYAutoClick);

        COMMAND_ID_HANDLER_EX(IDOK, OnClose);
        COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose);

        COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnAnyChange);
        COMMAND_CODE_HANDLER_EX(EN_UPDATE, OnAnyChange);
        COMMAND_CODE_HANDLER_EX(CBN_SELCHANGE, OnAnyChange);

        MSG_WM_TIMER(OnTimer);

        MSG_WM_HELP(OnHelp);

        //CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    LRESULT OnInitDialog(HWND, LPARAM);
    LRESULT OnClose(UINT, int, HWND);

    LRESULT OnHelp(LPHELPINFO=0);

    void safe_on_finish(process::prslt);

    void OnAnyChange(...);
    void OnLocationChanged(...);
    void OnTddChanged(...);

    void OnXAutoClick(UINT, int, HWND);
    void OnYAutoClick(UINT, int, HWND);

    LRESULT OnTimer(UINT);

    object_id getLocationByIndex(int selectionIndex);

protected:

    void DelayedApply();
    void Apply();

    void MakeFeatureList();

private:

    bool GetActualTddCheck();

private:

    bool      invalid_;
    bool      ignore_;

    CWindow   wndParent;
    CComboBox xbox_, ybox_;
    CButton   xauto_,yauto_;
    CButton   xlog_, ylog_;
    CEdit     xmin_, xmax_;
    CEdit     ymin_, ymax_;
    CButton   pm_scatter_,
              pm_line_;

    CButton   bychannel_;
    CButton   tddata_;
    CComboBox linestyle_;
    CComboBox binnumber_;

    CButton   use_location_;
    CButton   use_clusters_;
    CComboBox location_type_;

    std::vector<aera::chars> features_;
    std::vector<std::string> strings_;

    plot_model  *model_;
};
