#pragma once

#include "controls/plot/range.h"
#include "plottypes.h"
#include "tileview/TVView.h"

class CPlotWndBase
    : public CTileViewImpl<CPlotWndBase>
{
public:
    typedef CTileViewImpl<CPlotWndBase> baseClass;
    BEGIN_MSG_MAP_EX(CPlotWndBase)
    MESSAGE_HANDLER_EX(WM_SIZE, OnSizeChanged);
    MESSAGE_HANDLER_EX(WM_WINDOWPOSCHANGED, OnSizeChanged);
    MESSAGE_HANDLER_EX(WM_SHOWWINDOW, OnSizeChanged);

    MSG_WM_PAINT(OnPaint);
    MSG_WM_ERASEBKGND(OnEraseBackground);

    MSG_WM_GETDLGCODE(OnGetDlgCode);

    MSG_WM_MOUSEMOVE(OnMouseMove);

    MESSAGE_HANDLER_EX(TVVM_COPYCLIP, OnCopyClip);

    CHAIN_MSG_MAP(baseClass);
    END_MSG_MAP()

    CPlotWndBase();

    virtual void serialization(serl::archiver &)
    {
    }

    static CRect MakeOutframe(CRect);

protected:

    void  CalculateFrame(CDCHandle);

    CRect MakeFrameFromRect(CDCHandle, CRect);

    CRect get_outframe() const;
    CRect get_frame() const;

    int map_to(bool isyaxe, double value) const;
    double unmap_double(bool isyaxe, int x) const;
    std::string unmap(bool isyaxe, int x) const;

    void OnPaint(CDCHandle dc, CRect rc);

protected:

    virtual void OnSizeChanged();
    virtual void DrawContents(CDCHandle dc, CRect frame);

private:

    void DrawMarkLeft(CDCHandle dc, CRect frame, int y, std::string mark);
    void DrawMarkDown(CDCHandle dc, CRect frame, int x, std::string mark);

    void DrawMarkLeft(CDCHandle dc, int y);
    void DrawMarkDown(CDCHandle dc, int x);

    void DrawFrame(CDCHandle, CRect frame);
    void DrawTitles(CDCHandle dc, CRect frame);

private:

    LRESULT OnSizeChanged(UINT, WPARAM, LPARAM);

    LRESULT OnGetDlgCode(LPMSG);

    LRESULT OnPaint(HDC hdc);
    LRESULT OnEraseBackground(HDC);

    LRESULT OnMouseMove(UINT, CPoint);

    LRESULT OnCopyClip(UINT, WPARAM, LPARAM);

    virtual void SetCaption() {};

protected:

    CRect frame_;
    CRect outframe_;
    CSize framesize_;

    bool  copyclip_;
    bool  track_values_;
    bool	show_axes_captions_;

    std::string caption_;
    std::string xcaption_;
    std::string ycaption_;

    axe_info    xaxe_;
    axe_info    yaxe_;

    plot::range xrange_;
    plot::range yrange_;
};
