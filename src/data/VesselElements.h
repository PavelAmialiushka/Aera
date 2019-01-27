#pragma once

#include "finterpol.h"
#include "utilites/strings.h"

class vessel_test;

namespace location
{

class VesselContext;

struct VesselElement;
typedef shared_ptr<VesselElement> PVElement;

////////////////////////////////////////////////////////////////////////////////////

struct VesselElement
{
    // ������ ��������
    double height_z;

    // ������ �������� �� �����������
    double height_y;

    // ����������, ���������������� �������� ��������
    // ������������ ������
    double  bottom_z;
    double  bottom_y;
    int     index;

    VesselElement(double h);
    virtual ~VesselElement(){}

public:
    static PVElement make_cyl(double h, double d);
    static PVElement make_cone(double h, double lower_d, double higher_d);
    static PVElement make_sphere(double lower_d, double higher_d);
    static PVElement make_ellipsoid(double h, double lower_d, double higher_d);
    static PVElement make_ellipsoid(double lower_d, double higher_d);
    static PVElement make_flat(double d, double d2);
public:

    double top_y() const;
    double get_width() const;

public:
    // ������ ������ �������� � ����� `z`
    virtual double get_radius_by_lz(double z) const = 0;
    virtual double get_radius_by_ly(double y) const;

    // ������� ���������
    virtual double z2y(double z) const=0;
    virtual double y2z(double y) const=0;

    // ������ ������� ������������� �����
    virtual bool geodesic_step(double ds, VesselContext& context) const=0;

};

////////////////////////////////////////////////////////////////////////////////////
/// \brief �������
///

struct VesselElementCyl
        : public VesselElement
{
    double diameter;

private:
    double calc_height_y() const;

public:
    VesselElementCyl(double h, double d);

public:
    virtual double get_radius_by_lz(double z) const;
    virtual double z2y(double z) const;
    virtual double y2z(double y) const;

    virtual bool geodesic_step(double dl, VesselContext&) const;
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief ���������� �����
///

struct VesselElementCone
        : public VesselElement
{
    // ������� �������
    double diameter1;

    // ������ �������
    double diameter2;

public:
    VesselElementCone(double h, double d, double d2);

public:
    // �������� �� ���� ����� ������ ������?
    bool is_lower_bottom() const;

public:
    virtual double get_radius_by_lz(double z) const;
    virtual double get_radius_by_ly(double y) const;

    virtual double z2y(double z) const;
    virtual double y2z(double y) const;

    virtual bool geodesic_step(double dl, VesselContext&) const;

private:
    double calc_height_y() const;
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief ������������� �����
///

struct VesselElementEllipsoid
        : public VesselElementCone
{
    double a;  // ������� (��������������) �������
    double a2; // ������ � ����� ����� ����� (0 ��� �����������)
    double b;  // ����� (������������) �������
    double e;  // ��������������
    double e1; // �������������� 1
    double c;  // �������� ������ ��������

    double ellipsoid_height_z;

    finterpol local_z2y_f;
    finterpol local_y2z_f;
    finterpol local_y2B_f;
    finterpol local_B2y_f;

    friend class vessel_test;

public:
    VesselElementEllipsoid(double h, double d, double d2);

public:
    virtual double get_radius_by_lz(double z) const;
    virtual double get_radius_by_ly(double y) const;

    virtual double z2y(double z) const;
    virtual double y2z(double y) const;

    virtual bool geodesic_step(double ds, VesselContext&) const;

private:
    double get_height_z_by_diameters() const;
    void check_if_polar_mode(VesselContext&) const;

    bool geodesic_flat(double ds, VesselContext&) const;
    bool geodesic_ellipsoid(double ds, VesselContext&) const;

    double calc_height_y() const;
    void   generate_local_fs();
};

}

