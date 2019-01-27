#include "resource.h"
#include "FilterDlg.h"

#include "controls/PlotWindow.h"

#include "data/node.h"
#include "data/filter.h"
#include "data/nodeFactory.h"

#include "utilites/document.h"

#include "wtladdons.h"

#include "utilites/help.h"

#include "utilites/foreach.hpp"
#include "utilites/tokenizer.h"
#include "utilites/Localizator.h"

#include "utilites/foreach.hpp"
#include <atlmisc.h>

using namespace hits;

static void do_not_delete(void*) {}

LRESULT CFilterDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    _lw(m_hWnd, IDD_FILTER);
    CenterWindow();

    grid_.SubclassWindow( GetDlgItem(IDC_FILTER_GRID) );

    CRect rc; grid_.GetWindowRect(rc);
    int width=rc.Width() - GetSystemMetrics(SM_CXVSCROLL);

    grid_.InsertColumn(0, "#", LVCFMT_LEFT, 20, 0);
    grid_.InsertColumn(1, "filter", LVCFMT_LEFT, width-20, 1);

    filter_=boost::dynamic_pointer_cast<nodes::filter>(
                nodes::factory().get_node_by_name("filter"));

    std::vector<nodes::filter_item> sets=filter_->get_filter_set();

    unsigned row=0;
    foreach(nodes::filter_item const& set, sets)
    {
        grid_.InsertItem(row, PropCreateReadOnlyItem("", strlib::strf("%d", row+1).c_str()));
        grid_.SetSubItem(row, 1, PropCreateReadOnlyItem("", ""));
        row++;
    }

    SetTimer(0, 300);
    UpdateGridValues();

    node_.reset( new nodes::filter );
    node_->set_parent(nodes::factory().get_node_by_name("sorter").get());
    node_->set_negative_flag(1);

    grader_ = nodes::factory().create_temp_grader(node_);

    plot1_.reset( new CPlotView(0, 1), do_not_delete);
    axe_info x2={aera::C_Amplitude, true, 0, 0, false};
    axe_info y2={aera::C_Duration, true, 0, 0, true};
    plot_info p2={scatter, true, false, 200};
    plot1_->set_xaxe_info(x2);
    plot1_->set_yaxe_info(y2);
    plot1_->set_plot_params(p2);
    plot1_->set_parent(boost::static_pointer_cast<nodes::node>(grader_));

    CRect rc1; ::GetWindowRect(GetDlgItem(IDC_FL_CHART2), rc1);
    ScreenToClient(rc1);
    plot1_->Create(m_hWnd, rc1, 0, WS_CHILD|WS_VISIBLE, 0, IDC_FL_CHART_2);

    //////////////////////////////////////////////////////////////////////////

    plot2_.reset( new CPlotView(0, 1), do_not_delete );
    axe_info x3={aera::C_Amplitude, true, 0, 0, false};
    axe_info y3={aera::C_Hits, true, 0, 0, true};
    plot_info p3={maxmin, true, false, 200};
    plot2_->set_xaxe_info(x3);
    plot2_->set_yaxe_info(y3);
    plot2_->set_plot_params(p3);
    plot2_->set_parent(boost::static_pointer_cast<nodes::node>(grader_));

    CRect rc2; ::GetWindowRect(GetDlgItem(IDC_FL_CHART3), rc2);
    ScreenToClient(rc2);
    plot2_->Create(m_hWnd, rc2, 0, WS_CHILD|WS_VISIBLE, 0, IDC_FL_CHART2_2);

    //////////////////////////////////////////////////////////////////////////

    plot3_.reset( new CPlotView(0, 1), do_not_delete );
    axe_info x1={aera::C_Time, true, 0, 0, false};
    axe_info y1={aera::C_Duration, true, 0, 0, true};
    plot_info p1={scatter, true, false, 200};
    plot3_->set_xaxe_info(x1);
    plot3_->set_yaxe_info(y1);
    plot3_->set_plot_params(p1);
    plot3_->set_parent(boost::static_pointer_cast<nodes::node>(grader_));

    CRect rc3; ::GetWindowRect(GetDlgItem(IDC_FL_CHART), rc3);
    ScreenToClient(rc3);
    plot3_->Create(m_hWnd, rc3, 0, WS_CHILD|WS_VISIBLE, 0, IDC_FL_CHART3_2);

    //////////////////////////////////////////////////////////////////////////
    /// \brief DlgResize_Init
    ///
    DlgResize_Init(true, true, 0);

    //////////////////////////////////////////////////////////////////////////

    if (row)
    {
        grid_.SelectItem(0, 1);
    }

    return 0;
}

void CFilterDialog::DlgResize_UpdateLayout(int cx, int cy)
{
    CDialogResize<CFilterDialog>::DlgResize_UpdateLayout(cx, cy);
    CRect rc1; plot1_->GetWindowRect(&rc1); ScreenToClient(&rc1);
    CRect rc2; plot3_->GetWindowRect(&rc2); ScreenToClient(&rc2);

    CRect rc = CRect(rc1.left, rc1.top, rc2.right, rc2.bottom);
    CPoint c = rc.CenterPoint();

    plot1_->MoveWindow(CRect( rc.left, rc.top, c.x, c.y));
    plot2_->MoveWindow(CRect( c.x, rc.top, rc.right, c.y ));
    plot3_->MoveWindow(CRect( rc.left, c.y, rc.right, rc.bottom ));
}

LRESULT CFilterDialog::OnRestore(UINT, int, HWND)
{
    unsigned index=grid_.GetSelectedIndex();

    std::vector<nodes::filter_item> sets=
            filter_->get_filter_set();

    if (index < sets.size())
    {
        sets.erase(sets.begin()+index);
        filter_->set_filter_set(sets);

        grid_.DeleteItem( index );

        if (sets.empty())
        {
            node_->set_filter_set(std::vector<nodes::filter_item>());
        } else {
            grid_.SelectItem(std::min(index, (unsigned)grid_.GetItemCount()-1), 1);
        }

        Document().SetModified();
    }

    return 0;
}



LRESULT CFilterDialog::OnIdClose(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(IDOK);

    return 0;
}

LRESULT CFilterDialog::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "filter-history");
    return 0;
}

LRESULT CFilterDialog::OnChangedFilter(LPNMHDR)
{
    unsigned index=grid_.GetSelectedIndex();

    std::vector<nodes::filter_item> sets=
            filter_->get_filter_set();

    if (index < sets.size())
    {
        nodes::filter_item item=sets[index];

        node_->clear();
        node_->append_set( item.fset );
    }
    return 0;
}

void CFilterDialog::UpdateGridValues()
{
    std::vector<nodes::filter_item> sets=
            filter_->get_filter_set();

    bool need_update=false;
    for(unsigned index=0; index<sets.size(); ++index)
    {
        nodes::filter_item item=sets[index];
        if (!item.fset) need_update=true;
        std::string text=item.fset
                ? strlib::strf("%d records", item.fset->size())
                : "n/a";
        HPROPERTY prop=grid_.GetProperty(index, 1);
        prop->SetValue( CComVariant(text.c_str()) );
    }

    grid_.Invalidate();

    if (!need_update)
    {
        unsigned index=grid_.GetSelectedIndex();
        grid_.SelectItem(index, 0);
        grid_.SelectItem(index, 1);
        KillTimer(0);
    }
}

LRESULT CFilterDialog::OnTimer(UINT, TIMERPROC)
{
    UpdateGridValues();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
