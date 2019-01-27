#pragma once

#include "resource.h"
#include "utilites/help.h"

#include "data/LocationSetup.h"

class CNewLocation : public CDialogImpl<CNewLocation>
{
public:
    // vessel
    enum { IDD = IDD_NEW_LOCATION2 };

    BEGIN_MSG_MAP_EX(CNewLocation)

        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_CODE_HANDLER(BN_DBLCLK, OnDblClk);

        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        MSG_WM_HELP(OnHelp);

    END_MSG_MAP()


    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        _lw(m_hWnd, IDD_NEW_LOCATION2);

        CheckRadioButton(
            IDC_NL_ZONAL, IDC_NL_VESSEL,
            IDC_NL_LINEAR);

        return 0;
    }

    LRESULT OnDblClk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        SendMessage(WM_COMMAND, IDOK, 0);
        return 0;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        if (wID!=IDCANCEL)
        {
            if (IsDlgButtonChecked(IDC_NL_ZONAL)) loc_=loc_.create(location::locZonal);
            if (IsDlgButtonChecked(IDC_NL_LINEAR)) loc_=loc_.create(location::locLinear);
            if (IsDlgButtonChecked(IDC_NL_PLANAR)) loc_=loc_.create(location::locPlanar);
            if (IsDlgButtonChecked(IDC_NL_VESSEL)) loc_=loc_.create(location::locVessel);
        }

        return EndDialog(wID);;
    }

    LRESULT OnHelp(LPHELPINFO)
    {
        //help(m_hWnd, "newlocation");
        return 0;
    }

    location::Setup loc_;

};
