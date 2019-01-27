#include "stdafx.h"

#include "TVCreator.h"

#include "utilites/serl/Archive.h"

CTVCreator::CTVCreator()
{
    debug::Assert<fault>(0, HERE);
}

CTVCreator::CTVCreator(make_object_t o, make_window_t w, make_serial_t s)
    : object_(0),
      make_object_(o),
      make_window_(w),
      make_serial_(s)
{}

CTVCreator::~CTVCreator() {}

void CTVCreator::create()
{
    object_=make_object_();
}

HWND CTVCreator::activate(HWND hwnd)
{
    return make_window_(object_, hwnd);
}

void CTVCreator::serialization(serl::archiver &ar)
{
    bool existance=object_;
    ar.serial("exists", existance);
    if (existance)
    {
        if (!object_) create();
        make_serial_(object_, ar);
    }
}
