#ifndef LISTVIEWCTRLOWNERDRAW_H
#define LISTVIEWCTRLOWNERDRAW_H

#include <atlwin.h>
#include <atlctrls.h>
#include <atlgdi.h>
#include <CommCtrl.h>
using WTL::CListViewCtrl;
using WTL::CCustomDraw;

#define CDRF_SKIPPOSTPAINT      0x00000100 // don't draw the focus rect

class CListViewCtrlOwnerDraw
        : public CWindowImpl<CListViewCtrlOwnerDraw, CListViewCtrl>
        , public CCustomDraw<CListViewCtrlOwnerDraw>

{

    BEGIN_MSG_MAP_EX(CListViewCtrlOwnerDraw)
      CHAIN_MSG_MAP(CCustomDraw<CListViewCtrlOwnerDraw>)
    END_MSG_MAP()

    DWORD OnPreErase(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return  CDRF_NOTIFYITEMDRAW;
    }

    DWORD OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
    {
        return  CDRF_NOTIFYITEMDRAW;
    }

    DWORD OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
    {
        NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( lpNMCustomDraw );

        int index = pLVCD->nmcd.dwItemSpec;
        bool selected = GetItemState(index, LVIS_FOCUSED);
        bool active = GetFocus() == m_hWnd;

        if (selected)
        {
//            COLORREF back = RGB(212, 212, 212);
            COLORREF back = RGB(51, 153, 254);
            COLORREF front = RGB(255,255,255);

            char text[1024];
            LVITEM item = {LVIF_TEXT};
            item.iItem = index;
            item.pszText = text;
            item.cchTextMax = sizeof(text);
            GetItem(&item);

            CDCHandle dc(pLVCD->nmcd.hdc);

            CRect rc;
            GetSubItemRect(index, 0, LVIR_LABEL, &rc);

            CBrush brush = ::CreateSolidBrush(back);
            dc.FillRect(rc, brush);

            dc.SetTextColor(front);
            rc.left +=2;
            dc.DrawText(text, -1, rc, DT_LEFT | DT_NOCLIP | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            return CDRF_SKIPDEFAULT;
        }

        // Tell Windows to paint the control itself.
        return CDRF_DODEFAULT;
    }
};

#endif // LISTVIEWCTRLOWNERDRAW_H
