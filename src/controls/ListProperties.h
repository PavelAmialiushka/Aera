#pragma once

#include "aera\Resource.h"

#include "controls\ListModel.h"

class ListPropertyDlg : public CDialogImpl<ListPropertyDlg>
{
public:
    enum { IDD=IDD_LISTPPCOMMON };

    ListPropertyDlg(list_model *model, HWND parent);

    typedef CDialogImpl<ListPropertyDlg> baseClass;

    BEGIN_MSG_MAP_EX(ListPropertyDlg)
    MSG_WM_INITDIALOG(OnInitDialog)

    COMMAND_ID_HANDLER_EX(IDOK, OnClose);
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnClose);

    COMMAND_ID_HANDLER_EX(IDC_LEFT, OnLeft)
    COMMAND_ID_HANDLER_EX(IDC_RIGHT, OnRight)
    COMMAND_ID_HANDLER_EX(IDC_LEFTALL, OnLeftAll)
    COMMAND_ID_HANDLER_EX(IDC_RIGHTALL, OnRightAll)
    COMMAND_ID_HANDLER_EX(IDC_UP, OnUp)
    COMMAND_ID_HANDLER_EX(IDC_DOWN, OnDown)

    COMMAND_HANDLER_EX(IDC_LP_USE_LOCATION, BN_CLICKED, OnCheckButton);
    COMMAND_HANDLER_EX(IDC_LP_LOCATION, CBN_SELCHANGE, OnLocationChanged);

    COMMAND_HANDLER(IDC_LP_TIME1, BN_CLICKED, OnCheckButton);
    COMMAND_HANDLER(IDC_LP_TIME2, BN_CLICKED, OnCheckButton);
    COMMAND_HANDLER(IDC_LP_REL_TIME, BN_CLICKED, OnCheckButton);
    COMMAND_HANDLER(IDC_LP_NAV_MODE, BN_CLICKED, OnCheckButton);

    MSG_WM_TIMER(OnTimer);

    COMMAND_HANDLER_EX(IDC_LIST_CURRENT, LBN_DBLCLK, OnCurrentDblClick);
    COMMAND_HANDLER_EX(IDC_LIST_AVIABLE, LBN_DBLCLK, OnAvailableDblClick);

    MSG_WM_HELP(OnHelp);

    //CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    LRESULT OnInitDialog(HWND hWnd, LPARAM ppage);

    LRESULT OnLeft(UINT code, int id, HWND wnd);
    LRESULT OnRight(UINT code, int id, HWND wnd);
    LRESULT OnLeftAll(UINT code, int id, HWND wnd);
    LRESULT OnRightAll(UINT code, int id, HWND wnd);
    LRESULT OnDragListNotify(UINT code, int id, HWND wnd);

    LRESULT OnUp(UINT code, int id, HWND wnd);
    LRESULT OnDown(UINT code, int id, HWND wnd);

    LRESULT OnCurrentDblClick(UINT code, int id, HWND wnd);
    LRESULT OnAvailableDblClick(UINT code, int id, HWND wnd);

    LRESULT OnCheckButton(...);
    void OnLocationChanged(...);

    LRESULT OnHelp(LPHELPINFO=0);

    void    Apply();
    LRESULT OnTimer(UINT);

    LRESULT OnClose(UINT, int, HWND);

private:

    void update_list();
    void _MoveLeft(int rsel);
    void _MoveRight(int lsel);

    void RealApply();
    void MakeFeatureList();
    void MakeFeatureListDelayed();
    void EnableControls(bool);

private:
    list_model  *model_;

    bool         delayed_;

    CWindow   wndParent;
    CButton   use_location_;
    CComboBox location_type_;

    std::vector<std::string> aviable_list_;
    std::vector<std::string> current_list_;

    CDragListBox m_current;
    CListBox m_available;
};
