#pragma once

#include "resource.h"
#include "PlanarSensorWidget.h"

#include "LocationDlgBase.h"

#include "others/viskoe/PropertyGrid.h"

#include "data/PlanarSetup.h"

#include <atlframe.h>

//////////////////////////////////////////////////////////////////////////

class CPlanarLocationDlg
        : public CDialogImpl<CPlanarLocationDlg>
        , public CLocationDlgBase<CPlanarLocationDlg>
        , public WTL::CDialogResize<CPlanarLocationDlg>
{
public:
    enum { IDD = IDD_LOCATION_PLANAR };

public:

    CPlanarLocationDlg(location::PlanarSetup *llc);
    ~CPlanarLocationDlg();

    enum
    {
        CLM_HEADER = CLM_NEXT,
        CLM_UNITS,
        CLM_DEVIATION,
        CLM_HORVESSEL,
        CLM_VERTVESSEL,
    };

public:
    BEGIN_MSG_MAP_EX(CPlanarLocationDlg)
        CHAIN_MSG_MAP(CDialogResize<CPlanarLocationDlg>)

        MSG_WM_INITDIALOG(OnInitDialog)

        NOTIFY_HANDLER_EX(IDC_LL_CHLIST, PIN_ITEMCHANGED, OnSensorGridItemChanged);
        NOTIFY_HANDLER_EX(IDC_LL_CHLIST, LVN_KEYDOWN, OnGridKeyDown);
        NOTIFY_HANDLER_EX(IDC_LL_CHLIST, NM_RCLICK,	OnGridRClick);

        NOTIFY_HANDLER_EX(IDC_ZL_GRID, PIN_ITEMCHANGED, OnGridItemChanged)

        COMMAND_ID_HANDLER(IDC_LL_ADD, OnAddCommand);
        COMMAND_ID_HANDLER(IDC_LL_REMOVE, OnRemoveCommand);

        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        MSG_WM_HELP(OnHelp);
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CPlanarLocationDlg)
        DLGRESIZE_CONTROL(IDC_STATIC2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_INSERT_POINT, DLSZ_SIZE_X|DLSZ_SIZE_Y)

        DLGRESIZE_CONTROL(IDC_ZL_GRID1,  DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_LL_ADD,    DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_LL_REMOVE, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_LL_CHLIST, DLSZ_MOVE_X|DLSZ_SIZE_Y)

        DLGRESIZE_CONTROL(IDOK,          DLSZ_MOVE_X|DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    void DlgResize_UpdateLayout(int cx, int cy);
private:

    LRESULT OnAddCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnRemoveCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnInitDialog(HWND, LPARAM);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    // notify
    LRESULT OnGridItemChanged(LPNMHDR);
    LRESULT OnSensorGridItemChanged(LPNMHDR);
    LRESULT OnGridKeyDown(LPNMHDR);
    LRESULT OnGridRClick(LPNMHDR);

    LRESULT OnHelp(LPHELPINFO);

private:
    void  SetupMainGrid();

    void  ImportControlData();
    void  ImportCoordinateGridData();

    void  ExportControlData();
    void  ExportCoordinateGridData();

    void  UpdateSensorWidget();

    void  SortSensorGrid();
    void  InsertNewLine();
    void  DeleteCurrentLine();

private:

    int     lock_changes_;

    CPlanarSensorWidget sensor_widget_;
    CPropertyGridCtrl   sensor_grid_;

    location::PlanarSetup *loc_;
};

