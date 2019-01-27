
#include "settingsdlg.h"

#include "utilites/Localizator.h"
#include "utilites/help.h"

#include "wtladdons.h"

#include "data/nodeFactory.h"
#include "data/parametric.h"

#include "utilites/serl/serl_registry.h"

LRESULT CSettingsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  _lw(m_hWnd, IDD_SETTINGSDLG);

  parametric values[4];
  nodes::factory().get_parametric_data(values);

  CWindowText( GetDlgItem(IDC_SETT_NAME1) ) = values[0].name;
  CWindowText( GetDlgItem(IDC_SETT_UNIT1) ) = values[0].unit_name;
  CWindowText( GetDlgItem(IDC_SETT_ADDEND1) ) = boost::lexical_cast<std::string>(values[0].addend);
  CWindowText( GetDlgItem(IDC_SETT_FACTOR1) ) = boost::lexical_cast<std::string>(values[0].factor);

  CWindowText( GetDlgItem(IDC_SETT_NAME2) ) = values[1].name;
  CWindowText( GetDlgItem(IDC_SETT_UNIT2) ) = values[1].unit_name;
  CWindowText( GetDlgItem(IDC_SETT_ADDEND2) ) = boost::lexical_cast<std::string>(values[1].addend);
  CWindowText( GetDlgItem(IDC_SETT_FACTOR2) ) = boost::lexical_cast<std::string>(values[1].factor);

  CWindowText( GetDlgItem(IDC_SETT_NAME3) ) = values[2].name;
  CWindowText( GetDlgItem(IDC_SETT_UNIT3) ) = values[2].unit_name;
  CWindowText( GetDlgItem(IDC_SETT_ADDEND3) ) = boost::lexical_cast<std::string>(values[2].addend);
  CWindowText( GetDlgItem(IDC_SETT_FACTOR3) ) = boost::lexical_cast<std::string>(values[2].factor);

  CWindowText( GetDlgItem(IDC_SETT_NAME4) ) = values[3].name;
  CWindowText( GetDlgItem(IDC_SETT_UNIT4) ) = values[3].unit_name;
  CWindowText( GetDlgItem(IDC_SETT_ADDEND4) ) = boost::lexical_cast<std::string>(values[3].addend);
  CWindowText( GetDlgItem(IDC_SETT_FACTOR4) ) = boost::lexical_cast<std::string>(values[3].factor);

  CenterWindow(GetParent());

  redbrush_.CreateSolidBrush( RGB(255,0,0) );

  return TRUE;
}

LRESULT CSettingsDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  try {
    if (wID==IDOK)
    {
      parametric values[4];
      values[0].name=CWindowText( GetDlgItem(IDC_SETT_NAME1) );
      values[0].unit_name=CWindowText( GetDlgItem(IDC_SETT_UNIT1) );
      values[0].addend=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_ADDEND1) ));
      values[0].factor=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_FACTOR1) ));

      values[1].name=CWindowText( GetDlgItem(IDC_SETT_NAME2) );
      values[1].unit_name=CWindowText( GetDlgItem(IDC_SETT_UNIT2) );
      values[1].addend=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_ADDEND2) ));
      values[1].factor=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_FACTOR2) ));

      values[2].name=CWindowText( GetDlgItem(IDC_SETT_NAME3) );
      values[2].unit_name=CWindowText( GetDlgItem(IDC_SETT_UNIT3) );
      values[2].addend=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_ADDEND3) ));
      values[2].factor=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_FACTOR3) ));

      values[3].name=CWindowText( GetDlgItem(IDC_SETT_NAME4) );
      values[3].unit_name=CWindowText( GetDlgItem(IDC_SETT_UNIT4) );
      values[3].addend=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_ADDEND4) ));
      values[3].factor=boost::lexical_cast<double>( CWindowText( GetDlgItem(IDC_SETT_FACTOR4) ));

      std::deque<std::string> list;
      serl::load_archiver( 
        new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data\\Parametric") ).
        serial_container("mru-list", list);

      std::string sum =MakeWindowText( GetDlgItem(IDC_SETT_ADDEND1) );
                  sum+=" ";
                  sum+=MakeWindowText( GetDlgItem(IDC_SETT_FACTOR1) );
      list.push_front(sum);

                  sum=MakeWindowText( GetDlgItem(IDC_SETT_ADDEND2) );
                  sum+=" ";
                  sum+=MakeWindowText( GetDlgItem(IDC_SETT_FACTOR2) );
      list.push_front(sum);

                  sum=MakeWindowText( GetDlgItem(IDC_SETT_ADDEND3) );
                  sum+=" ";
                  sum+=MakeWindowText( GetDlgItem(IDC_SETT_FACTOR3) );
      list.push_front(sum);

                  sum=MakeWindowText( GetDlgItem(IDC_SETT_ADDEND4) );
                  sum+=" ";
                  sum+=MakeWindowText( GetDlgItem(IDC_SETT_FACTOR4) );
      list.push_front(sum);

      while (list.size()>10) list.pop_back();

      serl::save_archiver( 
        new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data\\Parametric") ).
        serial_container("mru-list", list);

      nodes::factory().set_parametric_data(values);
    }
    EndDialog(wID);
  } catch(boost::bad_lexical_cast&) 
  {
  }
  return 0;
}

LRESULT CSettingsDlg::OnSetParametric(UINT, int wm, HWND)
{
  std::deque<std::string> list;
  serl::load_archiver( 
    new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data\\Parametric") ).
    serial_container("mru-list", list);

  std::sort(STL_II(list));
  list.erase(std::unique(STL_II(list)), list.end());

  CMenu menu( ::CreatePopupMenu() );
  
  for(unsigned index=0; index<list.size(); ++index) 
  {
    menu.AppendMenu(MF_STRING, index+1, list[index].c_str());
  }

  if (list.size())
  {
    CRect rc; ::GetWindowRect(GetDlgItem(wm), rc);
    int ret=menu.TrackPopupMenu(
      TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
      rc.left, rc.bottom, m_hWnd);

    if (!ret) return 0;
    --ret;
    
    std::string last=list[ret];
    list.erase(list.begin()+ret);
    list.push_front(last);

    switch(wm)
    {
    case IDC_SET_PARAMETRIC1:
      CWindowText(GetDlgItem(IDC_SETT_ADDEND1))=tokenizer::head(last, " ");
      CWindowText(GetDlgItem(IDC_SETT_FACTOR1))=tokenizer::head(last, " ");
      break;
    case IDC_SET_PARAMETRIC2:
      CWindowText(GetDlgItem(IDC_SETT_ADDEND2))=tokenizer::head(last, " ");
      CWindowText(GetDlgItem(IDC_SETT_FACTOR2))=tokenizer::head(last, " ");
      break;
    case IDC_SET_PARAMETRIC3:
      CWindowText(GetDlgItem(IDC_SETT_ADDEND3))=tokenizer::head(last, " ");
      CWindowText(GetDlgItem(IDC_SETT_FACTOR3))=tokenizer::head(last, " ");
      break;
    case IDC_SET_PARAMETRIC4:
      CWindowText(GetDlgItem(IDC_SETT_ADDEND4))=tokenizer::head(last, " ");
      CWindowText(GetDlgItem(IDC_SETT_FACTOR4))=tokenizer::head(last, " ");
      break;
    }
    serl::save_archiver( 
      new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data\\Parametric") ).
      serial_container("mru-list", list);

  }

  return 0;
}

LRESULT CSettingsDlg::OnCtlColorEdit(HDC dc, HWND hwnd)
{
  int id=::GetWindowLong(hwnd, GWL_ID);
  switch ( id )
  {
  case IDC_SETT_ADDEND1:
  case IDC_SETT_FACTOR1:
  case IDC_SETT_ADDEND2:
  case IDC_SETT_FACTOR2:
  case IDC_SETT_ADDEND3:
  case IDC_SETT_FACTOR3:
  case IDC_SETT_ADDEND4:
  case IDC_SETT_FACTOR4:
    SetBkMode(dc, TRANSPARENT);
    try {
      boost::lexical_cast<double>(MakeWindowText(hwnd));
    } catch (boost::bad_lexical_cast) {
      return (LRESULT)redbrush_.m_hBrush;
    }
  }
  SetMsgHandled(false);
  return 0;
}

LRESULT CSettingsDlg::OnChangeEdit(UINT, int id, HWND)
{
  switch ( id )
  {
  case IDC_SETT_ADDEND1:
  case IDC_SETT_FACTOR1:
  case IDC_SETT_ADDEND2:
  case IDC_SETT_FACTOR2:
  case IDC_SETT_ADDEND3:
  case IDC_SETT_FACTOR3:
  case IDC_SETT_ADDEND4:
  case IDC_SETT_FACTOR4:
    ::InvalidateRect(GetDlgItem(id), 0, true);
  } 
  return 0;
}

LRESULT CSettingsDlg::OnHelp(LPHELPINFO)
{
  //help(m_hWnd, "parametric");
  return 0;
}
