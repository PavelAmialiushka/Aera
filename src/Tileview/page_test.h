#pragma  once

#include "page.h"
#include "utilites/unittest.h"

#include <boost/format.hpp>

class UNITTEST_SUITE(Page_test)
{
public:
    enum {m=point::max};

    static Page *create_page(rect *begin, rect *end)
    {
        Page *page=new Page;
        for (unsigned index=0; begin!=end; ++index, ++begin)
        {
            page->push_back(new Window(0, index, *begin));
        }
        return page;
    }

    static Page *create_page()
    {
        Page *page=new Page;
        page->push_back(new Window(0, 0, rect(.000,.000,.500,.500)));
        page->push_back(new Window(0, 1, rect(.500,.000,.999,.500)));
        page->push_back(new Window(0, 2, rect(.000,.500,.333,.999)));
        page->push_back(new Window(0, 3, rect(.333,.500,.666,.999)));
        page->push_back(new Window(0, 4, rect(.666,.500,.999,.999)));
        return page;
    }

    static Page *create_page12()
    {
        return create_page(12);
    }

    static Page *create_page(int sz)
    {
        Page *page = new Page;
        for (int i=0; i<sz; ++i) page->append_new_window();
        page->rearrange_windows();
        return page;
    }

    static Page *create_propeller()
    {
        Page *page=create_page(5);
        page->at(0)->set_rect(rect(.000,.000,.750,.250));
        page->at(1)->set_rect(rect(.750,.000,.999,.500));
        page->at(2)->set_rect(rect(.000,.250,.500,.999));
        page->at(3)->set_rect(rect(.500,.250,.750,.500));
        page->at(4)->set_rect(rect(.500,.500,.999,.999));
        return page;
    }

    static void assert_equal(Page *page,rect *begin,rect *end,const debug::SourcePos &loc)
    {
        std::ostringstream tmp; tmp << '\n';
        int fails=0;
        if (end-begin!=(int)page->size())
        {
            tmp << boost::format("Incorrect size. Expected %2d. Actual %2d\n")
                % (end-begin)
                % page->size()
                ;
            fails+=1;
        }

        int counter=page->size();
        for (rect *index=begin; index<end && counter>0; ++index, --counter)
        {
            rect test_rect=page->at(index-begin)->get_rect();
            int ok= (*index==test_rect);
            tmp << boost::format("#%2d value: %15s; not equal to: %15s %s\n")
                %(index-begin)
                %*index
                %test_rect
                % (ok?"ok":"fail");
            fails+=!ok;
        }
        if (fails)
        {
            failAt(tmp.str(), loc.get_string());
        }
    }
};
