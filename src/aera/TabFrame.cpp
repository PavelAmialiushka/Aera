#include "stdafx.h"

#include "TabFrame.h"
#include "utilites/Localizator.h"
#include "resource.h"

#include "tileview/TVFrame.h"

LRESULT CTabbedChildWindowX::OnNotify(int id, LPNMHDR hdr)
{
    if (hdr->code >= TVFN_FIRST && hdr->code <= TVFN_LAST)
    {
        return ::SendMessage(GetParent(), WM_NOTIFY, (WPARAM)id, (LPARAM)hdr);
    }
    else
    {
        switch (hdr->code)
        {
        case NM_RCLICK:
        {
            CMenu menu( ::CreatePopupMenu() );
            menu.AppendMenu(MF_STRING, IDC_PAGES_NEW_PAGE, _lcs("&New page"));
            menu.AppendMenu(MF_STRING, IDC_PAGES_RENAME_PAGE, _lcs("&Rename"));
            menu.AppendMenu(MF_SEPARATOR);
            menu.AppendMenu(MF_STRING, IDC_PAGES_DELETE_PAGE, _lcs("&Delete tab"));

            CPoint pt; GetCursorPos(&pt);
            int ret=menu.TrackPopupMenu(
                        TPM_RETURNCMD|TPM_BOTTOMALIGN|TPM_RIGHTALIGN,
                        pt.x, pt.y, *this);

            if (ret!=0)
            {
                ::PostMessage(GetParent(), WM_COMMAND, MAKEWPARAM(ret, 1), 0);
            }
        }
        break;
        case CTCN_SELCHANGE:
            ::SetFocus(GetActiveView());
        default:
            break;
        }
    }
    SetMsgHandled(false);
    return 0;
}

HWND  CTabbedChildWindowX :: GetView(unsigned index)
{
    CTabViewTabItem *item=GetTabCtrl().GetItem(index);
    debug::Assert<fault>(item, HERE);
    return item->GetTabView();
}
