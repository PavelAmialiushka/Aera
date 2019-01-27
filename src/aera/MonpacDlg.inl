
#include "aera/resource.h"
#include "utilites/Localizator.h"

#include "MonpacDlg.h"

#include "data/monpac.h"
#include "data/criterion.h"
#include "data/rtf_outputer.h"
#include "data/console_out.h"

#include "utilites/help.h"

#include "aera/wtladdons.h"

#include "Controls/MonpacWnd.h"

#include "data/nodeFactory.h"
#include "data/node.h"
#include "data/stage.h"
#include "data/channel.h"

#include "Tileview/TVFrame.h"


#define _blc boost::lexical_cast
#define _blcs(s) boost::lexical_cast<std::string>(s)
#define _blcc(s) boost::lexical_cast<std::string>(s).c_str()

//////////////////////////////////////////////////////////////////////////

namespace
{
int monpac_cur_sel_mode=0;
}

std::string conv(const char* frmt, double d) {
    char buffer[24]; sprintf(buffer, frmt, d); return buffer; }

std::string conv(const char* frmt, int d) {
    char buffer[24];  sprintf(buffer, frmt, d); return buffer; }


std::string itoa( int i ) {
    return conv("%d", i); }

//////////////////////////////////////////////////////////////////////////

CMonpacDialog::~CMonpacDialog()
{
    grader_->detach_listener(this);
}

LRESULT CMonpacDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    lock_changes_=1;
    timer_=0;
    intens_mode_=0;

    _lw(m_hWnd, IDD_MONPAC);

    CenterWindow(GetParent());

    grader_= nodes::factory().create_temp_grader();
    grader_->attach_listener(this);

    CRect img_rect;
    ::GetWindowRect( GetDlgItem(IDC_MP_GRID), img_rect);
    ScreenToClient(img_rect);
    view_=new CMonpacWindow();
    view_->Create(m_hWnd, img_rect, NULL, WS_CHILD|WS_VISIBLE);

    stages_.Attach( GetDlgItem(IDC_MP_STAGES) );

    // заполняем комбобокс стадиями
    stages_.AddString(_lcs("Entire test"));
    stages_.SetCurSel(0);

    stage stg=nodes::factory().get_sel_stage();
    for(unsigned index=0; index<stages::instance()->size(); ++index)
    {
        stage item=*stages::instance()->at(index);
        stages_.AddString( item.name.c_str() );

        if (item.get_id()==stg.get_id())
            stages_.SetCurSel(index+1);
    }

    // grid
    grid_.Attach( GetDlgItem(IDC_MP_GRID) );
    grid_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);


    ImportCriterion();

    SetMonpacMode(monpac_cur_sel_mode);
    OnChangedStage();

    return TRUE;
}

void CMonpacDialog::SetMonpacMode(int mode)
{
    monpac_cur_sel_mode=mode;
    intens_mode_=mode;
    CheckRadioButton(IDC_MP_EVAL, IDC_MP_GRAPH,
                     mode==0 ? IDC_MP_EVAL :
                               mode==1 ? IDC_MP_INTENS : IDC_MP_GRAPH);

    if (int count=grid_.GetItemCount())
    {
        selected_item_=0;
        for(int index=0; index<count; ++index)
        {
            if (grid_.GetItemState(index, LVIS_SELECTED)) selected_item_=index;
        }
    }

    if (mode<2)
    {
        grid_.ShowWindow(SW_SHOW);
        view_->ShowWindow(SW_HIDE);

        CRect rc; grid_.GetWindowRect(rc);
        int width=rc.Width()-GetSystemMetrics(SM_CXVSCROLL)-30-13;

        grid_.DeleteAllItems();

        while (grid_.DeleteColumn(0)) ;
        grid_.InsertColumn(0, "Ch",       LVCFMT_RIGHT, 30, -1);
        if (intens_mode_) {
            grid_.InsertColumn(1, _lcs("MPtotal#Total hits"),   LVCFMT_RIGHT, width/4, 1);
            grid_.InsertColumn(2, _lcs("Severity"),   LVCFMT_RIGHT, width/4, 1);
            grid_.InsertColumn(3, _lcs("MPHI#Hist.index"),  LVCFMT_RIGHT, width/4, 1);
            grid_.InsertColumn(4, _lcs("ZIP"),    LVCFMT_RIGHT, width/4, 1);
        } else {
            grid_.InsertColumn(1, _lcs("MPHolds#Holds"),    LVCFMT_RIGHT, width*2/6, 1);
            grid_.InsertColumn(2, _lcs("MPtotal#Total hits"),   LVCFMT_RIGHT, width/6, 2);
            grid_.InsertColumn(3, _lcs("Amp>65"),   LVCFMT_RIGHT, width/6, 3);
            grid_.InsertColumn(4, _lcs("Durat#Duration"),   LVCFMT_RIGHT, width/6, 4);
            grid_.InsertColumn(5, _lcs("ZIP"),      LVCFMT_RIGHT, width/6, 5);
        }
    } else {
        grid_.ShowWindow(SW_HIDE);
        view_->ShowWindow(SW_SHOW);
    }

    DelayedApply();
}

void CMonpacDialog::ExportCriterion()
{
    int type=
            IsDlgButtonChecked(IDC_MP_TYPE1) ? 0 :
                                               IsDlgButtonChecked(IDC_MP_TYPE2) ? 1 : 2;
    criterion_.set_vessel(type);

    int ith=IsDlgButtonChecked(IDC_MP_IT_EVAL) ? 0 : 1;
    criterion_.set_test_threshold( ith );

    unsigned num=stages_.GetCurSel();
    criterion_.set_stage(
                num==0
                ? *stages::instance()
                : *stages::instance()->at(num-1) );

    nodes::factory().set_monpac_criterion(criterion_, id_);

    monpac_=shared_ptr<monpac::classifier>(new monpac::classifier(criterion_));
    monpac_->set_parent( grader_.get() );
    monpac_->attach_listener(this);
}

void CMonpacDialog::OnChanged(...)
{
    if (!lock_changes_)
    { lock_changes_=1;
        update_needed_=true;

        // заполняем критерий
        // при этом сам вызывается обновитель
        ExportCriterion();

        lock_changes_=0;
    }
}

void CMonpacDialog::DelayedApply()
{
    update_needed_=true;
    ::SetTimer(m_hWnd, 1, 100, NULL);
}

LRESULT CMonpacDialog::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ::KillTimer(m_hWnd, 1);

    EndDialog(wID);
    return 0;
}

void CMonpacDialog::OnUpdate(...)
{
    update_needed_=true;
    Apply();
}

void CMonpacDialog::Apply()
{
    if (update_needed_)
    {
        update_needed_=false;
        if (int count=grid_.GetItemCount())
        {
            selected_item_=0;
            for(int index=0; index<count; ++index)
            {
                if (grid_.GetItemState(index, LVIS_SELECTED)) selected_item_=index;
            }
        }
        grid_.DeleteAllItems();

        result_ = process::get_result<monpac::monpac_result>(monpac_);
        for(unsigned index=0; index<result_->ch_info_.size(); ++index)
        {
            monpac::channel ch=result_->ch_info_[index];

            grid_.InsertItem(index, _blcc(ch.get_number()) );
            grid_.SetItemData(index, index);
            if (index==(unsigned)selected_item_)
            {
                grid_.SetItemState(index, LVIS_SELECTED, -1);
                grid_.EnsureVisible(index, false);
            }
        }

        view_->SetMonpac(result_);
    }
}

LRESULT CMonpacDialog::OnTimer(UINT, TIMERPROC)
{
    // вызывается спустя небольшую
    // задержку после изменения параметров

    if (!monpac_)
        return 0;

    monpac::pmonpac_result result = process::get_result<monpac::monpac_result>(monpac_, false);
    if (!result)
        return 0;

    ::KillTimer(m_hWnd, 1);

    update_needed_=true;
    Apply();
    return 0;
}

void CMonpacDialog::DrawItem(CDCHandle dc, int i, int si, std::string str, bool clr)
{
    CRect rc; grid_.GetSubItemRect(i, si, LVIR_BOUNDS, rc);
    dc.SetTextColor(clr ? textcolor_ : RGB(255,0,0));
    dc.DrawText(str.c_str(), str.size(), rc, DT_RIGHT);
}

LRESULT CMonpacDialog :: OnDrawItem(UINT ctrl, LPDRAWITEMSTRUCT dis)
{
    CDCHandle dc(dis->hDC);

    int index=dis->itemID;

    if (dis->itemData >= result_->ch_info_.size()) return 0;
    monpac::channel chn=result_->ch_info_[dis->itemData];

    textcolor_=dis->itemState&LVIS_FOCUSED ? 0xFFFFFF : 0x000000;

    dc.SetTextColor(
                (chn.meet_duration() &&
                 chn.meet_hits65() &&
                 chn.meet_hold_hits() &&
                 chn.meet_total_hits()) ||
                chn.get_zip() < monpac::zip_c
                ? textcolor_ : RGB(255,0,0));

    dc.FillRect(&dis->rcItem, CBrush(
                    ::CreateSolidBrush(dis->itemState&LVIS_FOCUSED ? 0x7F0000 : 0xFFFFFF) ));

    std::string str;
    CRect labelrc;

    str=itoa(  chn.get_number() );
    grid_.GetItemRect(index, labelrc, LVIR_SELECTBOUNDS);
    labelrc.left+=5;
    dc.DrawText(str.c_str(), str.size(), labelrc, DT_LEFT);

    int item=0;
    if (!intens_mode_)
    {
        str=std::string();
        for(unsigned sz=result_->criterion_.get_holds().size(), ix=0; ix<sz; ++ix)
        {
            double hits = chn.get_hold_hits(ix);
            str = tokenizer::join(str, " - ", strlib::strf("%d", hits));
        }

        DrawItem(dc, index, ++item,
                 str,
                 chn.meet_hold_hits());

        DrawItem(dc, index, ++item,
                 _blcs(chn.get_total_hits()) ,
                 chn.meet_total_hits());

        DrawItem(dc, index, ++item,
                 _blcs(chn.get_hits65()),
                 chn.meet_hits65());

        DrawItem(dc, index, ++item,
                 _blcs(chn.get_duration()),
                 chn.meet_duration());

    } else {

        DrawItem(dc, index, ++item,
                 _blcs(chn.get_zip_hits()),
                 true);

        DrawItem(dc, index, ++item,
                 conv("%6.1f", chn.s_index()),
                 true);

        DrawItem(dc, index, ++item,
                 conv("%6.3f", chn.h_index()),
                 true);
    }

    const char* zip_str[]={"-", "A", "B", "C", "D", "E", "N/A",};

    DrawItem(dc, index, ++item,
             zip_str[(int)chn.get_zip()],
            chn.get_zip()<monpac::zip_c);

    return TRUE;
}

LRESULT CMonpacDialog::CopyCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (intens_mode_<2)
    {
        if (monpac_ && ::OpenClipboard(m_hWnd))
        {
            ::EmptyClipboard();

            monpac::rtf_outputter rtf(intens_mode_);
            rtf.accept(monpac_.get());
            std::string text=rtf.get_text();
            text=strlib::replace(text, "\n", "\r\n");

            HGLOBAL clipbuffer= ::GlobalAlloc(GMEM_DDESHARE, text.size()+1);
            char* buffer=(char*)::GlobalLock(clipbuffer);
            strcpy(buffer, text.c_str());
            ::GlobalUnlock(clipbuffer);

            int format=::RegisterClipboardFormat(CF_RTF);
            ::SetClipboardData(format, clipbuffer);
            ::CloseClipboard();
        }
    } else {
        ShowCopyClipDlg(view_->m_hWnd);
    }
    return 0;
}

LRESULT CMonpacDialog::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "monpac");
    return 0;
}

void CMonpacDialog::OnChangedMode(...)
{
    int cr=
            IsDlgButtonChecked(IDC_MP_EVAL) ? 0 :
                                              IsDlgButtonChecked(IDC_MP_INTENS) ? 1 : 2;

    if ( cr!=intens_mode_)
    {
        SetMonpacMode(cr);
    }

}

void CMonpacDialog::ImportCriterion()
{
    lock_changes_=true;

    unsigned sindex=stages_.GetCurSel();

    if (sindex!=0) {
        id_=stages::instance()->at(sindex-1)->get_id();
    } else {
        id_=object_id();
    }

    criterion_=nodes::factory().get_monpac_criterion(id_);

    CheckRadioButton(IDC_MP_TYPE1, IDC_MP_TYPE3,
                     criterion_.get_vessel()==0 ? IDC_MP_TYPE1 :
                                                  criterion_.get_vessel()==1 ? IDC_MP_TYPE2 :
                                                                               IDC_MP_TYPE3 );
    CheckRadioButton(IDC_MP_IT_EVAL, IDC_MP_IT_TEST,
                     criterion_.get_test_threshold() ? IDC_MP_IT_TEST : IDC_MP_IT_EVAL);

    lock_changes_=0;
}

void CMonpacDialog::OnChangedStage(...)
{
    int stg_number=stages_.GetCurSel()-1;
    shared_ptr<stage> new_stage=
            stg_number>=0
            ? stages::instance()->at(stg_number)
            : shared_ptr<stage>( new stage );

    grader_->set_stage(*new_stage);

    // сначала импортируем настройки
    ImportCriterion();

    // а затем перасчитываем график
    OnChanged();
}

void CMonpacDialog::safe_on_finish(process::prslt)
{
    OnUpdate();
}


