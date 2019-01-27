#include "stdafx.h"

#include "LinearCollector.h"
#include "LinearSetup.h"
#include "LocationUnit.h"

namespace location
{

//////////////////////////////////////////////////////////////////////////

LinearCollector::LinearCollector(LinearSetup const &setup)
    : ZonalCollector( setup )
{
    work_deviation_ = setup.get_deviation().get_time( work_velocity_ );
    work_circulation_ = setup.get_circular_length().get_distance(work_velocity_);

    //////////////////////////////////////////////////////////////////////////

    work_channels_.clear();
    typedef std::pair<int, double> value_type;
    std::map<int, double> channels=setup.get_sensors();

    foreach(value_type pair, channels)
    {
        work_channels_[pair.first]=
                unit_value(pair.second, setup.get_unit())
                .get_distance( work_velocity_ );
    }

    unit_=setup.get_unit();
}

namespace
{
bool hit_in_list(const index_t &index, const std::map<int, double> &map)
{
    return map.count(static_cast<int>(index.channel) )!=0;
}
}

bool LinearCollector::calculate_coords(double* &result)
{
    // ����������� ���� ��� �������, ������� �� ������ � �����
    std::vector<index_t>::iterator it =
            std::stable_partition(STL_II(package_),
                           bind(hit_in_list, _1, boost::cref(work_channels_)));

    // ����, ������� �� ������ � ������ ������������
    ignore_.insert(ignore_.end(), it, package_.end());
    package_.erase(it, package_.end());

    // ����� ������������ ��� ������������ �������
    if (package_.size()<2)
        return false;


    // ��������� �� �������
    if (use_peak_time_
            ? package_[1].peak < package_[0].peak
            : package_[1].time < package_[0].time )
    {
        std::swap(package_[0], package_[1]);
    }

    // ���������� ������, �� ������� ������ �������
    const int cha=static_cast<int>(package_[0].channel);
    const int chb=static_cast<int>(package_[1].channel);

    // ���������� ���
    const double dt=use_peak_time_
                    ? (package_[1].peak - package_[0].peak)
                    : (package_[1].time - package_[0].time);

    // ���������� �������
    double xa=work_channels_[cha];
    double xb=work_channels_[chb];

    double distance = fabs(xa - xb);
    if (work_circulation_)
    {
        // ���� ���������� ������ �������� ����� ����������,
        // �� "������������
        if (distance > work_circulation_ / 2)
        {
            if (xa < xb) xa += work_circulation_;
            else xb += work_circulation_;
        }
    }

    // �������� ����� ��������
    const double mediana=(xa + xb)/2;

    // �������� ���������� ����� ��������
    const double hdistance=distance/2;

    // ��������� ��������� ��������� �� �� ������ �������
    double x=dt*work_velocity_/2;

    // ���� ���������� ������� �� ������� ������� � ������ �����������
    // �� ���������� ��� �������
    if (x > hdistance+work_deviation_)
        return false;

    // ���� ������� �� �������, �� �� ��������� �����������, �� ������������
    // ���������� � ���������� �������
    if (x > hdistance)
        x=hdistance;

    double *data=(double *)allocate(sizeof(double));

    // ������ ���������� x-coord
    double sx = xa>xb ? (mediana + x) : (mediana - x);
    if (work_circulation_)
        sx = fmod2(sx, work_circulation_);

    data[0]= sx;

    if (unit_!=m)
    {
        data[0] = unit_value(data[0], m)
                .to_unit(unit_, work_velocity_);
    }

    // ������� ����������
	minmax_map_.add_value(aera::C_CoordX, *data);

    result = data;
    return true;
}

void LinearCollector::create_minmax_map()
{
    // ������� ��������� ��������
    typedef std::pair<int, double> value_type;
    foreach(value_type pair, work_channels_)
    {
        double x = unit_value(pair.second, m)
                .to_unit(unit_, work_velocity_);

        minmax_map_.add_value(aera::C_CoordX, x);
    }
}

std::vector<aera::chars>           LinearCollector::get_chars() const
{
    std::vector<aera::chars> temp;
    temp.push_back( aera::C_CoordX );
    return temp;
}

std::map<aera::chars, std::string> LinearCollector::get_units() const
{
    std::map<aera::chars, std::string> temp;
    temp[ aera::C_CoordX ] = location::get_short_name( unit_ );
    return temp;
}


}
