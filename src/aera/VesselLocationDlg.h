#pragma once

#include "resource.h"
#include "VesselSensorWidget.h"

#include "LocationDlgBase.h"

#include "others/viskoe/PropertyGrid.h"

#include "data/VesselSetup.h"

//////////////////////////////////////////////////////////////////////////

class CVesselLocationDlg
        : public CDialogImpl<CVesselLocationDlg>
        , public CLocationDlgBase<CVesselLocationDlg>
        , public WTL::CDialogResize<CVesselLocationDlg>
{
public:
    enum { IDD = IDD_LOCATION_VESSEL };

    typedef CLocationDlgBase<CVesselLocationDlg> LocationDlgBase;
public:

    CVesselLocationDlg(location::VesselSetup *llc);
    ~CVesselLocationDlg();

    enum
    {
        CLM_HEADER = CLM_NEXT,
        CLM_UNITS,
        CLM_DEVIATION,
        CLM_VESSELORIENTATION,
    };

public:
    BEGIN_MSG_MAP_EX(CVesselLocationDlg)
        CHAIN_MSG_MAP(CDialogResize<CVesselLocationDlg>)

        MSG_WM_INITDIALOG(OnInitDialog)

        NOTIFY_HANDLER_EX(IDC_VL_CHLIST, PIN_ITEMCHANGED, OnSensorGridItemChanged)
        NOTIFY_HANDLER_EX(IDC_VL_CHLIST, LVN_KEYDOWN, OnGridKeyDown)
        NOTIFY_HANDLER_EX(IDC_VL_CHLIST, NM_RCLICK,	OnGridRClick)

        NOTIFY_HANDLER_EX(IDC_VL_PARTS, PIN_ITEMCHANGED, OnVesselGridItemChanged)

        NOTIFY_HANDLER_EX(IDC_ZL_GRID, PIN_ITEMCHANGED, OnGridItemChanged)

        COMMAND_ID_HANDLER(IDC_VL_ADD, OnAddCommand)
        COMMAND_ID_HANDLER(IDC_VL_REMOVE, OnRemoveCommand)

        COMMAND_ID_HANDLER(IDC_VL_ADD_PART, OnAddPart)
        COMMAND_ID_HANDLER(IDC_VL_REMOVE_PART, OnRemovePart)

        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        MSG_WM_HELP(OnHelp);
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CVesselLocationDlg)
        DLGRESIZE_CONTROL(IDC_STATIC2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_INSERT_POINT, DLSZ_SIZE_X|DLSZ_SIZE_Y)

        DLGRESIZE_CONTROL(IDC_ZL_GRID1,  DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_VL_ADD,    DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_VL_REMOVE, DLSZ_MOVE_X)

        DLGRESIZE_CONTROL(IDC_VL_PARTS,       DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_VL_ADD_PART,    DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_VL_REMOVE_PART, DLSZ_MOVE_X)

        DLGRESIZE_CONTROL(IDC_VL_CHLIST, DLSZ_MOVE_X|DLSZ_SIZE_Y)

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

    LRESULT OnAddPart(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnRemovePart(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnInitDialog(HWND, LPARAM);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    // notify
    LRESULT OnGridItemChanged(LPNMHDR);
    LRESULT OnSensorGridItemChanged(LPNMHDR);
    LRESULT OnVesselGridItemChanged(LPNMHDR);
    LRESULT OnGridKeyDown(LPNMHDR);
    LRESULT OnGridRClick(LPNMHDR);

    LRESULT OnHelp(LPHELPINFO);

private:
    void  SetupMainGrid();

    void  ImportControlData();
    void  ExportControlData();

    void  InsertNewPart();
    void  DeleteCurrentPart();
    void  ImportVesselGridData();
    void  UpdateVesselGridData();
    void  ExportVesselGridData();

    void  InsertNewSensor();
    void  DeleteCurrentSensor();
    void  ImportSensorGridData();    
    void  ExportSensorGridData();
    void  SortSensorGrid();

    void  UpdateSensorWidget();


private:

    int     lock_changes_;

    combo_list sensor_placement_type_;
    combo_list vessel_orientation_;
    combo_list vessel_parts_;
    combo_list yaxe_type_;

    CVesselSensorWidget sensor_widget_;

    CPropertyGridCtrl   sensor_grid_;
    CPropertyGridCtrl   vessel_grid_;

    location::VesselStructure vessel_;
    location::VesselSetup  *loc_;
};

