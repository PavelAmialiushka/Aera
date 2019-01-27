#pragma once

#include "resource.h"
#include "LinearSensorWidget.h"

#include "others/viskoe/PropertyGrid.h"

#include "data/LinearSetup.h"

#include "LocationDlgBase.h"

//////////////////////////////////////////////////////////////////////////
class linear_sensor_map;

class CLinearLocationDlg
        : public CDialogImpl<CLinearLocationDlg>
        , public CLocationDlgBase<CLinearLocationDlg>
        , public WTL::CDialogResize<CLinearLocationDlg>
{
public:
    enum { IDD = IDD_LOCATION_LINEAR };

public:

    CLinearLocationDlg(location::LinearSetup *llc);
    ~CLinearLocationDlg();

    enum
    {
        CLM_HEADER = CLM_NEXT,
        CLM_UNITS,
        CLM_CIRCULAR,
    };

    BEGIN_MSG_MAP_EX(CLinearLocationDlg)
        CHAIN_MSG_MAP(CDialogResize<CLinearLocationDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)

        NOTIFY_HANDLER_EX(IDC_LL_CHLIST, PIN_ITEMCHANGED, OnGridItemChanged);
        NOTIFY_HANDLER_EX(IDC_LL_CHLIST, LVN_KEYDOWN,     OnGridKeyDown);
        NOTIFY_HANDLER_EX(IDC_LL_CHLIST, NM_RCLICK,       OnGridRClick);

        NOTIFY_HANDLER_EX(IDC_ZL_GRID, PIN_ITEMCHANGED, OnGridItemChanged)

        COMMAND_ID_HANDLER(IDC_LL_ADD, OnAddCommand);
        COMMAND_ID_HANDLER(IDC_LL_REMOVE, OnRemoveCommand);

        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        MSG_WM_HELP(OnHelp);
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CLinearLocationDlg)
        DLGRESIZE_CONTROL(IDC_STATIC3, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_INSERT_POINT, DLSZ_SIZE_X|DLSZ_SIZE_Y)

        DLGRESIZE_CONTROL(IDC_ZL_GRID,   0)
        DLGRESIZE_CONTROL(IDC_LL_ADD,    0)
        DLGRESIZE_CONTROL(IDC_LL_REMOVE, 0)
        DLGRESIZE_CONTROL(IDC_LL_CHLIST, DLSZ_SIZE_X)

        DLGRESIZE_CONTROL(IDOK,          DLSZ_MOVE_X|DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    void DlgResize_UpdateLayout(int cx, int cy);
private:

    LRESULT OnGridItemChanged(LPNMHDR hdr);

    LRESULT OnAddCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnRemoveCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnInitDialog(HWND, LPARAM);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    // notify
    LRESULT OnSensorGridItemChanged(LPNMHDR);
    LRESULT OnGridKeyDown(LPNMHDR);
    LRESULT OnGridRClick(LPNMHDR);

    LRESULT OnHelp(LPHELPINFO);

private:
    void        SetupMainGrid();

    void        ImportControlData();
    void        ImportCoordinateGridData();
    void        ImportSensorData(int);
    void        ImportSensorWidget();

    void        ExportControlData();
    void        ExportSensorGridData();
    void        ExportSensorData(int);

    void        SortData();
    void        InsertNewLine();
    void        DeleteLine();

private:

    shared_ptr<linear_sensor_map> sensors_map_;

private:
    int                 lock_changes_;

    CLinearSensorWidget sensor_widget_;
    CPropertyGridCtrl   sensor_grid_;

    location::LinearSetup  *loc_;
};

