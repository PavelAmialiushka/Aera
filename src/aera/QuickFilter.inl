#include "resource.h"
#include "QuickFilter.h"

#include "data/node.h"
#include "data/nodeFactory.h"
#include "data/Syntaxer.h"

#include "utilites/help.h"

#include "wtladdons.h"

#include "utilites/foreach.hpp"
#include "utilites/tokenizer.h"
#include "utilites/Localizator.h"

#include "utilites/serl/serl_registry.h"

void CQuickFilter::SetRTFEdit()
{
    text_.SubclassWindow(GetDlgItem(IDC_QF_TEXT));

    CHARFORMAT cf={sizeof(CHARFORMAT)};
    cf.dwMask=CFM_FACE;
    strcpy(cf.szFaceName, "Courier New");
    text_.SetDefaultCharFormat(cf);
    text_.SetCaseSensitive(false);
    text_.SetDelayedFormat(false);
    text_.SetChangeCase(false);

    SYNTAXCOLOR sc0={0x000000, 0, 0};
    text_.SetSyntaxColor(SC_NORMAL, sc0);

    SYNTAXCOLOR sc1={0xFF0000, 1, 0};
    text_.SetSyntaxColor(SC_NUMBER, sc1);

    SYNTAXCOLOR sc2={0x00007F, 1, 0};
    text_.SetSyntaxColor(SC_KEYWORD, sc2);
}

static std::string simplize(std::string text)
{
    std::string result;

    std::string filter=tokenizer::head(text, "#");
    result += tokenizer::head(filter, "\n");
    if (!filter.empty())
        result += " ...";

    result = strlib::replace( result, "&&", "&&&&");

    if (tokenizer::head(text, ",")=="1")
    {
        result += " (" + text + ")";
    }

    return result;
}


void CQuickFilter::OnTimer(UINT id, TIMERPROC)
{
    KillTimer(id);
    FillInVariables();
}

void  CQuickFilter::FillInVariables()
{
    nodes::node *node=nodes::factory().get_active_node().get();

    if (!node->is_ready())
    {
        SetTimer(1, 100);
        return;
    }


    nodes::presult r = process::get_result<nodes::result>(node);
    std::vector<aera::chars> chars=r->ae->get_chars();

    CMenuHandle parent=GetMenu();
    CMenuHandle menu=parent.GetSubMenu(0);
    while (menu.GetMenuItemCount())
        menu.DeleteMenu(0, MF_BYPOSITION);

    if (chars.empty())
    {
        menu.InsertMenu(0, MFT_STRING|MFS_GRAYED, (UINT_PTR)0, _lcs("MRU-none#none"));
    }
    else for(unsigned index=0; index<chars.size(); ++index)
    {
        text_.AddKeyword(aera::traits::get_short_name(chars[index]).c_str());
        std::string tmp=aera::traits::get_wide_name(chars[index]);
        if (tmp.empty()) continue;
        if (tmp.find(" ")==std::string::npos) text_.AddKeyword(tmp.c_str());

        tmp=aera::traits::get_short_name(chars[index])+"\t"+tmp;
        menu.InsertMenu(index, MFT_STRING, IDC_QF_MRUV_FIRST+index, tmp.c_str());
    }

    text_.FormatAll();
}

LRESULT CQuickFilter::OnMRUV(UINT, int id, HWND)
{
    CMenuHandle parent=GetMenu();
    CMenuHandle menu=parent.GetSubMenu(0);
    for(int index=0;index < menu.GetMenuItemCount(); ++index)
    {
        if (menu.GetMenuItemID(index)==id)
        {
            std::vector<char> buffer( menu.GetMenuStringLen(id, MF_BYCOMMAND)+1);
            menu.GetMenuString(id, &buffer[0], buffer.size(), MF_BYCOMMAND);
            std::string text = tokenizer::head( std::string(&buffer[0]), "\t" );
            text_.ReplaceSel( text.c_str(), true);
            text_.SetFocus();
        }
    }

    return 0;
}



LRESULT CQuickFilter::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    _lw(m_hWnd, IDD_QUICK_FILTER);
    _lm(GetMenu(), IDR_QF_MENU);

    // подстраивает размер меню
    SetMenu( GetMenu() );

    CenterWindow();

    CheckRadioButton(IDC_QF_SEL1, IDC_QF_SEL3, IDC_QF_SEL1);

    before_.SubclassWindow( GetDlgItem(IDC_QF_NEDIT1) );
    after_.SubclassWindow( GetDlgItem(IDC_QF_NEDIT2) );

    MakeWindowText((HWND)before_)="0.0";
    MakeWindowText((HWND)after_)="0.0";

    SendMessage(WM_COMMAND, IDC_QF_TIME, 0);

    SetRTFEdit();

    serl::load_archiver(
                new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data\\Filter")).
            serial_container("mru", serl_array_);

    CMenuHandle parent( GetMenu() );
    CMenuHandle history=parent.GetSubMenu(1);

    // удаляем содержимое
    while (history.GetMenuItemCount())
        history.RemoveMenu(0, MF_BYPOSITION);

    // вносим новый список
    if (serl_array_.empty())
    {
        history.InsertMenu(0, MFT_STRING|MFS_GRAYED, (UINT_PTR)0, _lcs("MRU-none#none"));
    }
    else for(unsigned index=0; index<serl_array_.size(); ++index)
    {
        history.InsertMenu(index, MFT_STRING, IDC_QF_MRU_FIRST+index, simplize(serl_array_[index]).c_str());
    }

    FillInVariables();

    return 0;
}

LRESULT CQuickFilter::OnMRU(UINT, int id, HWND)
{
    unsigned index=id-IDC_QF_MRU_FIRST;

    if (index < serl_array_.size())
    {
        std::string text=serl_array_.at(index);
        Restore(text);
    }

    return 0;
}

template<class C> std::string join(C const& c, std::string cat )
{
    std::string result;
    typename C::const_iterator iter=c.begin();
    while(iter!=c.end())
    {
        if (!result.empty()) result+=cat;
        result += *iter;
        ++iter;
    }

    return result;
}

LRESULT CQuickFilter::OnIdClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (wID!=IDCANCEL && !MakeWindowText(text_).string().empty())
    {
        std::string text=MakeWindowText(text_);

        text = "(" + join(tokenizer(text, "\n"), ")||(") + ")";

        int mode = selection_add;
        if (IsDlgButtonChecked(IDC_QF_SEL1)) mode = selection_set;
        if (IsDlgButtonChecked(IDC_QF_SEL3)) mode = selection_sub;

        double timebefore = 0, timeafter = 0;
        try
        {
            if (IsDlgButtonChecked(IDC_QF_TIME))
            {
                timebefore = boost::lexical_cast<double>(MakeWindowText(before_));
                timeafter =  boost::lexical_cast<double>(MakeWindowText(after_));
            }
        } catch (boost::bad_lexical_cast&) { /* ignore */ }

        select_action prm(mode, text);
        prm.set_time_ranges(timebefore, timeafter);
        nodes::factory().modify_selection(prm);

        std::string new_item=Store();
        serl_array_.erase(
                    std::remove(
                        STL_II(serl_array_),
                        new_item),
                    serl_array_.end());

        serl_array_.insert( serl_array_.begin(), new_item );
        if (serl_array_.size() > 16)
            serl_array_.pop_back();

        serl::save_archiver(
                    new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data\\Filter")).
                serial_container("mru", serl_array_);
    }

    EndDialog(IDOK);

    return 0;
}

void CQuickFilter::ExportText()
{
    text_.FormatAll();

    std::string text=MakeWindowText(text_);
    tokenizer tkn(text, "\n");
    try {

        std::string string;
        foreach(std::string line, tkn)
        {
            if (line.size())
            {
                filtering::node nd=filtering::tree_maker("("+line+")");
                if (nd.get_lexeme()!="()") line="("+line+")";
                string = tokenizer::join( string, "&&", line);
            }
        }

    } catch(filtering::syntax_error&)
    {
        unsigned index=0;
        foreach(std::string line, tkn)
        {
            try {
                filtering::tree_maker( line );
            } catch(filtering::syntax_error&)
            {
                LONG prevb, preve;
                text_.GetSel(prevb, preve);

                int beg=text_.LineIndex(index);
                int end=text_.LineIndex(index+1);
                text_.SetSel(beg, end);

                CHARFORMAT cf={sizeof(CHARFORMAT), CFM_COLOR};
                cf.crTextColor=RGB(255,0,0);
                text_.SetSelectionCharFormat(cf);

                text_.SetSel(prevb, preve);
            }
            ++index;
        }
    }
}

LRESULT CQuickFilter::OnTextUpdate(UINT, int, HWND)
{
    ExportText();
    return 0;
}

LRESULT CQuickFilter::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "filter");
    return 0;
}

LRESULT CQuickFilter::OnTimeChanged(UINT, int, HWND)
{
    ::EnableWindow( GetDlgItem(IDC_QF_NEDIT1), IsDlgButtonChecked(IDC_QF_TIME) );
    ::EnableWindow( GetDlgItem(IDC_QF_NEDIT2), IsDlgButtonChecked(IDC_QF_TIME) );
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

std::string CQuickFilter::Store()
{
    std::string result=
            MakeWindowText( text_ ).string();

    if (!strlib::endswith(result,"\n"))
        result += "\n";

    result += "#";

    result +=
            IsDlgButtonChecked( IDC_QF_TIME )
            ? "1,"
            : "0,";
    result += MakeWindowText( before_ );
    result += ",";
    result += MakeWindowText( after_ );

    return result;
}

void CQuickFilter::Restore(std::string text)
{
    MakeWindowText( text_ )=
            tokenizer::head(text, "#");

    std::string use=tokenizer::head(text, ",");
    CheckDlgButton( IDC_QF_TIME, use=="1" );

    text = tokenizer::head(text, "\n");
    MakeWindowText( before_ )= tokenizer::head(text, ",");
    MakeWindowText( after_ )= tokenizer::head(text, ",");

    SendMessage(WM_COMMAND, IDC_QF_TIME, 0);
}

//////////////////////////////////////////////////////////////////////////

static std::string get_filter_dir()
{
    return utils::get_app_path("FilterTemplates");
}

LRESULT CQuickFilter::OnSave(UINT, int, HWND)
{
    std::string filter_dir=get_filter_dir();

    CFileDialog dlg(
                false, ".afl", (filter_dir+"\\noname").c_str(),
                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR,
                "Aera filter files\0*.afl\0"
                );

    if (dlg.DoModal()==IDOK)
    {
        std::ofstream( dlg.m_szFileName ) << Store() << std::endl;
    }
    return 0;
}

LRESULT CQuickFilter::OnLoad(UINT, int, HWND)
{
    std::string filter_dir=get_filter_dir();

    CFileDialog dlg(
                true, ".afl", (filter_dir+"\\noname").c_str(),
                OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
                "Aera filter files\0*.afl\0"
                );

    if (dlg.DoModal()==IDOK)
    {
        std::ifstream source( dlg.m_szFileName );
        std::vector<char> buffer;

        std::copy(
                    std::istreambuf_iterator<char>(source),
                    std::istreambuf_iterator<char>(),
                    std::back_inserter(buffer));

        Restore(std::string(&buffer[0], buffer.size()));
    }
    return 0;
}

//LRESULT CQuickFilter::OnInsert(UINT, int id, HWND)
//{
//  if (id==IDC_QF_INSERT_VAR)
//    text_.ReplaceSel( var_.c_str(), true );
//  else if (id==IDC_QF_INSERT_OPER)
//    text_.ReplaceSel( oper_.c_str(), true );
//  else if (id==IDC_QF_INSERT_AND)
//    text_.ReplaceSel( " && ", true);
//  else
//    text_.ReplaceSel( " || ", true);
//  text_.SetFocus();
//  return 0;
//}
