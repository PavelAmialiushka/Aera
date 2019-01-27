#include "stdafx.h"

#include "StagesWindow.h"
#include "NagDlg.h"

#include "utilites/Localizator.h"
#include "utilites/serl/Archive.h"
#include "utilites/serl/serl_registry.h"

#include "utilites/document.h"

#include "data/nodeFactory.h"

#include "StagesDlg.h"

#include "toolbarbtns.h"
#include "resource.h"

#include <atlwin.h>

CStagesWnd::CStagesWnd()
{
}

void CStagesWnd::serialization(serl::archiver &arc)
{
    arc.serial("docking", serial_dock_pos_.bDocking);
    arc.serial("side",  serial_dock_pos_.dockPos.dwDockSide);

    arc.serial("left",  serial_dock_pos_.rect.left);
    arc.serial("right", serial_dock_pos_.rect.right);
    arc.serial("top",   serial_dock_pos_.rect.top);
    arc.serial("bottom",serial_dock_pos_.rect.bottom);

    arc.serial("nBar", serial_dock_pos_.dockPos.nBar);
    arc.serial("fPctPos", serial_dock_pos_.dockPos.fPctPos);
    arc.serial("nWidth", serial_dock_pos_.dockPos.nWidth);
    arc.serial("nHeight", serial_dock_pos_.dockPos.nHeight);
}

HWND CStagesWnd::Create(HWND hparent)
{
    CWindow parent(hparent);
    CRect prc; parent.GetWindowRect(prc);
    parent.ClientToScreen(prc);

    return baseClass::Create(
               parent, CRect(prc.TopLeft(), CSize(200, 200)), "",
               WS_VISIBLE|WS_CAPTION|WS_POPUP|WS_OVERLAPPED|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_SYSMENU
               |WS_SYSMENU|WS_THICKFRAME);
}

LRESULT CStagesWnd::OnCreate(LPCREATESTRUCT)
{
    listctrl_.Create(m_hWnd, rcDefault, "",
                     WS_VISIBLE|WS_CHILD|WS_BORDER
                     |LVS_REPORT|LVS_SINGLESEL|LVS_NOCOLUMNHEADER);
    listctrl_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

    listctrl_.InsertColumn(0, "Stage", 0, 200, 0);

//  toolbar_.Create(m_hWnd, rcDefault, "",
//		WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT,//|CCS_NORESIZE|CCS_NOPARENTALIGN,
//		TBSTYLE_EX_DRAWDDARROWS
//		);
//  toolbar_.SetButtonStructSize();
//
//  CToolbarButtons main=IDR_STAGE_WND;
//  main.Append(ID_STAGE_PROPERTY, 0);
//  main.Apply(toolbar_);

    BuildList();
    OnChangeLanguage();

    stages::instance()->connect( (int)this, bind(&CStagesWnd::UpdateList, this));

    return 0;
}

void CStagesWnd::OnChangeLanguage()
{
    SetWindowText(_lcs("Stages"));
}

LRESULT CStagesWnd::OnDestroy()
{
    stages::instance()->disconnect((int)this);
    return 0;
}


void CStagesWnd::UpdateList()
{
    listctrl_.DeleteAllItems();
    BuildList();
}

void CStagesWnd::BuildList()
{
    listctrl_.InsertItem(0, _lcs("Entire test"));

    bool selected=false;
    stage cstage=nodes::factory().get_sel_stage();

    unsigned count=stages::instance()->size();
    for (unsigned index=0; index<count; ++index)
    {
        shared_ptr<stage> stg=stages::instance()->at(index);
        listctrl_.InsertItem(index+1, stg->name.c_str());
        if (*stg==cstage)
        {
            selected=true;
            listctrl_.SelectItem(index+1);
        }
    }
    if (!selected) listctrl_.SelectItem(0);

}

LRESULT CStagesWnd::OnEraseBackground(HDC hdc)
{
    CDCHandle dc=hdc;

    CRect rc; GetClientRect(rc);
    dc.FillRect(rc, GetSysColorBrush(COLOR_3DFACE));

    return 0;
}

void CStagesWnd::SavePositionToRegistry()
{
    if (registry_loaded_)
    {
        GetDockingWindowPlacement(&serial_dock_pos_);

        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Layouts"))
        .serial("StagesBar", *this);
    }
}

void CStagesWnd::LoadPositionFromRegistry()
{
    GetDockingWindowPlacement(&serial_dock_pos_);

    serl::load_archiver(
        new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Layouts"))
    .serial("StagesBar", *this);

    SetDockingWindowPlacement(&serial_dock_pos_);
    GetDockingWindowPlacement(&serial_dock_pos_);
    registry_loaded_=true;
}

LRESULT CStagesWnd::OnSizeChanged(UINT uMsg, WPARAM, LPARAM)
{
    CRect rc; GetClientRect(rc);
    rc.DeflateRect(5, 5);

    if (toolbar_)
    {
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

LRESULT CStagesWnd::OnItemChanged(LPNMHDR hdr)
{
    LPNMLISTVIEW nm=(LPNMLISTVIEW)hdr;
    if (nm->uNewState & LVIS_SELECTED)
    {
        int index=nm->iItem;
        stage stg=index==0
                  ? stage()
                  : *stages::instance()->at(index-1);
        nodes::factory().set_sel_stage(stg);
    }
    return 0;
}

LRESULT CStagesWnd::OnItemDblClick(LPNMHDR)
{
    OnStageProperty(0, 0, 0);
    return 0;
}

LRESULT CStagesWnd::OnStageProperty(UINT, int, HWND)
{
    Document().Nag();

    CStagesDlg dlg;
    dlg.DoModal();
    return 0;
}
