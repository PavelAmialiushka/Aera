#pragma once

//////////////////////////////////////////////////////////////////////////

#include "geometry.h"
#include "wgeometry.h"

#include "operators.h"
#include "TVView.h"

//////////////////////////////////////////////////////////////////////////

enum _TVN
{
    TVFN_LAST = 0u-100u,
    TVFN_FIRST = 0u-200u,
    TVFN_WINDOWADDED,
    TVFN_WINDOWREMOVED,
    TVFN_WINDOWRESIZED,
};

struct TVFNMHDR : NMHDR
{
    TVFNMHDR(HWND hwnd, UINT c)
    {
        hwndFrom=hwnd;
        idFrom=GetWindowLong(hwnd, GWL_ID);
        code=c;
    };

    LRESULT SendMessage()
    {
        return ::SendMessage(
                   GetParent(hwndFrom),
                   WM_NOTIFY,
                   (WPARAM)idFrom,
                   (LPARAM)this);
    }
};

struct CBmp
{
    CBmp()
        : bitmap_(new CBitmap())
    {
    }

    CBmp(HBITMAP hbmp, CSize sz=CSize())
        : bitmap_(new CBitmap(hbmp)),
          size_(sz)
    {
    }

    CBmp(const CBmp &bmp)
        : bitmap_(bmp.bitmap_),
          size_(bmp.size_)
    {}

    CBmp &operator=(CBmp bmp)
    {
        swap(bmp);
        return *this;
    }

    void swap(CBmp &bmp)
    {
        bitmap_.swap(bmp.bitmap_);

        std::swap(size_, bmp.size_);
    }

    operator CBitmapHandle() const
    {
        return CBitmapHandle( *bitmap_ );
    }

    CSize GetSize() const
    {
        return size_;
    }

private:
    shared_ptr<CBitmap> bitmap_;
    CSize               size_;
};

/////////////////////////////////////////////////////////////////////////

void ShowCopyClipDlg(HWND window);

class CTileViewFrame
    : public CWindowImpl<CTileViewFrame>
{
public:
    DECLARE_WND_CLASS(NULL)

    BOOL PreTranslateMessage(MSG *pMsg);

    BEGIN_MSG_MAP_EX(CTileViewView)
    MESSAGE_HANDLER(WM_PAINT, OnPaint);

    MSG_WM_CREATE(OnCreate);
    MSG_WM_DESTROY(OnDestroy);
    MSG_WM_SIZE(OnSize);

    MESSAGE_HANDLER_EX(WM_SHOWWINDOW, OnShowWindow);
    MESSAGE_HANDLER_EX(WM_WINDOWPOSCHANGED, OnShowWindow);

    MSG_WM_SETFOCUS(OnSetFocus);
    MSG_WM_KILLFOCUS(OnFocus);

    MSG_WM_ERASEBKGND(OnEraseBackground);

    MSG_WM_MOUSEMOVE(OnMouseMove);
    MSG_WM_LBUTTONDOWN(OnLButtonDown);
    MSG_WM_LBUTTONUP(OnLButtonUp);
    MSG_WM_MBUTTONDOWN(OnMButtonDown);

    MSG_WM_SETCURSOR(OnSetCursor);

    MESSAGE_HANDLER(TVFM_CHILDLBUTTONDOWN, OnChildLButtonDown);
    MESSAGE_HANDLER(TVFM_DELETECHILD,      OnDeleteChild);
    MESSAGE_HANDLER(TVFM_UPDATELAYOUT,		 OnUpdateLayout);
    MESSAGE_HANDLER(TVFM_REPLACECHILD,     OnReplaceChild);
    MESSAGE_HANDLER(TVFM_ARRANGEWINDOWS,   OnArrangeWindows);
    MESSAGE_HANDLER(TVFM_STARTDRAG,				 OnStartDrag);
    MESSAGE_HANDLER(TVFM_ISINDRAGMODE,		 OnIsInDragMode);

    MESSAGE_HANDLER_EX(TVVM_COPYCLIP, OnCopyClip);

    COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen);
    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE, OnFileSave);

    REFLECT_NOTIFICATIONS();
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    CTileViewFrame(bool autodelete=false);

private:

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnDestroy();

    LRESULT OnSize(UINT, CSize);

    LRESULT OnShowWindow(UINT, WPARAM, LPARAM);

    LRESULT OnSetFocus(HWND);

    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);

    LRESULT OnFocus(HWND);
    LRESULT OnEraseBackground(HDC);

    LRESULT OnSetCursor(HWND=0, UINT=0, UINT=0);

    LRESULT OnMouseMove(UINT, CPoint);
    LRESULT OnLButtonDown(UINT, CPoint);
    LRESULT OnLButtonUp(UINT, CPoint);
    LRESULT OnMButtonDown(UINT, CPoint);

    LRESULT OnChildLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnDeleteChild(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnReplaceChild(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnUpdateLayout(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnStartDrag(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnIsInDragMode(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT SetFocusNext(UINT, WPARAM, LPARAM, BOOL &);
    LRESULT OnArrangeWindows(UINT, WPARAM, LPARAM, BOOL &);

    LRESULT OnCopyClip(UINT, WPARAM, LPARAM);

public:

    void    ReceiveSubWindows(std::vector<pwindow_t> &);
    void    UpdateLayout();

    void    HideAll();
    void    ShowAll();
    void    FocusNext();

    CRect   GetChildRect(pwindow_t) const;

    void    AppendWindows(int);

    void    SetCreateWindowCallback(boost::function1<HWND, HWND> func);

    void    serialization(serl::archiver &ar);
    LRESULT OnFileOpen(UINT, int, HWND);
    LRESULT OnFileSave(UINT, int, HWND);

    void    OnFinalMessage(HWND);
    void    ShowArrangeWindow();
    void    AddArrangeWindow();

private:

    HWND     CreateNewWindow();

private:
    boost::function1<HWND, HWND> do_create_window_;
    std::string									 serialization_buffer_;

    CToolTipCtrl    tooltip_;
    TOOLINFO        tool_;

    //! Autodeletes CTVFrame object in ONFinalMessage handler.
    bool    autodelete_;

    //! Hides subwindows while moving splitter
    //!	to increase perfomance
    bool		hide_while_moving_;

    bool		dragging_;
    pwindow_t drag_window_;
    CImageList drag_list_;
    CPoint    drag_start_point_;

    // painting
    std::map<int, CBmp> bitmaps_;

    //! member to perform resize operation
    shared_ptr<XSlideLine>     mobil_zone_;

    //! member to perform insert operation
    shared_ptr<Inserter>      inserter_;

    //! members for select operations
    bool  settingselection_;
    point selectionpoint_;
    shared_ptr<SelectionRect> selection_;

    //! window container
    Page        page_;

    //! subwindow having keyboard focus
    pwindow_t   focus_;

    friend class CArrangeWnd;
};