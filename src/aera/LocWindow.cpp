#include "stdafx.h"

#include "LocWindow.h"

#include "zonalLocationDlg.h"
#include "LinearLocationDlg.h"
#include "PlanarLocationDlg.h"
#include "VesselLocationDlg.h"

#include "NagDlg.h"

#include "utilites/Localizator.h"
#include "utilites/serl/Archive.h"
#include "utilites/document.h"

#include "data/locationSetup.h"
#include "data/nodeFactory.h"

#include "toolbarbtns.h"

#include "NewLocation.h"

#include "data/LocationSetup.h"
#include "data/ZonalSetup.h"
#include "data/LinearSetup.h"
#include "data/PlanarSetup.h"
#include "data/VesselSetup.h"

CLocWnd::CLocWnd()
{
}


void CLocWnd::serialization(serl::archiver &arc)
{
}

HWND CLocWnd::Create(HWND hparent)
{
    ignore_=0;
    CWindow parent(hparent);
    CRect prc; parent.GetWindowRect(prc);
    parent.ClientToScreen(prc);

    return baseClass::Create(
               parent, CRect(prc.TopLeft(), CSize(200, 200)), "",
               WS_VISIBLE|WS_CAPTION|WS_POPUP|WS_OVERLAPPED
               |WS_CLIPSIBLINGS|WS_CLIPCHILDREN
               |WS_SYSMENU|WS_THICKFRAME);
}

LRESULT CLocWnd::OnCreate(LPCREATESTRUCT)
{
    listctrl_.Create(m_hWnd, rcDefault, "",
                     WS_VISIBLE|WS_CHILD|WS_BORDER
                     |LVS_REPORT|LVS_SINGLESEL|LVS_NOCOLUMNHEADER);
    listctrl_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

    listctrl_.InsertColumn(0, "Location", 0, 200, 0);


    toolbar_.Create(m_hWnd, rcDefault, "",
                    WS_CHILD|WS_VISIBLE
                    |TBSTYLE_FLAT
                    |TBSTYLE_LIST
                    |CCS_TOP);
    toolbar_.SetButtonStructSize();

    CToolbarButtons main=IDR_LOCATION_WND;
    main.Append(IDC_LOCWIN_INSERT, 0, ""/*_lcs("LW-Insert#Insert")*/, TBSTYLE_AUTOSIZE);
    main.Append(IDC_LOCWIN_DELETE, 1, ""/*_lcs("LW-Delete#Delete")*/, TBSTYLE_AUTOSIZE);
    main.Append();
    main.Append(IDC_LOCWIN_PROPERTY, 2, "", TBSTYLE_AUTOSIZE);
    main.Apply(toolbar_);
    OnChangeLanguage();

    OnSizeChanged(0, 0, 0);

    BuildList();
    nodes::factory().location_slot().connect(this,
            bind(&CLocWnd::UpdateList, this));

    return 0;
}

void CLocWnd::OnChangeLanguage()
{
    SetWindowText(_lcs("Location"));
}

LRESULT CLocWnd::OnDestroy()
{
    nodes::factory().location_slot().disconnect(this);
    return 0;
}


void CLocWnd::UpdateList()
{
    if (!ignore_)
    {
        ignore_=true;
        listctrl_.DeleteAllItems();
        BuildList();
        ignore_=false;
    }
}

void CLocWnd::BuildList()
{
    listctrl_.InsertItem(0, _lcs("No location"));

    std::vector< location::Setup > lprm=
        nodes::factory().get_location_setup();

    bool selected=false;
    unsigned cloc=
        nodes::factory().get_sel_location();

    unsigned count=lprm.size();
    for (unsigned index=0; index<count; ++index)
    {
        std::string name=lprm[index].name();
        listctrl_.InsertItem(index+1, name.c_str());
        if (index==cloc)
        {
            selected=true;
            listctrl_.SelectItem(index+1);
        }
    }
    if (!selected) listctrl_.SelectItem(0);
}

LRESULT CLocWnd::OnEraseBackground(HDC hdc)
{
    CDCHandle dc=hdc;

    CRect rc; GetClientRect(rc);
    dc.FillRect(rc, GetSysColorBrush(COLOR_3DFACE));

    return 0;
}


LRESULT CLocWnd::OnSizeChanged(UINT uMsg, WPARAM, LPARAM)
{
    CRect rc; GetClientRect(rc);
    rc.DeflateRect(5, 5);

    if (toolbar_)
    {
        toolbar_.AutoSize();

        CRect tb;
        toolbar_.GetWindowRect(tb);
        ScreenToClient(tb);

        rc.top=tb.bottom+5;
    }

    listctrl_.MoveWindow(rc);

    CRect lrc; listctrl_.GetClientRect(lrc);
    listctrl_.SetColumnWidth(0, lrc.Width()-3);

    return 0;
}

LRESULT CLocWnd::OnItemChanged(LPNMHDR hdr)
{
    LPNMLISTVIEW nm=(LPNMLISTVIEW)hdr;
    if (nm->uNewState & LVIS_SELECTED)
    {
        int index=nm->iItem;
        if (index==0)
        {
            nodes::factory().set_sel_location(-1);
        }
        else
        {
            nodes::factory().set_sel_location(index-1);
        }

        if (toolbar_)
        {
            TBBUTTONINFO info=
            {
                sizeof(TBBUTTONINFO),
                TBIF_STATE, 0, 0,
                index==0 ? 0 : TBSTATE_ENABLED
            };
            toolbar_.SetButtonInfo(IDC_LOCWIN_PROPERTY, &info);

            info.fsState=index<2 ? 0 : TBSTATE_ENABLED;
            toolbar_.SetButtonInfo(IDC_LOCWIN_DELETE, &info);
        }
    }
    return 0;
}

LRESULT CLocWnd::OnItemDblClick(LPNMHDR hdr)
{
    LPNMLISTVIEW nm=(LPNMLISTVIEW)hdr;
    if (nm->iItem!=0)
    {
        OnProperty(0, nm->iItem, 0);
    }
    return 0;
}

LRESULT CLocWnd::OnProperty(UINT, int pos, HWND)
{
    Document().Nag();

    std::vector< location::Setup > setups=
        nodes::factory().get_location_setup();

    int index=listctrl_.GetSelectedIndex();
    if (index>=1)
    {
        --index;

        if (setups[index].planar())
        {
            CPlanarLocationDlg(setups[index].planar()).DoModal();
        } else if (setups[index].linear())
        {
            CLinearLocationDlg(setups[index].linear()).DoModal();
        } else if (setups[index].vessel())
        {
            CVesselLocationDlg( setups[index].vessel()).DoModal();
        } else if (setups[index].zonal())
        {
            CZonalLocationDlg( setups[index].zonal() ).DoModal();
        }


        nodes::factory().set_location_setup( setups );
    }

    return 0;
}

LRESULT CLocWnd::OnInsertPos(UINT, int pos, HWND)
{
    CNewLocation dlg;
    if (dlg.DoModal()!=IDCANCEL)
    {
        std::vector< location::Setup > lcks=
            nodes::factory().get_location_setup();

        lcks.push_back( dlg.loc_ );
        nodes::factory().set_location_setup( lcks );

        listctrl_.InsertItem(listctrl_.GetItemCount()-1,
                             dlg.loc_.name().c_str());

        listctrl_.SetItemState(listctrl_.GetItemCount()-2,
                               LVIS_SELECTED, LVIS_SELECTED);

        OnProperty(0, pos, 0);
    }

    return 0;
}

LRESULT CLocWnd::OnDeletePos(UINT, int pos, HWND)
{
    std::vector< location::Setup > lcks=
        nodes::factory().get_location_setup();

    int index=listctrl_.GetSelectedIndex();
    if (index >= 2)
    {
        listctrl_.DeleteItem(index);
        listctrl_.SetItemState(
            std::min(index, listctrl_.GetItemCount()-1),
            LVIS_SELECTED, LVIS_SELECTED);

        --index;
        lcks.erase(lcks.begin()+index);
        nodes::factory().set_location_setup( lcks );
    }
    return 0;
}
