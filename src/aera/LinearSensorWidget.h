#pragma once

#include "data/LinearSetup.h"

class CLinearSensorWidget: public CWindowImpl<CLinearSensorWidget>{
public:

    BEGIN_MSG_MAP_EX(CLinearSensorWidget)
        MSG_WM_PAINT(OnPaint);
    END_MSG_MAP()

    CLinearSensorWidget();

private:

    LRESULT OnPaint(HDC);

public:

    void setLocation(location::LinearSetup*);

private:

    std::pair<double, double> GetRange();

private:

    location::LinearSetup*    loc_;

    std::map<int, double>	sensors_;
    double                  circular_;
};
