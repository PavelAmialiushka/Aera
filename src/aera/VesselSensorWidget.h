#pragma

#include "data/VesselSetup.h"

using namespace location;

class CVesselSensorWidget
        : public CWindowImpl<CVesselSensorWidget>
{
public:

    BEGIN_MSG_MAP_EX(CVesselSensorWidget)
        MSG_WM_PAINT(OnPaint);
    END_MSG_MAP()

    CVesselSensorWidget();

private:

    LRESULT OnPaint(HDC);

public:
    void setLocation(VesselSetup*);

private:
    VesselSetup*    loc_;
    VesselStructure formula_;
    VesselSensors	sensors_;
};
