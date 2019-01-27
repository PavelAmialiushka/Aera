#include "StdAfx.h"

#include "controls/ListProperties.h"
#include "data/collection.h"

#include "utilites/localizator.h"
#include "utilites/help.h"

#include "data/nodefactory.h"

#include "utilites/document.h"

ListPropertyDlg::ListPropertyDlg(list_model *model, HWND parent)
    : model_(model)
    , delayed_(false)
    , wndParent(parent)
{
}

LRESULT ListPropertyDlg::OnInitDialog(HWND hWnd, LPARAM ppage)
{
    _lw(m_hWnd, IDD_LISTPPCOMMON);

    CenterWindow();

    m_current=GetDlgItem(IDC_LIST_CURRENT);
    m_current.MakeDragList();
    m_available=GetDlgItem(IDC_LIST_AVIABLE);

    use_location_.Attach( GetDlgItem(IDC_LP_USE_LOCATION) );
    location_type_.Attach( GetDlgItem(IDC_LP_LOCATION) );

    MakeFeatureList();

    bool mode=model_->get_rel_mode();
    CheckDlgButton(IDC_LP_REL_TIME, mode ? BST_CHECKED : BST_UNCHECKED);

    int rel=model_->get_peak_mode();
    CheckRadioButton(IDC_LP_TIME1, IDC_LP_TIME2,
                     rel==0 ? IDC_LP_TIME1 : IDC_LP_TIME2);

    bool nav=model_->get_nav_mode();

    // set location types
    std::vector< location::Setup > tmp=
        nodes::factory().get_location_setup();

    int current_index=-1;
    object_id   cur_location = model_->get_location();
    for (unsigned index=0; index<tmp.size(); ++index)
    {
        location::Setup loc=tmp[index];
        location_type_.AddString( loc.name().c_str() );
        if (loc.get_id()==cur_location)
            current_index=index;
    }

    if (current_index!=-1)
    {
        location_type_.SetCurSel( current_index );
    }
    else if (!cur_location.empty())
    {
        location_type_.AddString(_lcs("LP-noloc#location not found"));
        location_type_.SetCurSel( tmp.size() );
    }
    else if (tmp.size() == 0)
    {
        location_type_.AddString(_lcs("LP-noloc-empty#empty"));
        location_type_.SetCurSel( tmp.size() );
    } else
    {
        location_type_.SetCurSel( 0 );
    }

    use_location_.SetCheck( model_->get_location()!=object_id() );

    CheckDlgButton(IDC_LP_NAV_MODE, nav ? BST_CHECKED : BST_UNCHECKED);

    OnCheckButton();

    return TRUE;
}

void ListPropertyDlg::OnLocationChanged(...)
{
    RealApply();
    MakeFeatureList();
}


void ListPropertyDlg::MakeFeatureListDelayed()
{
    std::vector<aera::chars> feats;
    if (model_->valid())
        feats = model_->get_avl_chars();
    else
    {
        feats.push_back(aera::C_Time);
        feats.push_back(aera::C_Channel);
        feats.push_back(aera::C_Amplitude);
        feats.push_back(aera::C_Energy);
        feats.push_back(aera::C_Duration);
        feats.push_back(aera::C_Risetime);
        feats.push_back(aera::C_ASL);
        feats.push_back(aera::C_Hits);
        feats.push_back(aera::C_CoordX);
        feats.push_back(aera::C_CoordY);
    }

    std::vector<aera::chars> current=model_->get_features();

    while (m_current.GetCount()) m_current.DeleteString(0);
    for (unsigned index=0; index<current.size(); ++index)
    {
        std::string name=aera::traits::get_wide_name(current[index]);
        feats.erase(std::remove(STL_II(feats), current[index]), feats.end());
        m_current.AddString(name.c_str());
    }

    while (m_available.GetCount()) m_available.DeleteString(0);
    for (unsigned index=0; index<feats.size(); ++index)
    {
        std::string name=aera::traits::get_wide_name(feats[index]);
        if (!name.empty())
            m_available.AddString(name.c_str());
    }

    EnableControls(true);
}

void ListPropertyDlg::EnableControls(bool en)
{
    m_available.EnableWindow(en);
    m_current.EnableWindow(en);
    ::EnableWindow( GetDlgItem(IDC_LEFT), en );
    ::EnableWindow( GetDlgItem(IDC_LEFTALL), en );
    ::EnableWindow( GetDlgItem(IDC_RIGHT), en );
    ::EnableWindow( GetDlgItem(IDC_RIGHTALL), en );
    ::EnableWindow( GetDlgItem(IDC_UP), en );
    ::EnableWindow( GetDlgItem(IDC_DOWN), en );
}

void ListPropertyDlg::MakeFeatureList()
{    
    if (!model_->valid())
    {
        if (!delayed_)
        {
            SetTimer(2, 500);
            MakeFeatureListDelayed();
        }
        delayed_=true;
    }
    else
    {
        delayed_=false;
        MakeFeatureListDelayed();
    }
}

void ListPropertyDlg::_MoveLeft(int rsel)
{
    if (rsel==-1)
    {
        rsel=0;
    }
    if (rsel>=m_available.GetCount()) return;

    std::vector<char> buffer(m_available.GetTextLen(rsel)+1);
    m_available.GetText(rsel, &buffer[0]);

    std::string rstr=&buffer[0];
    m_available.DeleteString(rsel);
    m_current.AddString(rstr.c_str());
    m_available.SetCurSel(std::min(rsel, m_available.GetCount()-1));

    m_current.SetCurSel(m_current.FindString(0, rstr.c_str()));

    Apply();
}

LRESULT ListPropertyDlg::OnLeft(UINT code, int id, HWND wnd)
{
    try
    {
        _MoveLeft(m_available.GetCurSel());

    } StopAndLog;
    return 0;
}

LRESULT ListPropertyDlg::OnLeftAll(UINT code, int id, HWND wnd)
{
    try
    {
        while (m_available.GetCount())
        {
            _MoveLeft(m_available.GetCurSel());
        }

    } StopAndLog;
    return 0;
}

void ListPropertyDlg::_MoveRight(int lsel)
{
    if (lsel==0) return;
    CString lstr;
    m_current.GetText(lsel, lstr);
    m_current.DeleteString(lsel);
    m_available.AddString(lstr);
    m_current.SetCurSel(std::min(lsel, m_current.GetCount()-1));

    m_available.SetCurSel(m_available.FindString(0, lstr));
    Apply();
}

LRESULT ListPropertyDlg::OnRight(UINT code, int id, HWND wnd)
{
    try
    {
        int lsel=m_current.GetCurSel();
        if (lsel==LB_ERR) return TRUE;
        _MoveRight(lsel);

    } StopAndLog;
    return 0;
}

LRESULT ListPropertyDlg::OnRightAll(UINT code, int id, HWND wnd)
{
    try
    {

        while (m_current.GetCount()>1) _MoveRight(1);

    } StopAndLog;
    return 0;
}

LRESULT ListPropertyDlg::OnDragListNotify(UINT code, int id, HWND wnd)
{
    return 0;
}

LRESULT ListPropertyDlg::OnUp(UINT code, int id, HWND wnd)
{
    try
    {

        int sel=m_current.GetCurSel();
        if (sel>0+1)
        {
            CString str;
            m_current.GetText(sel, str);
            m_current.DeleteString(sel);
            m_current.InsertString(sel-1, str);
            m_current.SetCurSel(sel-1);
        }
        Apply();

    } StopAndLog;
    return 0;
}

LRESULT ListPropertyDlg::OnDown(UINT code, int id, HWND wnd)
{
    try
    {
        int sel=m_current.GetCurSel();
        if (sel<m_current.GetCount()-1 && sel!=0)
        {
            CString lstr;
            m_current.GetText(sel, lstr);
            m_current.DeleteString(sel);
            m_current.InsertString(sel+1, lstr);
            m_current.SetCurSel(sel+1);
        }
        Apply();
    } StopAndLog;
    return 0;
}

LRESULT ListPropertyDlg::OnCurrentDblClick(UINT code, int id, HWND wnd)
{
    try
    {
        int sel=m_current.GetCurSel();
        if (sel!=LB_ERR)
        {
            _MoveRight(sel);
        }
    } StopAndLog;
    return 0;
}

LRESULT ListPropertyDlg::OnAvailableDblClick(UINT code, int id, HWND wnd)
{
    try
    {
        int sel=m_available.GetCurSel();
        if (sel!=LB_ERR)
        {
            _MoveLeft(sel);
        }
    } StopAndLog;
    return 0;
}

class TickCounter
{
public:
    TickCounter(DWORD time)
        : next_(GetTickCount()+time)
    {
    }

    operator bool() const
    {
        return GetTickCount() <= next_;
    }
private:
    DWORD next_;
};

void ListPropertyDlg::Apply()
{
    SetTimer(1, 50);
}

void ListPropertyDlg::RealApply()
{
    try
    {
        std::map<std::string, aera::chars> map;
        for (unsigned index=aera::C_Begin; index<aera::C_End; ++index)
        {
            std::string name=aera::traits::get_wide_name(aera::chars(index));
            map[name]=aera::chars(index);
        }

        std::vector<aera::chars> chars;
        for (int index=0; index<m_current.GetCount(); ++index)
        {
            CString name; m_current.GetText(index, name);
            std::string text=name;
            if (map.count(text))
                chars.push_back(map[text]);
        }

        // location
        int lc=use_location_.GetCheck() ? location_type_.GetCurSel() : -1;

        std::vector< location::Setup > tmp=nodes::factory().get_location_setup();
        object_id sample=lc >=0 && (unsigned)lc < tmp.size() ? tmp[lc].get_id() : object_id();

        process::suspend_lock lock(model_);

        if (model_->get_location()!=sample)
        {
            model_->set_location(sample);
        }

        model_->set_nav_mode( IsDlgButtonChecked(IDC_LP_NAV_MODE) );
        model_->set_rel_mode( IsDlgButtonChecked(IDC_LP_REL_TIME) );
        model_->set_peak_mode( IsDlgButtonChecked(IDC_LP_TIME1) ? 0 : 1 );

        model_->set_features(&chars[0], chars.size());

        model_->update_config();
        wndParent.Invalidate();

    } StopAndLog;

    Document().SetModified(true);
}

LRESULT ListPropertyDlg::OnClose(UINT, int id, HWND)
{
    RealApply();
    return EndDialog(id);
}

LRESULT ListPropertyDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "listing");
    return 0;
}

LRESULT ListPropertyDlg::OnTimer(UINT id)
{
    if (id==1)
    {
        RealApply();
        KillTimer(1);
    }
    else
    {
        MakeFeatureList();
        KillTimer(2);
    }
    return 0;
}

LRESULT ListPropertyDlg::OnCheckButton(...)
{
    int mode=IsDlgButtonChecked( IDC_LP_REL_TIME );
    ::EnableWindow( GetDlgItem(IDC_LP_TIME1), mode );
    ::EnableWindow( GetDlgItem(IDC_LP_TIME2), mode );

    location_type_.EnableWindow( use_location_.GetCheck() );

    Apply();
    return 0;
}
