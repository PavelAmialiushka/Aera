#pragma once

#include "resource.h"

#include "others/EditNumber.h"
#include "others/viskoe/PropertyGrid.h"

#include "data/ZonalSetup.h"

#include "LocationDlgBase.h"

class CZonalLocationDlg
        : public CDialogImpl<CZonalLocationDlg>
        , public CLocationDlgBase<CZonalLocationDlg>
        , public WTL::CDialogResize<CZonalLocationDlg>

{
public:
    enum { IDD = IDD_LOCATION_ZONAL };

    BEGIN_MSG_MAP_EX(CZonalLocationDlg)
        CHAIN_MSG_MAP(CDialogResize<CZonalLocationDlg>)

        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnChangeCheck);
        NOTIFY_CODE_HANDLER_EX(PIN_ITEMCHANGED, OnGridItemChanged)

        MSG_WM_HELP(OnHelp);
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    BEGIN_DLGRESIZE_MAP(CZonalLocationDlg)
        DLGRESIZE_CONTROL(IDC_STATIC2,   DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_ZL_GRID,   DLSZ_SIZE_X|DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDOK,          DLSZ_MOVE_X|DLSZ_MOVE_Y)
    END_DLGRESIZE_MAP()

    void DlgResize_UpdateLayout(int cx, int cy);

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    CZonalLocationDlg( location::ZonalSetup *setup );

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnGridItemChanged(LPNMHDR);

    LRESULT OnChangeCheck(UINT, int, HWND);

    LRESULT OnHelp(LPHELPINFO);

};
