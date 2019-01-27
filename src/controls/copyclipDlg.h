#pragma once

#include "aera/wtladdons.h"
#include "aera/resource.h"

#include "utilites/Localizator.h"

#include "utilites/strings.h"

class CCopyClipDlg : public CDialogImpl<CCopyClipDlg>
{
public:
    enum { IDD = IDD_PLOT_COPYCLIPDLG };

    int lock;
    int dpi, dpiA, dpiB;
    int pixelWidth;
    int pixelHeight;
    int mmWidth;
    int mmHeight;

    BEGIN_MSG_MAP(CCopyClipDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDRETRY, OnCloseCmd)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)

        COMMAND_HANDLER(IDC_COPYCLIP_WIDTH, EN_CHANGE, OnChangeText);
        COMMAND_HANDLER(IDC_COPYCLIP_HEIGHT, EN_CHANGE, OnChangeText);
        COMMAND_HANDLER(IDC_COPYCLIP_WIDTHMM, EN_CHANGE, OnChangeText);
        COMMAND_HANDLER(IDC_COPYCLIP_HEIGHTMM, EN_CHANGE, OnChangeText);

    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
    {
        _lw(*this, IDD_PLOT_COPYCLIPDLG);
        CenterWindow(GetParent());
        lock = 0;

        HDC dc = ::GetDC(0);
        dpi = GetDeviceCaps(dc, LOGPIXELSX); // 96
//        dpiA = GetDeviceCaps(dc, HORZRES); // 73.5 (םו בü¸ע ס גמנהמל)
//        dpiB = GetDeviceCaps(dc, HORZSIZE);
        dpiA = dpi * 10;
        dpiB = 254;
        ::ReleaseDC(0, dc);

        mmWidth = pixelWidth * dpiB / dpiA;
        mmHeight = pixelHeight * dpiB / dpiA;

        CWindowText( GetDlgItem(IDC_COPYCLIP_WIDTH)  )=strlib::read(pixelWidth, std::string());
        CWindowText( GetDlgItem(IDC_COPYCLIP_HEIGHT) )=strlib::read(pixelHeight, std::string());

        return TRUE;
    }


    LRESULT OnChangeText(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        if (lock) return 0;
        ++lock;

        int input=strlib::read(CWindowText( GetDlgItem(wID)  ), 0);
        int output;
        int outputID;

        if (wID == IDC_COPYCLIP_WIDTH)
        {
            outputID = IDC_COPYCLIP_WIDTHMM;
            pixelWidth = input;
            output = mmWidth = pixelWidth * dpiB / dpiA;
        } else if (wID == IDC_COPYCLIP_HEIGHT)
        {
            outputID = IDC_COPYCLIP_HEIGHTMM;
            pixelHeight = input;
            output = mmHeight = pixelHeight* dpiB / dpiA;
        } else if (wID == IDC_COPYCLIP_WIDTHMM)
        {
            outputID = IDC_COPYCLIP_WIDTH;
            mmWidth = input;
            output = pixelWidth = mmWidth * dpiA / dpiB;
        } else if (wID == IDC_COPYCLIP_HEIGHTMM)
        {
            outputID = IDC_COPYCLIP_HEIGHT;
            mmHeight = input;
            output = pixelHeight = mmHeight * dpiA / dpiB;
        }

        CWindowText( GetDlgItem(outputID)  )=strlib::read(output, std::string());

        --lock;
        return 0;
    }

    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
    {
        try
        {
            if ((wID)==IDOK)
            {
                pixelWidth=boost::lexical_cast<int>(CWindowText( GetDlgItem(IDC_COPYCLIP_WIDTH)  ));
                pixelHeight=boost::lexical_cast<int>(CWindowText( GetDlgItem(IDC_COPYCLIP_HEIGHT) ));
            }
            EndDialog(wID);
        }
        catch (boost::bad_lexical_cast &)
        {
        }
        return 0;
    }

    unsigned *params;
};
