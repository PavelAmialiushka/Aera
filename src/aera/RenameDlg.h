#pragma once

#include "wtladdons.h"
#include "utilites/Localizator.h"
#include "resource.h"

class CRenameDlg : public CDialogImpl<CRenameDlg>
{
public:
    enum { IDD = IDD_PAGES_RENAME };

    BEGIN_MSG_MAP(CRenameDlg)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
    {
        _lw(*this, IDD_PAGES_RENAME);

        CenterWindow(GetParent());

        rets_=(std::string *)lParam;
        rename_.Attach( GetDlgItem(IDC_PAGES_RENAME_EDIT) );
        CWindowText(rename_.m_hWnd)=*rets_;

        rename_.SetFocus();
        rename_.SetSelAll();

        return TRUE;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        if ((wID)==IDOK)
        {
            *rets_=CWindowText(rename_.m_hWnd);
        }
        EndDialog(wID);
        return 0;
    }

    std::string  *rets_;
    CEdit rename_;
};
