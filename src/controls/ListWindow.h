#pragma once

#include "data/traits.h"
#include "tileview/TVView.h"

#include "ListMaker.h"
#include "ListModel.h"

//////////////////////////////////////////////////////////////////////////

class CListWindow
    : public CTileViewImpl<CListWindow>
    , public process::host_listener
{
public:
    DEFINE_ERROR(fault, "ListWnd: unexpected");
    DEFINE_ERROR(movecursor_failure, "moving cursor error");

    typedef CTileViewImpl<CListWindow> baseClass;
    BEGIN_MSG_MAP_EX(CListWindow)

    MSG_WM_SIZE(OnSize);

    MSG_WM_LBUTTONDOWN(OnLButtonDown);

    MSG_WM_LBUTTONUP(OnLButtonUp);
    MSG_WM_LBUTTONDBLCLK(OnLButtonDbl);

    MSG_WM_MBUTTONDOWN(OnMButtonDown);
    MSG_WM_CONTEXTMENU(OnRButtonDown);

    MSG_WM_MOUSEWHEEL(OnMouseWheel);

    MSG_WM_MOUSEMOVE(OnMouseMove);
    MSG_WM_KEYDOWN(OnKeyDown);
    MSG_WM_CHAR(OnChar);
    MSG_WM_HSCROLL(OnHScroll);
    MSG_WM_VSCROLL(OnVScroll);
    MSG_WM_SETFOCUS(OnSetFocus);
    MSG_WM_KILLFOCUS(OnLostFocus);
    MSG_WM_TIMER(OnTimer);
    MSG_WM_COMMAND(OnCommand);

    MSG_WM_PAINT(OnPaint);
    MSG_WM_ERASEBKGND(OnEraseBackground);

    MESSAGE_HANDLER_EX(TVVM_COPYCLIP, OnCopyClip);

    CHAIN_MSG_MAP(baseClass);
    END_MSG_MAP()

    CListWindow();
    ~CListWindow();

    void update()
    {
        Invalidate();
    }

    // CONTROLLER

    LRESULT OnSize(UINT, CSize);
    LRESULT OnLButtonDown(UINT, CPoint);
    LRESULT OnLButtonUp(UINT, CPoint);
    LRESULT OnLButtonDbl(UINT, CPoint pt);
    LRESULT OnMButtonDown(UINT, CPoint);
    LRESULT OnRButtonDown(HWND, CPoint);
    LRESULT OnMouseMove(UINT, CPoint);
    LRESULT OnMouseWheel(UINT, short, CPoint);
    LRESULT OnKeyDown(TCHAR, UINT, UINT);
    LRESULT OnChar(TCHAR, UINT, UINT);
    LRESULT OnHScroll(int, short, HWND);
    LRESULT OnVScroll(int, short, HWND);
    LRESULT OnSetFocus(HWND);
    LRESULT OnLostFocus(HWND);
    LRESULT OnTimer(UINT);
    LRESULT OnCommand(UINT, int, HWND);
    LRESULT OnCopyClip(UINT, WPARAM, LPARAM);

    virtual void OnFinalMessage(HWND);

    int get_line() const; // current line at page
    int get_pos() const; // current line from beginning
    int get_wpos() const;
    int get_page() const;
    int get_text_width() const;

    int get_line_at_point(CPoint point) const;

    int lines() const; // lines at page
    const CRect &get_rect() const;

    int get_max_wpos() const;
    int effwidth() const;
    int effheight() const;
    int get_text_char_height() const;
    int get_text_char_width() const;

    bool has_focus() const;
    int get_text_length() const;
    // modifiers

    void update_scrollbars();
    void check_borders();

    void move_cursor(int pos);
    void set_vscroll_pos();
    void set_hscroll_pos();

    void center_on(int pos);
    void move_page(int pos);
    void move_hor(int i);

    void set_width(int y);
    void set_rect(const CRect &r);
    void set_text_extent(CSize te);

    void set_text_length(int x);

    void set_promptstring(std::string s);
    std::string get_promptstring() const;

    void locate_time();

    void serialization(serl::archiver &);

public:

    LRESULT OnEraseBackground(HDC);
    LRESULT OnPaint(HDC);
    LRESULT OnPaint(HDC, CRect, bool copyclip = false);

protected:

    void   safe_on_finish(process::prslt);
    void   safe_on_restart();

    void   on_pos_changed();

private:

    void CopyClipboard();
    void CopyPage();
    void CopyImage();

protected:
    int timerid_;

    // view
private:
    list_model  model_;

    std::string promptstring_;

    // время на которое устанавливается курсор.
    // или -1, если все равно
    double		  preffered_time_;

    CSize text_extent_; // размеры одного символа
    CRect rect_;
    int text_length_;

    int pageline_;      // номер первой на экране строки
    int lineno_;        // выделенной строки от начала экрана
    int wide_;          // виртуальная ширина экрана
    int wpos_;          // сдвиг экрана от начала
};
