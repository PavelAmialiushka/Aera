#pragma once

#include "others/atlgdix.h"
#include "others/tabbing/DotNetTabCtrl.h"
#include "others/tabbing/TabbedFrame.h"

//////////////////////////////////////////////////////////////////////////

class CTabbedChildWindowX
    : public CTabbedFrameImpl<CTabbedChildWindowX,
      CDotNetTabCtrl<CTabViewTabItem>,
      CTabbedChildWindowBase<CTabbedChildWindowX, CWindow, TabbedChildWindowWinTraits> >
{
protected:

    typedef CTabbedChildWindowX thisClass;
    typedef CTabbedFrameImpl<CTabbedChildWindowX, CDotNetTabCtrl<CTabViewTabItem>, CTabbedChildWindowBase<CTabbedChildWindowX, CWindow, TabbedChildWindowWinTraits> > baseClass;

// Constructors
public:
    CTabbedChildWindowX(bool bReflectNotifications = false)
        : baseClass(bReflectNotifications)
    {
    }

// Message Handling
public:
    DECLARE_FRAME_WND_CLASS_EX(_T("TabbedChildWindowX"), 0, 0, COLOR_APPWORKSPACE)

    BOOL PreTranslateMessage(MSG *pMsg)
    {
        //if(baseClass::PreTranslateMessage(pMsg))
        //  return TRUE;

        //return m_view.PreTranslateMessage(pMsg);

        HWND hWndFocus = ::GetFocus();
        if (m_hWndActive != NULL && ::IsWindow(m_hWndActive) &&
                (m_hWndActive == hWndFocus || ::IsChild(m_hWndActive, hWndFocus)))
        {
            //active.PreTranslateMessage(pMsg);
            if (::SendMessage(m_hWndActive, WM_FORWARDMSG, 0, (LPARAM)pMsg))
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    LRESULT OnNotify(int id, LPNMHDR hdr);

    BEGIN_MSG_MAP_EX(thisClass)
    MSG_WM_NOTIFY(OnNotify);
    CHAIN_MSG_MAP(baseClass)
    END_MSG_MAP()

    HWND  GetView(unsigned index);

};
