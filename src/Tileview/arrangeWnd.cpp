#include "stdafx.h"

#include "arrangeWnd.h"

#include "TVFrame.h"
#include "geometry.h"

#include <boost/preprocessor.hpp>

#include "utilites/foreach.hpp"
#include "utilites/help.h"

//////////////////////////////////////////////////////////////////////////

const int edge=14;
const int margin=14;
const int image_size_x=120;
const int image_size_y=80;

//////////////////////////////////////////////////////////////////////////

static void mutate(Page *page, int cols, int horz)
{
    int count=page->size();
    int rows=( count + cols - 1) / cols ;
    int rowx = rows-1;

    int index=0;
    for (int row=0; row<rowx; ++row)
        for (int col=0; col < cols; ++col)
        {
            page->at(index++)->set_rect(
                !horz
                ? rect(CRect(col, row, col+1, row+1), CSize(cols, rows) )
                : rect(CRect(rows-row, col, rows-row-1, col+1), CSize(rows, cols) )
            );
        }

    int last_cols=count - cols*(rows-1);
    for (int row=rowx; row < rows; ++row)
        for (int col=0; col < last_cols; ++col)
        {
            page->at(index++)->set_rect(
                !horz
                ? rect(CRect(col, row, col+1, row+1),	CSize(last_cols, rows) )
                : rect(CRect(rows-row, col, rows-row-1, col+1),	CSize(rows, last_cols) )
            );
        }
}

class Pattern
{
    Pattern(unsigned count)
        : impl_(count)
    {
    }

    static bool compare_rects(rect const &a, rect const &b)
    {
        // тот меньше, который ближе к левому верхнему краю
        return a.top_ == b.top_
               ? a.left_ < b.left_
               : a.top_ < b.top_;
    }

    void normalize()
    {
        std::sort(STL_II(impl_), compare_rects);
    }

public:

    // default is OK

    // Pattern(Pattern const &);
    // Pattern& operator(Pattern const&);

    static Pattern create(unsigned count, int cols)
    {
        Pattern result(count);
        int rows=( count + cols - 1) / cols ;
        int rowx = rows-1;

        int index=0;
        for (int row=0; row<rowx; ++row)
            for (int col=0; col < cols; ++col)
            {
                result.impl_[index++]=
                    rect(CRect(col, row, col+1, row+1), CSize(cols, rows) );
            }

        int last_cols=count - cols*(rows-1);
        for (int row=rowx; row < rows; ++row)
            for (int col=0; col < last_cols; ++col)
            {
                result.impl_[index++]=
                    rect(CRect(col, row, col+1, row+1),	CSize(last_cols, rows) );
            }
        result.normalize();
        return result;
    }

    static Pattern rotate(Pattern const &sample)
    {
        Pattern result(sample.size());
        for (unsigned index=0; index<sample.size(); ++index)
        {
            rect s=sample.at(index);
            rect y(
                /* left  */ rect::max - s.top_,
                /* top   */ s.left_,
                /* right */ rect::max - s.bottom_,
                /* bottom*/ s.right_
            );
            result.impl_[index]=y;
        }

        result.normalize();
        return result;
    }

    size_t copy_to(rect *buffer, size_t max_size) const
    {
        assert( max_size > size() );
        std::copy(STL_II(impl_), buffer);
        return size();
    }

    size_t get_raiting() const
    {
        size_t raiting=0;
        foreach(rect const& s, impl_)
        {
            raiting+= s.left_ * (s.right_ - s.left_);
        }
        return raiting;
    }


    bool operator==(Pattern const &other) const
    {
        return impl_==other.impl_;
    }

    unsigned size() const
    {
        return impl_.size();
    }

    const rect &at(unsigned index) const
    {
        return impl_.at(index);
    }

    rect &at(unsigned index)
    {
        return impl_.at(index);
    }

private:
    std::vector<rect> impl_;
};

static bool pattern_is_ugly(const Pattern &self)
{
    bool ugly=false;
    for (unsigned index=0; index<self.size(); ++index)
    {
        rect s=self.at(index);
        unsigned width=s.right_ - s.left_;
        unsigned height=s.bottom_ - s.top_;
        if ( height * 6 < rect::max ) ugly=true;
        if ( width  * 6 < rect::max ) ugly=true;

        if ( height * 4 < width ) ugly=true;
        if ( width  * 4 < height ) ugly=true;
    }
    return ugly;
}

static bool compare_patterns(Pattern const &a, Pattern const &b)
{
    return a.get_raiting() < b.get_raiting();
}

static void remove_redundant(std::vector<Pattern> &vector)
{
    for (unsigned index=0; index < vector.size(); ++index)
    {
        vector.erase(
            std::remove(
                vector.begin()+index+1,
                vector.end(),
                vector[index]),
            vector.end());
    }

    vector.erase(
        std::remove_if(
            vector.begin(),
            vector.end(),
            pattern_is_ugly),
        vector.end()
    );

    std::sort(STL_II(vector), compare_patterns);
}

static unsigned copy_to(std::vector<Pattern> const &vector, rect *buffer, unsigned max_size)
{
    size_t count=0;
    for (unsigned index=0; index < vector.size() && max_size > vector[index].size(); ++index)
    {
        unsigned copied = vector[index].copy_to(buffer, max_size);
        max_size-=copied;
        count += copied;
        buffer+= copied;
    }

    return count;
}

static std::pair<int, rect *> GetRects(int num)
{
    static rect rects1[]=
    {
        rect(.000,.000,.999,.999),
    };

    static rect rects2[]=
    {
        rect(.000,.000,.500,.999),
        rect(.500,.000,.999,.999),

        rect(.000,.000,.999,.500),
        rect(.000,.500,.999,.999),
    };

    static rect rects3[]=
    {
        rect(.000,.000,.999,.333),
        rect(.000,.333,.999,.666),
        rect(.000,.666,.999,.999),

        rect(.000,.000,.333,.999),
        rect(.333,.000,.666,.999),
        rect(.666,.000,.999,.999),

        rect(.000,.000,.999,.500),
        rect(.000,.500,.500,.999),
        rect(.500,.500,.999,.999),

        rect(.000,.000,.500,.500),
        rect(.500,.000,.999,.500),
        rect(.000,.500,.999,.999),

        rect(.000,.000,.500,.999),
        rect(.500,.000,.999,.500),
        rect(.500,.500,.999,.999),

        rect(.000,.000,.500,.500),
        rect(.000,.500,.500,.999),
        rect(.500,.000,.999,.999),
    };

    static rect rects4[]=
    {
        rect(.000,.000,.999,.250),
        rect(.000,.250,.999,.500),
        rect(.000,.500,.999,.750),
        rect(.000,.750,.999,.999),

        rect(.000,.000,.500,.500),
        rect(.500,.000,.999,.500),
        rect(.000,.500,.500,.999),
        rect(.500,.500,.999,.999),

        rect(.000,.000,.999,.666),
        rect(.000,.666,.333,.999),
        rect(.333,.666,.666,.999),
        rect(.666,.666,.999,.999),

        rect(.000,.000,.666,.999),
        rect(.666,.000,.999,.333),
        rect(.666,.333,.999,.666),
        rect(.666,.666,.999,.999),

        rect(.000,.000,.250,.999),
        rect(.250,.000,.500,.999),
        rect(.500,.000,.750,.999),
        rect(.750,.000,.999,.999),
    };

    static rect rects5[]=
    {
        rect(.000,.000,.999,.200),
        rect(.000,.200,.999,.400),
        rect(.000,.400,.999,.600),
        rect(.000,.600,.999,.800),
        rect(.000,.800,.999,.999),

        rect(.000,.000,.500,.500),
        rect(.500,.000,.999,.500),
        rect(.000,.500,.333,.999),
        rect(.333,.500,.666,.999),
        rect(.666,.500,.999,.999),

        rect(.000,.000,.666,.500),
        rect(.000,.500,.666,.999),
        rect(.666,.000,.999,.333),
        rect(.666,.333,.999,.666),
        rect(.666,.666,.999,.999),

        rect(.000,.000,.999,.333),
        rect(.000,.333,.999,.666),
        rect(.000,.666,.333,.999),
        rect(.333,.666,.666,.999),
        rect(.666,.666,.999,.999),

        rect(.000,.000,.750,.999),
        rect(.750,.000,.999,.250),
        rect(.750,.250,.999,.500),
        rect(.750,.500,.999,.750),
        rect(.750,.750,.999,.999),

        rect(.000,.000,.999,.750),
        rect(.000,.750,.250,.999),
        rect(.250,.750,.500,.999),
        rect(.500,.750,.750,.999),
        rect(.750,.750,.999,.999),
    };

    static rect rects6[]=
    {
        rect(.000,.000,.750,.999),
        rect(.750,.000,.999,.200),
        rect(.750,.200,.999,.400),
        rect(.750,.400,.999,.600),
        rect(.750,.600,.999,.800),
        rect(.750,.800,.999,.999),

        rect(.000,.000,.666,.500),
        rect(.000,.500,.666,.999),
        rect(.666,.000,.999,.250),
        rect(.666,.250,.999,.500),
        rect(.666,.500,.999,.750),
        rect(.666,.750,.999,.999),

        rect(.000,.000,.666,.333),
        rect(.000,.333,.666,.666),
        rect(.000,.666,.666,.999),
        rect(.666,.000,.999,.333),
        rect(.666,.333,.999,.666),
        rect(.666,.666,.999,.999),

        rect(.000,.000,.999,.333),
        rect(.000,.333,.999,.666),
        rect(.000,.666,.250,.999),
        rect(.250,.666,.500,.999),
        rect(.500,.666,.750,.999),
        rect(.750,.666,.999,.999),

        rect(.000,.000,.999,.250),
        rect(.000,.250,.999,.500),
        rect(.000,.500,.999,.750),
        rect(.000,.750,.333,.999),
        rect(.333,.750,.666,.999),
        rect(.666,.750,.999,.999),

        rect(.000,.000,.999,.250),
        rect(.000,.250,.999,.500),
        rect(.000,.500,.500,.750),
        rect(.500,.500,.999,.750),
        rect(.000,.750,.500,.999),
        rect(.500,.750,.999,.999),

        rect(.000,.000,.333,.500),
        rect(.333,.000,.666,.500),
        rect(.666,.000,.999,.500),
        rect(.000,.500,.333,.999),
        rect(.333,.500,.666,.999),
        rect(.666,.500,.999,.999),
    };

    // если есть заранее заданные
//	switch(num)
//	{
//	#define DECL(skip1, num, skip2) \
//		case num: \
//			return std::make_pair(SIZEOF( BOOST_PP_CAT(rects, num) )/num, (rect*)BOOST_PP_CAT(rects, num));
//
//	BOOST_PP_REPEAT_FROM_TO(2, 7, DECL, skip);
//	#undef DECL
//	};

    const int max_scheme=32*32;

    // массив, в который поместиться все
    static rect rectsx[max_scheme];

    std::vector<Pattern> array;
    if (num < 3)
    {
        array.push_back( Pattern::create(num, 1) );
    }
    else for (int index=1; index<num; ++index)
        {
            array.push_back( Pattern::create(num, index) );
        }

    unsigned count=array.size();
    for (unsigned index=0; index<count; ++index)
    {
        array.push_back( Pattern::rotate(array[index]) );
        array.push_back( Pattern::rotate(array.back()) );
        array.push_back( Pattern::rotate(array.back()) );
    }

    remove_redundant( array );
    if (array.empty())
    {
        Page page;
        for (int index=0; index<num; ++index)
            page.append_new_window();

        page.rearrange_windows();

        Pattern pt=Pattern::create(num, 1);
        for (unsigned index=0; index < page.size(); ++index)
            pt.at(index) = page.at(index)->get_rect();
        array.push_back( pt );
    }

    unsigned size = copy_to(array, rectsx, SIZEOF(rectsx));

    return std::make_pair(size/num, (rect *)rectsx);
}

//////////////////////////////////////////////////////////////////////////

CArrangeWnd::CArrangeWnd(CTileViewFrame *frame, bool inc)
    : window_count_(frame->page_.size()+inc),
      frame_(frame),
      base_(&frame->page_),
      rects_(0),
      pattern_count_(0),
      increase_(inc)
{
}

LRESULT CArrangeWnd::OnCreate(LPCREATESTRUCT cs)
{
    PostMessage(AM_SETNUMBER, window_count_);
    return 0;
}

void CArrangeWnd::DrawImage(CDCHandle dc, CPoint pt, int image)
{
    for (int index=0; index<window_count_; ++index)
    {
        rect rc=rects_[image*window_count_+index];
        CRect tr=rc.transform(CRect(pt, CSize(image_size_x, image_size_y)));
        dc.Rectangle(tr);
    }
}

LRESULT CArrangeWnd::OnSetNumber(UINT, LPARAM lParam, WPARAM)
{
    tie(pattern_count_, rects_)=GetRects(window_count_=lParam);

    int width = pattern_count_ > 1 ? (int)sqrt((double)pattern_count_)+1 : 1;
    int height = ( pattern_count_ + width - 1 ) / width ;

    CRect client=CRect(0, 0,
                       edge+width*(image_size_x+margin),
                       edge+height*(image_size_y+margin)
                      );
    AdjustWindowRectEx(client, GetStyle(), false, 0);
    MoveWindow(client);

    CenterWindow();
    return 0;
}


CRect CArrangeWnd::GetImageRect(int pattern)
{
    CRect rc; GetClientRect(rc);

    int width = rc.Width() / ( image_size_x+margin );
    int row = width ? pattern / width : 0;
    int col = pattern - row * width;

    return CRect(
               CPoint(edge+col*(image_size_x+margin), edge+row*(image_size_y+margin)),
               CSize(image_size_x, image_size_y));
}


LRESULT CArrangeWnd::OnEraseBkGnd(HDC)
{
    return 0;
}

LRESULT CArrangeWnd::OnPaint(HDC)
{
    CPaintDC padc(m_hWnd);
    CRect rc; GetClientRect(rc);

    // создаем memory dc и потготавливаем ее

    CDC dc( ::CreateCompatibleDC(padc) );

    CBitmap bmp; bmp.CreateCompatibleBitmap( padc, rc.Width(), rc.Height() );
    dc.SelectBitmap(bmp);

    dc.FillRect(rc, GetSysColorBrush( COLOR_3DFACE) );

    CPoint mouse; GetCursorPos(&mouse);
    ScreenToClient(&mouse);

    CBrush brsh( CreateSolidBrush( 0xFFFFFF ) );
    dc.SelectBrush(brsh);

    CPen pen( CreatePen(PS_SOLID, 0, 0) );
    CPenHandle oldp=dc.SelectPen(pen);

    // проходимся по картинкам и
    for (int pattern=0; pattern<pattern_count_; ++pattern)
    {
        if (GetImageRect(pattern).PtInRect(mouse))
        {
            pen = ::CreatePen(PS_SOLID, 0, RGB(255,0,0));
        }
        else
        {
            pen = ::CreatePen(PS_SOLID, 0, RGB(0,0,0));
        }
        dc.SelectPen( pen );
        DrawImage((HDC)dc, GetImageRect(pattern).TopLeft(), pattern);
    }

    dc.SelectPen(oldp);

    padc.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

    return 0;
}

LRESULT CArrangeWnd::OnMouseMove(UINT, CPoint)
{
    Invalidate();
    return 0;
}

LRESULT CArrangeWnd::OnLButtonDown(UINT, CPoint point)
{
    for (int pattern=0; pattern<pattern_count_; ++pattern)
    {
        if (GetImageRect(pattern).PtInRect(point))
        {
            if (increase_)
            {
                frame_->AppendWindows(1);
            }

            base_->sort_windows();

            rect *rects = rects_ + pattern*window_count_;
            for (int k=0; k<window_count_; ++k)
            {
                rect temp=base_->at(k)->get_rect();
                base_->at(k)->set_rect(rects[k]);
            }
            base_->check_validation();
            ::SendMessage(*frame_, TVFM_UPDATELAYOUT, 0, 0);
            PostMessage(WM_CLOSE, 0, 0);

            // чтобы нельзя было два раза подряд
            // выбрать схему
            pattern_count_=-1;
        }
    }

    return 0;
}

void CArrangeWnd::OnFinalMessage(HWND)
{
    delete this;
}

LRESULT CArrangeWnd::OnKeyDown(TCHAR cr, UINT, UINT)
{
    if (cr == VK_ESCAPE)
    {
        PostMessage(WM_CLOSE, 0, 0);
    }
    return 0;
}

LRESULT CArrangeWnd::OnKillFocus(HWND)
{
    PostMessage(WM_CLOSE, 0, 0);
    return 0;
}

LRESULT CArrangeWnd::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "arrange_windows");
    return 0;
}
