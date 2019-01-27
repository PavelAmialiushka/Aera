#include "stdafx.h"

#include "utilites/tokenizer.h"
#include "utilites/security.h"
#include "utilites/help.h"
#include "utilites/serl/Archive.h"

#include "wtladdons.h"

#include "LinearLocationDlg.h"
#include "PlanarLocationDlg.h"
#include "zonalLocationDlg.h"

#include "data/ZonalSetup.h"
#include "data/LinearSetup.h"
#include "data/PlanarSetup.h"

using namespace location;

void CZonalLocationDlg::DlgResize_UpdateLayout(int cx, int cy)
{
    CDialogResize<CZonalLocationDlg>::DlgResize_UpdateLayout(cx, cy);
}

CZonalLocationDlg::CZonalLocationDlg(location::ZonalSetup *setup)
{
    init(setup);
}

LRESULT CZonalLocationDlg::OnInitDialog(UINT , WPARAM , LPARAM , BOOL &)
{
    _lw(m_hWnd, IDD_LOCATION_ZONAL);

    DlgResize_Init(true, true, 0);
    MoveWindow(CRect(0, 0, 410, 170), 0);
    CenterWindow();

    //////////////////////////////////////////////////////////////////////////

    SetupMainGrid();
    ImportControlData();

    return 0;
}


LRESULT CZonalLocationDlg::OnGridItemChanged(LPNMHDR)
{
    ExportControlData();
    return 0;
}

LRESULT CZonalLocationDlg::OnCloseCmd(WORD , WORD wID, HWND hwnd, BOOL &)
{
    try
    {
        ExportControlData();

        EndDialog(0);
    }
    catch (boost::bad_lexical_cast)
    {
        SetMsgHandled(true);
        return 1;
    }
    return 0;
}

LRESULT CZonalLocationDlg::OnChangeCheck(UINT, int, HWND)
{
    return 0;
}

LRESULT CZonalLocationDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "zonal");
    return 0;
}
