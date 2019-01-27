#pragma once

#include "PlotBaseWnd.h"
#include "data/monpac.h"

class CMonpacWindow
    : public CPlotWndBase
{

    enum { UM_INVALIDATEFRAME =WM_USER+1 };

    typedef CPlotWndBase baseClass;
    BEGIN_MSG_MAP_EX(CPlotView)

    MSG_WM_CREATE(OnCreate);

    MSG_WM_MOUSEMOVE(OnMouseMove);
    MSG_WM_MOUSELEAVE(OnMouseLeave);

    CHAIN_MSG_MAP(baseClass);
    END_MSG_MAP();

public:

    CMonpacWindow();

    void SetMonpac(shared_ptr<class monpac::monpac_result>);

private:

    LRESULT	OnCreate(LPCREATESTRUCT);
    LRESULT OnMouseMove(UINT, CPoint);
    LRESULT OnMouseLeave();

    //
    void DrawLines(CDCHandle);
    void MakeChannelRects(CDCHandle dc, std::vector<std::pair<int, CRect> > &rects);
    std::string GetChannelsUnderCursor();

    virtual void DrawContents(CDCHandle dc, CRect frame);
    virtual void OnFinalMessage(HWND);

private:

    std::string   tip_;
    CToolTipCtrl	tooltip_;
    shared_ptr<monpac::monpac_result>    result_;
};
