#pragma once

#include "data/PlanarSetup.h"

using namespace location;

class CPlanarSensorWidget
        : public CWindowImpl<CPlanarSensorWidget>
{
public:

    BEGIN_MSG_MAP_EX(CPlanarSensorWidget)
        MSG_WM_PAINT(OnPaint);
    END_MSG_MAP()

    CPlanarSensorWidget();

private:

    LRESULT OnPaint(HDC);

public:
    void setLocation(PlanarSetup*);

private:
    PlanarSetup*    loc_;

    std::map<int, planar_coords>	sensors_;
    double horizontal_wrap_;
    double vertical_wrap_;
};
