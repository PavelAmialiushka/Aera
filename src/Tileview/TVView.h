#pragma once

#include "TVCreator.h"
#include "utilites/Localizator.h"
#include <ATLWIN.H>

enum TVM
{
    TVFM_CHILDLBUTTONDOWN=WM_USER,
    TVFM_DELETECHILD,
    TVFM_REPLACECHILD,
    TVFM_UPDATELAYOUT,
    TVFM_ARRANGEWINDOWS,
    TVFM_STARTDRAG,
    TVFM_ISINDRAGMODE,

    TVVM_GETCLASSPTR,
    TVVM_COPYCLIP,
};

typedef struct _PARENTMOUSEMESSAGE
{
    UINT   nFlags;
    CPoint point;
} PARENTMOUSEMESSAGE;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
template<class T>
class CTileViewImpl
    : public CWindowImpl<T>
{
    shared_ptr< CTVCreator > creator_;
public:

    typedef CWindowImpl<T> base_class;
    BEGIN_MSG_MAP_EX(CTileViewImpl)
    MESSAGE_HANDLER_EX(TVVM_GETCLASSPTR, OnGetClassPtr);

    MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk);

    END_MSG_MAP()

    virtual CTVCreator *MakeClassCreator()
    {
        return new CTVCreatorImpl<T>( static_cast<T *>(this) );
    }

    LRESULT OnGetClassPtr(UINT, WPARAM wParam, LPARAM)
    {
        CTVCreator **ptr=(CTVCreator **)wParam;
        if (!creator_)
        {
            creator_=shared_ptr<CTVCreator>(
                         MakeClassCreator());
        }

        if (ptr)
        {
            *ptr=creator_.get();
        }

        return TRUE;
    }

    LRESULT ParentLButtonDown(UINT nFlags, CPoint pt)
    {
        PARENTMOUSEMESSAGE pmm;
        pmm.nFlags=nFlags;
        pmm.point=pt;
        return ::SendMessage(GetParent(),
                             TVFM_CHILDLBUTTONDOWN,
                             (WPARAM)m_hWnd,
                             (LPARAM)&pmm
                            );
    }

    LRESULT StartDrag(CPoint pt)
    {
        return ::SendMessage(
                   GetParent(),
                   TVFM_STARTDRAG,
                   MAKEWPARAM(pt.x, pt.y),
                   (LPARAM)m_hWnd
               );
    }

    LRESULT IsInDragMode()
    {
        return ::SendMessage(
                   GetParent(),
                   TVFM_ISINDRAGMODE,
                   (WPARAM)m_hWnd,
                   0
               );
    }

    LRESULT DeleteFromParent()
    {
        return ::SendMessage(
                   GetParent(),
                   TVFM_DELETECHILD,
                   (WPARAM)m_hWnd,
                   0
               );
    }

    LRESULT ReplaceChild(HWND newhwnd)
    {
        return ::PostMessage(
                   GetParent(),
                   TVFM_REPLACECHILD,
                   (WPARAM)m_hWnd,
                   (LPARAM)newhwnd);
    }

    LRESULT OnLButtonDblClk(UINT, CPoint pt)
    {
        ClientToScreen(&pt);
        StartDrag(pt);
        return 0;
    }

};

/////////////////////////////////////////////////////////////////////////////

#define STOS(str) (str).c_str(), (str).length()

template<class T>
class CTVSelectImpl : public CTileViewImpl<T>
{
public:

    typedef CTileViewImpl<T> base_class;
    BEGIN_MSG_MAP_EX(CTVSelectImpl)
    MSG_WM_PAINT(OnPaint);
    MSG_WM_ERASEBKGND(OnEraseBackground);

    MSG_WM_CREATE(OnCreate);
    MSG_WM_DESTROY(OnDestroy);

    MSG_WM_SIZE(OnSize);

    MSG_WM_LBUTTONDOWN(OnLButtonDown);
    MSG_WM_MOUSEMOVE(OnMouseMove);
    MSG_WM_MOUSELEAVE(OnMouseLeave);

    MSG_WM_RBUTTONDOWN(OnRButtonDown);

    CHAIN_MSG_MAP(base_class);
    END_MSG_MAP()

    enum { nointersection=-1, };

    //std::vector<std::string> strings_;
    //std::vector< shared_ptr<CTVCreator> > creators_;

    struct pair
    {
        std::string string;
        shared_ptr<CTVCreator> maker;

        void serialization(serl::archiver &ar)
        {
            ar.serial("name", string);
            ar.serial_virtual_ptr("maker", maker);
        }
    };

    std::vector<pair> pairs_;
    CFont font_;
    unsigned    hilight_;

    void serialization(serl::archiver &ar)
    {
        ar.serial_container("types", pairs_);
    }

    LRESULT OnCreate(LPCREATESTRUCT)
    {
        font_.CreatePointFont(100, "Tahoma");
        hilight_=nointersection;
        return 0;
    }

    LRESULT OnDestroy()
    {
        return 0;
    }

    LRESULT OnRButtonDown(UINT, CPoint pt)
    {
        SetFocus();

        CMenu menu( ::CreatePopupMenu() );


        for (unsigned index=0; index<pairs_.size(); ++index)
        {
            menu.AppendMenu(MFT_STRING, 10+index, pairs_[index].string.c_str());
        }
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MFT_STRING, 2, _lcs("tileview-swap#Swap windows\tDblclk"));
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MF_STRING, 1, _lcs("tileview-delete#&Delete window"));
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MF_STRING|MF_GRAYED, (UINT_PTR)0, _lcs("Properties#Properties"));

        ClientToScreen(&pt);

        int res=menu.TrackPopupMenu(
                    TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
                    pt.x, pt.y, m_hWnd);

        switch (res)
        {
        case 0:
            break;
        case 1:
            DeleteFromParent();
            break;
        case 2:
            StartDrag(pt);
            break;
        default:
            static_cast<T *>(this)->OnNavigate(res-10);
        }
        return 0;
    }

    void OnFinalMessage(HWND hwnd)
    {
        delete this;
    }

    LRESULT OnSize(UINT, CSize)
    {
        SetMsgHandled(false);
        return 0;
    }

    LRESULT OnEraseBackground(HDC)
    {
        return 0;
    }

    void  PrepareToPaint(unsigned index, CDCHandle dc)
    {
        dc.SelectFont(font_);
        dc.SetTextColor(
            hilight_!=index
            ? RGB(0,0,255)
            : RGB(255,0,0));
        dc.SetBkMode(TRANSPARENT);
    }

    CSize GetItemSize(unsigned i, CDCHandle dc)
    {
        PrepareToPaint(i, dc);

        CSize sz;
        dc.GetTextExtent(STOS(pairs_[i].string), &sz);

        return sz;
    }

    CRect GetItemRect(unsigned i, CDCHandle dc)
    {
        CSize sz=GetItemSize(i, dc);
        CRect rect(CPoint(5, 5+i*sz.cy), sz);
        return rect;
    }

    LRESULT OnPaint(HDC)
    {
        CPaintDC paintdc(m_hWnd);

        CRect rc; GetClientRect(rc);

        CBitmap bitmap( ::CreateCompatibleBitmap(paintdc, rc.Width(), rc.Height()));
        CDC     dc( ::CreateCompatibleDC(paintdc) );
        dc.SelectBitmap(bitmap);

        dc.FillRect(rc, GetSysColorBrush(COLOR_3DFACE) );

        for (unsigned index=0; index<pairs_.size(); ++index)
        {
            PrepareToPaint(index, (HDC)dc);
            dc.DrawText(pairs_[index].string.c_str(), pairs_[index].string.size(),
                        GetItemRect(index, (HDC)dc),
                        DT_TOP|DT_LEFT|DT_SINGLELINE);
        }

        paintdc.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

        return 0;
    }

    unsigned GetItem(CPoint pt)
    {
        CDC dc=GetWindowDC();
        for (unsigned index=0; index<pairs_.size(); ++index)
        {
            if (GetItemRect(index, (HDC)dc).PtInRect(pt))
            {
                return index;
            }
        }
        return nointersection;
    }

    LRESULT OnLButtonDown(UINT, CPoint pt)
    {
        unsigned index=GetItem(pt);
        if (index==nointersection)
        {
            return ParentLButtonDown(0, pt);
        }

        static_cast<T *>(this)->OnNavigate(index);
        return 0;
    }

    LRESULT OnMouseMove(UINT, CPoint pt)
    {
        hilight_=GetItem(pt);
        if (hilight_!=nointersection) SetCapture();
        else                          ReleaseCapture();
        Invalidate();
        return 0;
    }

    LRESULT OnMouseLeave()
    {
        hilight_=nointersection;
        Invalidate();
        return 0;
    }

    bool OnNavigate(int)
    {
        return 0;
    }
};

//////////////////////////////////////////////////////////////////////////

struct VIEWCREATORSTRUCT
{
    HWND      (*proc)(UINT_PTR);
    UINT_PTR  value;
};

class CTVSelectView : public CTVSelectImpl<CTVSelectView>
{
public:
    DECLARE_WND_CLASS(NULL)

    bool OnNavigate(int index);
    void AppendWindowType(std::string name, CTVCreator *creator);
};

//////////////////////////////////////////////////////////////////////////

