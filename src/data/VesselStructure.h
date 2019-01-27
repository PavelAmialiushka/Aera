#pragma once

#include "VesselMisc.h"
#include "VesselPart.h"

class vessel_test;

namespace location
{

////////////////////////////////////////////////////////////////////////////////////

struct VesselStructure
{    
private:
    std::vector<VesselPart> parts;
    bool    is_vertical;
    bool    is_compiled;
    // доступно после компиляции

    double  height_y;
    double  height_z;
    double  width_x;

    // шаг для расчета геодеизческой линии
    double  default_step;

    friend class VesselSensor;
    friend class vessel_test;


public:
    VesselStructure();

    bool operator==(const VesselStructure& other) const;
    bool operator!=(const VesselStructure& other) const;

    void serialization(serl::archiver& ar);

public: // mermers
    unsigned size() const;
    VesselPart at(int index) const;
    PVElement element(int index) const;
    VesselPart operator[](int index) const;

    std::vector<VesselPart> const& get_parts() const;

    void clear();
    void append(VesselPart const& part);
    void append(int type, double d1=0, double d2=0, double height=0);
    void compile();

    bool get_is_vertical() const;
    void set_is_vertical(bool value);

    double get_height_z() const;
    double get_height_y() const;
    double get_width_x() const;
    double get_default_step() const;

private:
    static double calculate_height(double d1, double d2, double h);

public: // coordinates

    // преобразования координат
    gfiy_coords to_gfiy(lxy_coords const& c) const;
    gfiy_coords to_gfiy(lfiy_coords const& c) const;
    xyz_coords to_xyz(gfiy_coords const& c) const;
    gfiy_coords operator*(lfiy_coords const& crds) const
    { return to_gfiy(crds); }

    lfiy_coords to_lfiy(gfiy_coords const&, int index=-1) const;
    lxy_coords to_lxy(gfiy_coords const&, bool*ok=0) const;

    double get_radius_by_gy(double y) const;

    // координата x на развертке
    double to_nx(gfiy_coords const& fiy) const;

    double z2y(double z) const;
    double lz2ly(int el, double z) const;
    double y2z(double y) const;

    double local_distance(gfiy_coords const& target, gfiy_coords const& rhs, double* teta=0);

public: // geodesic
    void geodesic_step(VesselContext& ctx,
                       double default_step=-1) const;

    gfiy_coords geodesic_line(gfiy_coords start,
                              double azimuth, double distance,
                              double default_step = -1);

};

////////////////////////////////////////////////////////////////////////////////////

void create_net_map(PVElement self, std::vector<lxy_coords>& result);

////////////////////////////////////////////////////////////////////////////////////

struct VesselSensor
{
public:
    VesselStructure* vessel_;
    gfiy_coords     base_coords_;
    KDTree          tree_;
    std::vector<VesselPosition> rays_;

    int distance_index_;
    std::vector<VesselContext>  contexts_;

    struct ray_info
    {
        bool online;
        bool hidden;
        int counter;
        double neighbour_distance;
    };
    std::vector<ray_info> ray_info_;

    VesselPosition& ray(int x, int y);

public:
    VesselSensor(VesselStructure* v,
                 gfiy_coords base,
                 bool recalc_distances = false);

    double distance_to(gfiy_coords c, double* azimuth=0);

    // размеры ячеек хранилища лучей
    static size_t ray_step_count();
    static double ray_step_size();

    // количество ячеек хранилища дерева
    static size_t tree_step_per_ray_step();
    static double tree_step_size();
    static size_t tree_step_count();

    // количество лучей для расчета
    static size_t ray_count();

    // максимальное просчитываемое расстояние
    static double max_distance();

    void process_all();
    bool process_step();

    // запись и чтение из дерева
    VesselPosition const& get_from_tree(gfiy_coords const& gc, bool &found);


    void init_ray_info(gfiy_coords base);

private:

    void build_distance_map();
};

}

