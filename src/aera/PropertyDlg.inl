
#include "PropertyDlg.h"

#include "utilites/Localizator.h"
#include "utilites/tokenizer.h"
#include "wtladdons.h"

#include "data/nodeFactory.h"
#include "data/slice.h"
#include "data/node.h"

LRESULT CPropertyDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  _lw(m_hWnd, IDD_FILE_PROPERTY);

  date_.Attach( GetDlgItem(IDC_PROP_DATE) );
  title_.Attach( GetDlgItem(IDC_PROP_TITLE) );
  product_.Attach( GetDlgItem(IDC_PROP_PRODUCT) );

  CenterWindow(GetParent());

  LoadData();
  return TRUE;
}

LRESULT CPropertyDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    EndDialog(wID);
    return 0;
}

LRESULT CPropertyDlg::OnTimer(UINT id, TIMERPROC)
{ 
    KillTimer(id);
    LoadData();
    return 0;
}

void CPropertyDlg::LoadData()
{
    nodes::node* node=nodes::factory().get_active_node().get();

    if (!node->is_ready())
    {
        SetTimer(1, 100);
        return;
    }

    nodes::presult r = process::get_result<nodes::result>(node);
    data::pslice slice=r->raw;

    int count = 3;

    if (slice->size())
    {
        for(unsigned index=0; index<slice->size() && count; ++index)
        {
            data::raw_record record=slice->get_raw_record(index);
            switch(record.type_)
            {
            case data::CR_TITLE:
                MakeWindowText(title_)=std::string(record.string_, record.string_size_);
                --count;
                break;
            case data::CR_SOFTWARE:
                MakeWindowText(product_)=std::string(record.string_, record.string_size_);
                --count;
                break;
            case data::CR_DATE:
                MakeWindowText(date_)=std::string(record.string_, record.string_size_);
                --count;
                break;
            }
        }
    }
}
