#include "stdafx.h"

#include "TVView.h"

#include "utilites/serl/Archive.h"
serl::mark_as_rtti< CTVCreatorImpl<CTVSelectView> > rtto_;

bool CTVSelectView::OnNavigate(int index)
{
    pair pr=pairs_.at(index);
    pr.maker->create();
    HWND hwnd=pr.maker->activate(GetParent());
    ReplaceChild(hwnd);
    return 0;
}

void CTVSelectView::AppendWindowType(std::string name, CTVCreator *creator)
{
    pair pr;
    pr.string=name;
    pr.maker=shared_ptr<CTVCreator>(creator);
    pairs_.push_back(pr);
}
