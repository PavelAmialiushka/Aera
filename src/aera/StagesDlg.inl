
#include "Stagesdlg.h"
#include "wtladdons.h"

#include "utilites/Localizator.h"

#include "data/stage.h"
#include "data/nodeFactory.h"
#include "data/node.h"

#include "controls/PlotWindow.h"
#include "utilites/help.h"

#define _blc boost::lexical_cast
#define _blcz(a) boost::lexical_cast<std::string>(a).c_str()

class CEdit2 : public CWindowImpl<CEdit2>
{
public:
    BEGIN_MSG_MAP_EX(CEdit);
    MSG_WM_GETDLGCODE(OnGetDlgCode);
    END_MSG_MAP();

    LRESULT OnGetDlgCode(LPMSG)
    {
        return DLGC_WANTALLKEYS;
    }
} edit2;


double CStagesDlg::GetMaxTime()
{
    if (!result_) return 0;

    data::pslice slice=result_->raw;
    if (!slice || !slice->size()) return 0;

    return slice->get_value(slice->size()-1);
}

LRESULT CStagesDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    _lw(m_hWnd, IDD_STAGES);
    CenterWindow(GetParent());

    edit_label_=false;
    updating_=false;

    ATL::CWindow pos( GetDlgItem(IDC_STAGE_PLOTPOS) ) ;
    CRect rect; pos.GetWindowRect(rect);
    ScreenToClient(rect);

    view_=new CPlotView(true);
    view_->Create( m_hWnd, rect, "", WS_CHILD|WS_VISIBLE, 0, IDC_STAGE_PLOTPOS2);
    DlgResize_Init(true, true, 0);

    axe_info infox={aera::C_Time, true, 0, 1, false};
    view_->set_xaxe_info(infox);

    axe_info infoy={aera::C_Parametric1, true, 0, 1, false};
    view_->set_yaxe_info(infoy);

    plot_info infop={scatter, false, true, 200};
    view_->set_plot_params(infop);

    tree_.Attach( GetDlgItem(IDC_STAGE_TREE) );
    stage_name_.Attach( GetDlgItem(IDC_STAGE_NAME2) );
    start_time_.Attach( GetDlgItem(IDC_STAGE_START) );
    end_time_.Attach( GetDlgItem(IDC_STAGE_END) );

    current_stage_=nodes::factory().get_sel_stage();
    if (current_stage_!=stage())
        nodes::factory().set_sel_stage(stage());

    TryToGetResult();
    CreateTree();

    view_->set_visibility(1);

    return TRUE;
}

LRESULT CStagesDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (current_stage_!=nodes::factory().get_sel_stage())
    {
        nodes::factory().set_sel_stage(current_stage_);
    }

    // вызывает обновление окна стадий
    stages::instance()->changed();

    EndDialog(wID);
    return 0;
}

void CStagesDlg::CreateTree()
{
    root_item_=current_item_=tree_.InsertItem(_lcs("Entire test"), TVI_ROOT, TVI_LAST);
    current_item_.SetState(TVIS_EXPANDED, TVIS_EXPANDED);
    BuildTreeFromStage(root_item_, *stages::instance());
    current_item_.Select();
}

// удаляет и заново отстраивает под-элемента 
// rootitem дерево и, соответственно, stg
void CStagesDlg::BuildTreeFromStage(CTreeItem rootitem, const stage& stg)
{
    while(CTreeItem itm=rootitem.GetChild()) itm.Delete();

    for(unsigned index=0; index < stg.size(); ++index)
    {
        stage& child=*stg.at(index);
        CTreeItem item=rootitem.AddTail( child.name.c_str(), 0);
        item.SetState(TVIS_EXPANDED, TVIS_EXPANDED);

        BuildTreeFromStage(item, child);
    }
}

// 
//
void CStagesDlg::BuildTreeFromGraph()
{
    updating_=true;

    std::vector<shared_ptr<stage> > tmp;
    unsigned cindex=view_->GetSelectionRange(tmp);

    CTreeItem parent_item =
            current_item_==root_item_ ? root_item_ : current_item_.GetParent();
    GetStage(parent_item).children_=tmp;

    BuildTreeFromStage(parent_item, GetStage(parent_item));
    current_item_=parent_item.GetChild();

    while(cindex-- && current_item_)
        current_item_=current_item_.GetNextSibling();

    if (!current_item_) current_item_=parent_item.GetChild();
    current_item_.Select();

    updating_=false;
}

static unsigned GetTreeItemIndex(CTreeItem item)
{
    unsigned index=0;
    for(;item=item.GetPrevSibling(), item!=0;++index) ;
    return index;
}

stage& CStagesDlg::GetRealStage(CTreeItem item)
{
    // какую часть показывать
    if (item==0) item=current_item_;
    return (GetLevel()<2)
            ? GetStage(item)
            : *GetStage(item.GetParent()).at( GetTreeItemIndex(item) );
}



stage& CStagesDlg::GetStage(CTreeItem item)
{
    // отдает
    if (item==0) item=current_item_;
    switch(GetLevel(item))
    {
    // root
    case 0: return *stages::instance();

        // root child - 'stage'
    case 1: return *stages::instance()->at( GetTreeItemIndex(item) );

        // 'hold'
    default:
        return GetStage(item.GetParent());
    }
}

LRESULT CStagesDlg::OnAppendStage(UINT, int, HWND)
{
    stage& current_stage=GetStage(current_item_);
    if (GetLevel(current_item_)==2)
    {

    } else {

        shared_ptr<stage> spring=shared_ptr<stage>( new
                                                    stage( _lcs("stages-new#new stage"), 0, 0 ) );

        current_stage.insert(-1, spring);

        BuildTreeFromStage( current_item_, current_stage );

        GetTreeItemByStage(*spring).Select();
    }

    return 0;
}

struct cmp_stages : public std::binary_function<stage, stage, bool>
{
    bool operator()(const stage& a, const stage& b) const
    {
        return
                a.start != b.start
                ? a.start < b.start
                : a.end < b.end;
    }
};

typedef std::map<stage, CTreeItem, cmp_stages> stage_map_t;

static void collect_info(stage_map_t& map, const stage& stg, CTreeItem item)
{
    map[stg]=item;
    CTreeItem child=item.GetChild();
    for(unsigned index=0; child; ++index, child=child.GetNextSibling())
    {
        collect_info(map, *stg.children_.at(index), child);
    }
}

CTreeItem CStagesDlg::GetTreeItemByStage(const stage& pattern)
{
    stage_map_t map;
    collect_info(map, *stages::instance(), root_item_);

    return map[pattern];
}

LRESULT CStagesDlg::OnDeleteStage(UINT, int, HWND)
{
    if (current_item_!=root_item_)
    {
        unsigned cindex=GetTreeItemIndex(current_item_);

        CTreeItem parent_item=current_item_.GetParent();
        assert(parent_item);

        stage& parent_stg=GetStage(parent_item);

        assert(cindex < parent_stg.size());
        parent_stg.remove(cindex);

        root_item_.Select();

        BuildTreeFromStage(parent_item, parent_stg);
        if (CTreeItem child=parent_item.GetChild())
        {
            child.Select();
        } else {
            parent_item.Select();
        }

    }
    return 0;
}

void CStagesDlg::TryToGetResult()
{
    nodes::node *node = nodes::factory().get_active_node().get();

    if (!node->is_ready())
    {
        SetTimer(0, 100);
        return;
    }

    result_ = process::get_result<nodes::result>(node);
}

void CStagesDlg::OnTimer(UINT id, TIMERPROC)
{
    KillTimer(id);
    TryToGetResult();
}


LRESULT CStagesDlg::OnSelChanged(LPNMHDR hdr)
{
    if (updating_) return 0;
    LPNMTREEVIEW nm=(LPNMTREEVIEW)hdr;

    // изменяем содержание графика в зависимости от
    // выбранного этапа испытания
    current_item_=CTreeItem(nm->itemNew.hItem, &tree_);

    double maxtime=GetMaxTime();

    // родитель - этап, определяющий масштаб графика
    CTreeItem parent_item = GetLevel() > 0 ? current_item_.GetParent() : current_item_;

    // родительский этап
    stage& parent_stage = GetStage( parent_item );

    // собственно этап, выделяется на графике
    CTreeItem leaf = current_item_;

    bool autov=  parent_item ==root_item_ ? true : false;

    axe_info info=view_->get_xaxe_info();

    if (autov!=info.auto_scale || parent_stage.start!=info.lo || parent_stage.end!=info.hi)
    {
        info.auto_scale=autov;
        info.lo=parent_stage.start;
        info.hi=parent_stage.end;
        view_->set_xaxe_info(info);
    }

    // передаем в график братьев текущего объекта
    view_->SetSelectionRange( parent_stage.children_, GetTreeItemIndex(leaf) );
    SyncEditCtrls();
    return 0;
}

LRESULT CStagesDlg::OnUpdateRange(UINT, WPARAM, LPARAM)
{
    // вызывается при изменении положений
    // или количества выдержек на графике
    // НЕ  вызвается при измении текста в
    // editctrl

    SyncEditCtrls();
    BuildTreeFromGraph();

    return 0;
}

LRESULT CStagesDlg::OnUpdateRange2(UINT, WPARAM, LPARAM)
{
    // вызывается при dblclk
    return SendMessage(WM_COMMAND, ID_APPEND_STAGE, 0);
}


void CStagesDlg::SyncEditCtrls()
{
    updating_=true;

    std::vector<shared_ptr<stage> > tmp;
    unsigned cindex=view_->GetSelectionRange(tmp);

    bool isNormal=cindex < tmp.size() && current_item_!=root_item_;
    stage_name_.EnableWindow(isNormal ? true : false);
    start_time_.EnableWindow(isNormal ? true : false);
    end_time_  .EnableWindow(isNormal ? true : false);

    if (isNormal)
    {
        stage& stg= *tmp[cindex];
        MakeWindowText(stage_name_)=_blc<std::string>(stg.name);
        MakeWindowText(start_time_)=strlib::strf("%.6f", stg.start);
        MakeWindowText(end_time_)=strlib::strf("%.6f", stg.end);
    } else {
        MakeWindowText(stage_name_)=_lcs("Entire test");
        MakeWindowText(start_time_)="0";
        MakeWindowText(end_time_)="0";
    }


    updating_=false;
}

LRESULT CStagesDlg::OnBeginLabelEdit(LPNMHDR)
{
    if (GetLevel()==0) return 1;
    edit_label_=true;
    edit2.SubclassWindow(tree_.GetEditControl());
    return 0;
}

LRESULT CStagesDlg::OnEndLabelEdit(LPNMHDR)
{  
    std::string text=MakeWindowText(edit2);
    edit2.UnsubclassWindow();

    current_item_.SetText(text.c_str());
    GetRealStage().name=text;

    SyncEditCtrls();

    edit_label_=false;
    return 0;
}

LRESULT CStagesDlg::OnKeyDown(LPNMHDR hdr)
{
    LPNMTVKEYDOWN nm=reinterpret_cast<LPNMTVKEYDOWN>(hdr);
    switch(nm->wVKey) {
    case VK_F2:
        current_item_.EditLabel();
        break;
    case VK_INSERT:
        PostMessage(WM_COMMAND, (WPARAM)ID_APPEND_STAGE, (LPARAM)(HWND)tree_);
        break;
    case VK_DELETE:
        PostMessage(WM_COMMAND, (WPARAM)ID_DELETE_STAGE, (LPARAM)(HWND)tree_);
        break;
    case VK_APPS:
        OnTreeRClick(0);
        break;
    }
    return 0;
}

int CStagesDlg::GetLevel(CTreeItem item)
{
    if (item==0) item=current_item_;
    int result=0;

    while(item!=root_item_ && item) {
        item=item.GetParent();
        ++result;
    }
    return result;
}

LRESULT CStagesDlg::OnEditChange(UINT, int, HWND)
{
    if (updating_) return 0;
    try {
        std::vector<shared_ptr<stage> > tmp;
        unsigned cindex=view_->GetSelectionRange(tmp);
        if (cindex >= tmp.size()) return 0;
        stage& hld=*tmp[cindex];

        double a=_blc<double>(MakeWindowText(start_time_));
        double b=  _blc<double>(MakeWindowText(end_time_));
        std::string t=MakeWindowText(stage_name_);
        hld.start=a; hld.end=b; hld.name=t;

        BuildTreeFromGraph();
        view_->Invalidate();
    } catch (boost::bad_lexical_cast) {
    }
    return 0;
}

LRESULT CStagesDlg::OnTreeRClick(LPNMHDR hdr)
{ 
    CPoint pt=CPoint(::GetMessagePos());
    ::ScreenToClient(tree_, &pt);
    CTreeItem item=tree_.HitTest(pt, 0);
    if (!item) return 0;
    item.Select();

    CRect rc;
    item.GetRect(rc, false);
    ::ClientToScreen(tree_, &rc.BottomRight());
    ::ClientToScreen(tree_, &rc.TopLeft());

    CMenu menu( ::CreatePopupMenu() );
    menu.AppendMenu( MF_STRING
                     | (GetLevel(item)==2 ? MF_GRAYED : MF_ENABLED),
                     ID_APPEND_STAGE, _lcs("stages-append#Insert &stage"));

    menu.AppendMenu( MF_STRING
                     | (GetLevel(item)==0 ? MF_GRAYED : MF_ENABLED),
                     ID_RENAME_STAGE, _lcs("stages-rename#&Rename stage"));

    menu.AppendMenu( MF_SEPARATOR );
    menu.AppendMenu( MF_STRING
                     | (GetLevel(item)==0 ? MF_GRAYED : MF_ENABLED),
                     ID_DELETE_STAGE, _lcs("stages-delete#&Delete stage"));

    menu.TrackPopupMenu(TPM_TOPALIGN|TPM_LEFTALIGN, rc.left, rc.bottom, m_hWnd);
    return 0;
}

LRESULT CStagesDlg::OnTreeDblClk(LPNMHDR hdr)
{
    CPoint pt=CPoint(::GetMessagePos());
    ::ScreenToClient(tree_, &pt);
    CTreeItem item=tree_.HitTest(pt, 0);
    if (!item) return 0;
    item.Select();

    SendMessage(WM_COMMAND, ID_APPEND_STAGE, 0);
    return 0;
}

LRESULT CStagesDlg::OnTreeExpanding(LPNMHDR lParam)
{
    LPNMTREEVIEW nm =(LPNMTREEVIEW) lParam;
    return (nm->action&TVE_COLLAPSE) ? 1 : 0;
}

LRESULT CStagesDlg::OnRenameStage(UINT, int, HWND)
{
    current_item_.EditLabel();
    return 0;
}

LRESULT CStagesDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "stages");
    return 0;
}


