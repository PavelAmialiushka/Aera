#pragma once

#include <atlctrls.h>
#include <atlcrack.h>

using namespace WTL;

template<class T>
class CToolTipWindow
{
public:
    BEGIN_MSG_MAP(CToolTipWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
    END_MSG_MAP()


    void RemoveAllToolTipRegions()
    {
        regions_.clear();
    }

    void AddToolTipRegion(CRect rect, std::string text)
    {
        region t;
        t.rect = rect;
        t.text = text;
        regions_.push_back(t);
    }

    void Deactivate()
    {
        T* pT = static_cast<T*>(this);
        tooltipcontrol_.TrackActivate(CToolInfo(0, pT->m_hWnd, 0), false);
        isactive_ = false;
    }

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        bHandled = false;
        T* pT = static_cast<T*>(this);
        ATLASSERT(::IsWindow(pT->m_hWnd));

        tooltipcontrol_.Create(pT->m_hWnd);
        tooltipcontrol_.AddTool(CToolInfo(TTF_TRACK|TTF_ABSOLUTE|TTF_TRANSPARENT,pT->m_hWnd));
        tooltipcontrol_.SetMaxTipWidth(400);

        isactive_ = false;
        return 0;
    }

    LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
    {
        T* pT = static_cast<T*>(this);
        bHandled = false;

        Deactivate();

        return 0;
    }

    LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
    {
        T* pT = static_cast<T*>(this);
        bHandled = false;

        CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        std::string text;
        bool should_show = false;

        for(unsigned index=0; index<regions_.size(); ++index)
        {
            if (regions_[index].rect.PtInRect(pt))
            {
                should_show = true;
                if (text.size()) text += "\n\n";
                text += regions_[index].text;
            }
        }

        if (should_show)
        {
            if (text != text_)
            {
                tooltipcontrol_.UpdateTipText(CToolInfo(0, pT->m_hWnd, 0, 0, (char *)text.c_str()));
                text_ = text;
            }

            pT->ClientToScreen(&pt);
            tooltipcontrol_.TrackPosition(pt.x+16, pt.y+16);
            if (!isactive_)            
            {
                tooltipcontrol_.TrackActivate(CToolInfo(0, pT->m_hWnd, 0), true);
            }
            isactive_ = should_show;
        } else if (isactive_)
        {
            Deactivate();
        }

        return 0;
    }

private:
    struct region
    {
        std::string text;
        CRect       rect;
    };

    CToolTipCtrl tooltipcontrol_;
    std::string  text_;
    bool         isactive_;
    std::vector<region> regions_;


};
