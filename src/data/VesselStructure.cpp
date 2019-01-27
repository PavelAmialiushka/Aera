#include "stdafx.h"

#include "utilites/foreach.hpp"

#include "VesselStructure.h"
#include "VesselElements.h"

namespace location
{


unsigned VesselStructure::size() const
{
    return parts.size();
}

PVElement VesselStructure::element(int index) const
{
    return parts[index].get_element();
}

VesselPart VesselStructure::at(int index) const
{
    return parts[index];
}

VesselPart VesselStructure::operator[](int index) const
{
    return parts[index];
}

const std::vector<VesselPart> &VesselStructure::get_parts() const
{
    return parts;
}

void VesselStructure::clear()
{
    parts.clear();
    is_compiled = false;
}

void VesselStructure::append(const VesselPart &part)
{
    parts.push_back(part);
}

void VesselStructure::append(int type, double d1, double d2, double height)
{
    VesselPart p;
    p.type = type;
    p.diameter1 = d1;
    p.diameter2 = d2;
    p.height = height;
    append(p);
}

gfiy_coords VesselStructure::to_gfiy(const lxy_coords &local) const
{
    assert(is_compiled);
    int index = std::max(0u,
                         std::min(size()-1,
                         (unsigned)local.element_index));
    PVElement elem = element(index);

    gfiy_coords res;
    double ly = std::min(local.y, elem->height_y);
    res.y = elem->bottom_y + ly;
    double r = elem->get_radius_by_ly( ly );
    res.fi = r ? local.x / r : 0;
    return res;
}

gfiy_coords VesselStructure::to_gfiy(lfiy_coords const& local) const
{
    assert(is_compiled);
    assert( 0 <= local.element_index && local.element_index <= size());
    PVElement elem = element(local.element_index);

    gfiy_coords res;
    res.fi = local.fi;
    res.y = elem->bottom_y
            + std::min(local.y, elem->height_y);
//    assert( res.y <= elem->bottom_y + elem->height_y);
    return res;
}

xyz_coords VesselStructure::to_xyz(const gfiy_coords &c) const
{
    xyz_coords result;
    double r = get_radius_by_gy(c.y);
    double z = y2z(c.y);
    result.x = r * cos(c.fi);
    result.y = r * sin(c.fi);
    result.z = z;
    return result;
}


lfiy_coords VesselStructure::to_lfiy(const gfiy_coords & crds, int part_index) const
{
    assert(is_compiled);
    lfiy_coords result;
    result.fi = crds.fi;
    for(int index=0; index < size(); ++index)
    {
        PVElement el = element(index);

        if (part_index >=0 ? part_index == index
               : (el->bottom_y <= crds.y && crds.y <= el->bottom_y + el->height_y))
        {
            result.element_index = index;
            result.y = crds.y - el->bottom_y;
            return result;
        }
    }
    assert(!"incorrect y");
    result.element_index = -1;
    result.y = std::numeric_limits<double>::quiet_NaN();
    return result;
}

lxy_coords VesselStructure::to_lxy(const gfiy_coords & crds, bool *ok) const
{
    assert(is_compiled);
    lxy_coords result;
    for(int index=0; index < size(); ++index)
    {
        PVElement el = element(index);

        if (el->bottom_y <= crds.y && crds.y <= el->bottom_y + el->height_y)
        {
            result.element_index = index;
            result.y = crds.y - el->bottom_y;

            double r = el->get_radius_by_ly(result.y);
            if (r != 0)
                result.x = fmod2(r * crds.fi, 2 * M_PI * r);
            else
                result.x = 0;

            if (ok) *ok = true;
            return result;
        }
    }
    result.element_index = -1;
    result.y = std::numeric_limits<double>::quiet_NaN();

    if (ok) *ok = false;
    else    assert(!"incorrect y");

    return result;
}

double VesselStructure::get_radius_by_gy(double gy) const
{
    assert(is_compiled);
    for(int index=0; index < size(); ++index)
    {
        PVElement elem = element(index);
        if (elem->bottom_y <= gy && gy <= elem->bottom_y + elem->height_y)
        {
            double lz = elem->y2z( gy - elem->bottom_y );
            return elem->get_radius_by_lz(lz);
        }
    }
    assert(!"too big height y");
    return 0;
}

double VesselStructure::to_nx(gfiy_coords const& fiy) const
{
    assert(is_compiled);
    for(int index=0; index < size(); ++index)
    {
        PVElement elem = element(index);
        if (elem->bottom_y <= fiy.y && fiy.y <= elem->bottom_y + elem->height_y)
        {
            double lz = elem->y2z( fiy.y - elem->bottom_y );
            double r = elem->get_radius_by_lz(lz);
            double maxr = std::max(
                        elem->get_radius_by_lz(0),
                        elem->get_radius_by_lz(elem->height_z));

            double maxw = 2 * M_PI * maxr;
            double w = 2 * M_PI * r;
            double x = fiy.fi * r;

            x = w ? fmod2(x, w) : 0;
            const int col_size = 8;
            int col_index = w ? (x * col_size / w) : 0;

            double dwd = (maxw - w) / col_size / 2;
            double dw =  (1 + col_index * 2) * dwd;
            return x + dw;
        }
    }
    assert(!"too big height y");
    return 0;
}

double VesselStructure::z2y(double z) const
{
    if (z < 0) return 0;
    if (z > height_z) return height_y;
    for(int index=0; index < size(); ++index)
    {
        PVElement elem = element(index);
        if (elem->bottom_z <= z && z <= elem->bottom_z + elem->height_z)
        {
            return elem->bottom_y + elem->z2y(z - elem->bottom_z);
        }
    }
    assert(!"too big height z");
    return 0;
}

double VesselStructure::lz2ly(int index, double z) const
{
    if (index < 0 || index >= size())
    {
        return 0;
    }

    PVElement elem = element(index);
    return elem->z2y(z);
}

double VesselStructure::y2z(double y) const
{
    if (y < 0) return 0;
    if (y > height_y) return height_z;
    for(int index=0; index < size(); ++index)
    {
        PVElement elem = element(index);
        if (elem->bottom_y <= y && y <= elem->bottom_y + elem->height_y)
        {
            return elem->bottom_z + elem->y2z(y - elem->bottom_y);
        }
    }
    assert(!"too big height y");
    return 0;
}

double VesselStructure::local_distance(const gfiy_coords &target, const gfiy_coords &island, double *teta)
{
    // радиус в искомой точке
    double r0 = get_radius_by_gy(target.y);

    // радиус в найденной точке
    double r1 = get_radius_by_gy(island.y);

    double dx = r0 * sin(target.fi) - r1 * sin(island.fi);
    double dy = r0 * cos(target.fi) - r1 * cos(island.fi);
    double dz = y2z(target.y) - y2z(island.y);

    // погрешность расположения
    double dr = sqrt(dx*dx + dz*dz + dy*dy);
    if (teta)
    {
        // угол определяющий направление на заданную точку
        double dyy = target.y - island.y;
        double dxx = 0.5 * (r0 + r1) * (target.fi - island.fi);
        *teta = atan2(dyy, dxx);
        assert(!_isnan(*teta));
    }
    return dr;
}

bool VesselStructure::get_is_vertical() const
{
    return is_vertical;
}

void VesselStructure::set_is_vertical(bool value)
{
    is_vertical = value;
}

double VesselStructure::get_width_x() const
{
    assert(is_compiled);
    return width_x;
}

double VesselStructure::get_height_z() const
{
    assert(is_compiled);
    return height_z;
}

double VesselStructure::get_height_y() const
{
    assert(is_compiled);
    return height_y;
}

double VesselStructure::get_default_step() const
{
    return default_step;
}

size_t VesselSensor::ray_step_count()
{
    return  max_distance() / ray_step_size();
}


VesselStructure::VesselStructure()
{
    default_step = VesselSensor::ray_step_size();
    is_vertical = true;
}

// 360:      count= 43 / 11 (26.9% / 6.9%), max_est=0.069 max_dis=0.108, avr_dis=0.005
// 256:      count= 43 / 3 (26.9% / 1.9%), max_est=2.552 max_dis=0.345, avr_dis=0.006
// 192: 3210 count= 35 / 7 (21.9% / 4.4%), max_est=2.555 max_dis=0.095, avr_dis=0.004
// 128: 1944 count= 48 / 5 (30.0% / 3.1%), max_est=4.331 max_dis=0.056, avr_dis=0.004
//  64: 1049 count= 39 / 5 (24.4% / 3.1%), max_est=2.660 max_dis=0.010, avr_dis=0.003
//  32:      count= 50 / 8 (31.3% / 5.0%), max_est=2.940 max_dis=0.238, avr_dis=0.007

// после оптимизации
// 128: 2108[ms] count= 48 / 5 (30.0% / 3.1%), max_est=4.331m max_dis=0.056, avr_dis=0.004

// только половина лучей
// 128: 3714[ms] count= 43 / 2 (26.9% / 1.3%), max_est=2.570m max_dis=0.010, avr_dis=0.003

// только четверть лучей
// 128: 2358[ms] count= 51 / 3 (31.9% / 1.9%), max_est=2.617m max_dis=0.112, avr_dis=0.006

// точко четверть + включение (не активируется)
// 128: 2011[ms] count= 51 / 3 (31.9% / 1.9%), max_est=2.617m max_dis=0.112, avr_dis=0.006
// 160: 2609[ms] count= 43 / 4 (26.9% / 2.5%), max_est=3.187m max_dis=0.261, avr_dis=0.009
// 192: 2970[ms] count= 40 / 3 (25.0% / 1.9%), max_est=2.555m max_dis=0.021, avr_dis=0.004
// 256: 4507[ms] count= 41 / 6 (25.6% / 3.8%), max_est=2.552m max_dis=0.012, avr_dis=0.003

// Island-swap.
// 192: 3216[ms] count= 45 / 5 (28.1% / 3.1%), max_est=0.345m max_dis=0.019, avr_dis=0.003
// Island-swap C на основе A
// 192: 3024[ms] count= 46 / 11 (28.8% / 6.9%), max_est=2.614m max_dis=0.375, avr_dis=0.006
// Island-swap убрали нафиг С
// 192: 2995[ms] count= 42 / 3 (26.3% / 1.9%), max_est=0.333m max_dis=0.029, avr_dis=0.004

size_t VesselSensor::ray_count()
{
    return 192;
}

double VesselSensor::max_distance()
{
    return 10.0;
}

size_t VesselSensor::tree_step_per_ray_step()
{
    return 10;
}

double VesselSensor::ray_step_size()
{
    return 0.005;
}

double VesselSensor::tree_step_size()
{
    return tree_step_per_ray_step() * ray_step_size();
}

size_t VesselSensor::tree_step_count()
{
    return ray_step_count() / tree_step_per_ray_step();
}


void VesselStructure::geodesic_step(VesselContext &ctx, double step) const
{
    if (step < 0) step = get_default_step();

    int index = ctx.coords.element_index;
    assert(0 <= index && index < size());

    PVElement elem = element(index);

    if (elem->geodesic_step(step, ctx))
    {
        bool moving_up = cos(ctx.azimuth) > 0;
        if (moving_up)
        {
            ++ctx.coords.element_index;
            ctx.coords.y = 0;
        } else
        {
            --ctx.coords.element_index;
            if (ctx.coords.element_index >=0)
            {
                PVElement el2 = element(ctx.coords.element_index);
                ctx.coords.y = el2->height_y;
            }
        }
    }
}

gfiy_coords VesselStructure::geodesic_line(gfiy_coords start,
                                           double azimuth, double distance,
                                           double step)
{
    lfiy_coords base = to_lfiy( start );

    if (step < 0) step = get_default_step();

    VesselContext ctx(base, normal_angle(azimuth));
    while(ctx.distance < distance)
    {
        // если вышли за пределы
        if (!between_e(
                    ctx.coords.element_index,
                    0, (int)size()-1))
            break;

        // расчитываем следующую точку
        geodesic_step(ctx, step);
        assert(is_normal_angle(ctx.coords.fi));
    }

    return to_gfiy( ctx.coords );
}

void VesselStructure::serialization(serl::archiver &ar)
{
    ar.serial_container("formula", parts);
    ar.serial("vertical", is_vertical);

    if (ar.is_loading())
    {
        compile();
    }
}

VesselPosition &VesselSensor::ray(int x, int y)
{
    unsigned index = x + y * ray_count();
    assert(index < rays_.size());
    return rays_[index];
}

VesselSensor::VesselSensor(VesselStructure *v, gfiy_coords base, bool recalc)
    : vessel_(v)
    , base_coords_(base)
{
    distance_index_ = 0;

    std::vector<double> radius;

    // инициализируем дерево
    tree_.clear();

    // иницализируем хранилище лучей
    rays_.resize(ray_count() * ray_step_count());

    // собственно, лучи
    contexts_.reserve(ray_count());    

    init_ray_info(base);

    if (recalc)
    {
        build_distance_map();
    }
}

struct Island
{
    VesselSensor*  self;
    VesselPosition position;
    gfiy_coords    target;
    int rayx, rayy;
    double dr;
    double teta;
    double alpha;

    Island(VesselSensor* self, gfiy_coords target,  int index)
        : self(self)
        , target(target)
    {
        rayx = index % self->ray_count();
        rayy = index / self->ray_count();
        position = self->ray(rayx, rayy);

        update();
    }

    void update()
    {
        dr = self->vessel_->local_distance(target, position.coords, &teta);

        // угол между азимутом и тетой
        alpha = position.alpha(teta);
        assert(!_isnan(alpha));
    }

    void move_to(int x, int y)
    {
        rayx = fmod2(x, self->ray_count());
        rayy = std::min<unsigned>(y, self->ray_step_count()-1);

        position = self->ray(rayx, rayy);
        update();
    }

    void move_by(int x, int y)
    {
        move_to(rayx + x, rayy + y);
    }

    void rebase(int andx = true)
    {
        int dy = dr * cos(alpha) / self->ray_step_size();
        int dx = andx
                ? dr * sin(alpha) / (position.distance * (M_360 / self->ray_count()))
                : 0;

        if (dy || dx)
            move_to(rayx + dx, rayy + dy);
    }
};



double VesselSensor::distance_to(gfiy_coords target, double *azimuth)
{
    std::vector<VesselPosition> result;
    xyz_coords xyz = vessel_->to_xyz(target);

#if 1
    int id = tree_.getclosest_id(xyz);
    if (id==-1) return 100.0; // слишком далеко
#else
    // выбираем несколько точек, лежащих в ближнем круге
    int ids[4] = {0};
    int count = 0;
    double radius = tree_step_size();
    while(!count)
    {
        count = tree_.getclosest_ids(xyz, ids, SIZEOF(ids), radius);
        radius *= 2;
    }

    // поиск точки c наименьшим растоянием
    int id = ids[0];
    double idd = 1e9;
    for(unsigned index=0; index < count; ++index)
    {
        Island X(this, target, ids[index]);
        if (X.position.distance < idd)
        {
            idd = X.position.distance;
            id = ids[index];
        }
    }
#endif

    // основная точка
    Island A(this, target, id);
    A.rebase(true);

//    double dd = vessel_->to_xyz(A.position.coords).distance(xyz);

    // второй элемент - остров сбоку
    // куда двигаться, влево или вправо?
    // определяем величину смещения вбок
    Island B=A;
    int dx = sin(A.alpha) < 0 ? -1 : 1;
    B.move_by(dx, 0);
    B.rebase(false);

    if (B.dr < A.dr)
        std::swap(A, B);

    // работаем с 1`м островом
    double d = A.position.distance;
    double dr = A.dr;
    double dn = sqrt(d*d + dr*dr - 2*d*dr*cos(A.alpha));

    // определяем начальный азимут (нужно только для тестов)
    if (azimuth)
    {        
        // определяем веса вершин
        double f0 = A.dr>1e-6 ? 1 / A.dr : 1e9;
        double f1 = B.dr>1e-6 ? 1 / B.dr : 1e9;

        double total = f0 + f1;
        f0 /= total; f1 /= total;

        double az = A.position.start_azimuth;
        double daz = normal_angle(B.position.start_azimuth - az);
        *azimuth = normal_angle(az + daz * f1);
    }

    assert(!_isnan(dn));
    assert(dn >= 0);
    return dn;
}

void VesselSensor::init_ray_info(gfiy_coords base)
{
    ray_info info = {true, 0};
    ray_info_.resize(ray_count(), info);

    for(unsigned index=0; index < ray_count(); ++index)
    {
        lfiy_coords base = vessel_->to_lfiy( base_coords_ );
        double alpha = index * M_360 / ray_count();
        contexts_.push_back(VesselContext(base, normal_angle(alpha)));

        // работаем пока только с четвертью элементов
        ray_info_[index].hidden = (index % 4 != 0);
    }
}

void VesselSensor::process_all()
{
    while(process_step())
    {
    }
}


bool VesselSensor::process_step()
{
    double distance = distance_index_ * ray_step_size();
    if (distance_index_ >= ray_step_count())
        return false;

    KDTree local_tree;
    bool flag_distance =
            (distance_index_ % tree_step_per_ray_step() == 0)
            &&
            (distance_index_ > tree_step_per_ray_step() * 4);
    for(unsigned index=0; index < ray_count(); ++index)
    {
        if (!ray_info_[index].online)
        {
            if (ray_info_[index].counter)
                ray_info_[index].counter--;
            else
                continue;
        }

        VesselContext &context = contexts_[index];

        // если вышли за пределы
        if (!between_e(
                    context.coords.element_index,
                    0, (int)vessel_->size()-1))
        {
            continue;
        }

        // определяем координаты точки
        gfiy_coords gc = vessel_->to_gfiy(context.coords);
        VesselPosition pos(gc,
                           context.azimuth,
                           context.distance,
                           context.start_azimuth);

        // запоминаем в хранилище лучей
        pos.ray_index = index + distance_index_ * ray_count();
        ray(index, distance_index_) = pos;

        local_tree.add(vessel_->to_xyz(gc), pos.ray_index);

        // записываем только некоторые шаги
        if (flag_distance
                && (index==0 || distance_index_ != 0)
                && !ray_info_[index].hidden)
        {            
            tree_.add(vessel_->to_xyz(gc), pos.ray_index);
        }

        // расчитываем следующую точку
        vessel_->geodesic_step(context);
        assert(is_normal_angle(context.coords.fi));
    }

    if (flag_distance)
    {
        for(int index=0; index < ray_count(); ++index)
        {
            if (!ray_info_[index].online) continue;

            VesselPosition const& pos = ray(index, distance_index_);

            int ids[2] = {0};
            int count = local_tree.getclosest_ids(
                            vessel_->to_xyz(pos.coords),
                            ids, SIZEOF(ids),
                            tree_step_size());

            std::multimap<double, int> ixes;
            for(unsigned jo = 0; jo < count; ++jo)
            {
                int rayx = ids[jo] % ray_count();
                int rayy = ids[jo] / ray_count();

                if (rayx == index) continue;

                // следующий сосед
                VesselPosition const& sibl = ray(rayx, rayy);

                // расстояние до этой точки
                double d = vessel_->to_xyz(sibl.coords).distance(
                            vessel_->to_xyz(pos.coords));
                ixes.insert(std::make_pair(d, jo));
            }

            if (ixes.empty()) continue;
            int jo = ixes.begin()->second;
            int rayx = ids[jo] % ray_count();
            int rayy = ids[jo] / ray_count();

            // следующий сосед
            VesselPosition const& sibl = ray(rayx, rayy);

            // расстояние до этой точки
            double d1, d = vessel_->to_xyz(sibl.coords).distance(
                           vessel_->to_xyz(pos.coords));

            // если лучи достаточно прорежены, подключаем
            if (ray_info_[index].hidden)
            {
                double limit = 0;
                if (index % 2 == 1)
                {   // первый уровень
                    limit = tree_step_size();
                }
                else if (index % 4 == 2)
                {   // второй уровень
                    limit = tree_step_size() * 2;
                }

                // подключаем
                if (limit && d > limit)
                    ray_info_[index].hidden = false;
            }


            // абсолютная разница между индексами
            int dx = abs(mod2_half(rayx - index, ray_count()));

            if (dx == 1)
            {
                int left_index = std::min(index, rayx);
                if (std::max(index, rayx) == ray_count()-1)
                    left_index = 0;

                d1 = ray_info_[left_index].neighbour_distance;
                if (d1 == 0)
                {
                    ray_info_[left_index].neighbour_distance = d;
                    continue;
                }
                else if (d1 > d || d > tree_step_size()/10)
                {
                    // если расстояние между соседями увеличивается,
                    // то это нормально
                    continue;
                }
            }

#if 0
            // точки, по которым проходит разделение сосуда
            LOG(strlib::strf("{%.3f, %.3f},", pos.coords.fi, pos.coords.y));
#endif

            // больше просчет луча и его напарника не ведем
            ray_info_[index].online = false;
            ray_info_[rayx].online = false;

            ray_info_[index].counter = tree_step_per_ray_step();
            ray_info_[rayx].counter = tree_step_per_ray_step();

            break;
        }
    }

    ++distance_index_;
    return true;
}

const VesselPosition &VesselSensor::get_from_tree(const gfiy_coords &gc, bool &found)
{
    xyz_coords xyz = vessel_->to_xyz(gc);
    int index = tree_.getclosest_id(xyz);
    if (index == -1)
    {
        found = false;
        index = 0;
    } else
    {
        found = true;
    }

    int rayx = index % ray_count();
    int rayy = index / ray_count();

    return ray(rayx, rayy);
}

void VesselSensor::build_distance_map()
{
    while(process_step())
        ;
}

void VesselStructure::compile()
{
    is_compiled = true;

    std::vector<double> ds(parts.size()+1, 0.0);

    // первый проход, цилиндры обозначают соседние диаметры
    int index=0;
    foreach(VesselPart& part, parts)
    {
        part.has_diameter1_ = false;
        part.has_diameter2_ = false;
        part.has_height_ = false;

        if (part.type == VesselPart::Cylinder)
        {
            part.diameter2 = part.diameter1;
            part.has_diameter1_ = true;
            part.has_height_ = true;
            ds[index] = part.diameter1;
            ds[index+1] = part.diameter1;
        }

        ++index;
    }

    // второй проход
    for(index=parts.size(); index --> 0; )
    {   VesselPart& part = parts[index];

        if (part.type != VesselPart::Cylinder)
        {
            double &hi = ds[index+1];
            double &lo = ds[index];
            if (hi && lo)
            {
                part.diameter1 = std::max(hi, lo);
                part.diameter2 = std::min(hi, lo);
                part.is_upper_bottom_ = lo > hi;
            } else if (hi)
            {
                part.diameter1 = hi;
                part.has_diameter2_ = true;
                part.diameter2 = std::min(hi, part.diameter2);
                part.is_upper_bottom_ = false;
                lo = part.diameter2;
            } else if (lo)
            {
                part.diameter1 = lo;
                part.has_diameter2_ = true;
                part.diameter2 = std::min(lo, part.diameter2);
                part.is_upper_bottom_ = true;
            } else
            {
                part.has_diameter1_ = true;
                part.is_upper_bottom_ = true;
                lo = part.diameter1;
            }
        }

        switch(part.type) {
        case VesselPart::Ellipsoid:
            part.height = calculate_height(part.diameter1 / 4, part.diameter1, part.diameter2);
            break;
        case VesselPart::Sphere:
            part.height = calculate_height(part.diameter1 / 2, part.diameter1, part.diameter2);
            break;
        case VesselPart::Flat:
            part.height = 0;
            break;
        case VesselPart::Cone:
            part.has_height_ = true;
        }

        part.compile();
    }

    index=0;
    double bottom_y = 0;
    double bottom_z = 0;
    width_x = 0;
    foreach(VesselPart const& part, parts)
    {
        PVElement item = part.get_element();
        item->index = index;
        item->bottom_y = bottom_y;
        item->bottom_z = bottom_z;

        double xx = std::max(item->get_radius_by_lz(0),
                      item->get_radius_by_lz(item->height_z));
        width_x = std::max(width_x, xx);

        ++index;
        bottom_y += item->height_y;
        bottom_z += item->height_z;
    }

    height_y = bottom_y;
    height_z = bottom_z;
}

bool VesselStructure::operator==(const VesselStructure &other) const
{
    if (parts.size() != other.parts.size())
        return false;
    if (is_vertical != other.is_vertical)
        return false;
    for(unsigned index=0; index < other.parts.size(); ++index)
    {
        if (parts[index] != other.parts[index])
            return false;
    }
    return true;
}

bool VesselStructure::operator!=(const VesselStructure &other) const
{
    return !((*this) == other);
}

double VesselStructure::calculate_height(double eh, double d1, double d2)
{
    double cos = d2 / d1;
    double sin = sqrt(1 - cos*cos);
    return sin * eh;
}

void create_net_map(PVElement self, std::vector<lxy_coords>& result)
{
    const int col_size = 8;
    const int row_size = 10;

    double w0 = self->get_width();

    bool bottom =
            self->get_radius_by_ly(0) <
            self->get_radius_by_ly(self->height_y);

    for(int col=0; col < col_size; ++col)
    {
        int row;
        for(row = 0; row <= row_size; ++row)
        {
            int eff_row = bottom ? (row_size - row) : row;
            double ly = eff_row * self->height_y / row_size;
            double w = 2 * M_PI * self->get_radius_by_ly(ly);
            double dw = (w0 - w) / col_size / 2;

            double x = col*w0/col_size + dw;

            result.push_back(lxy_coords(-1, x, ly));
        }

        while(row --> 0)
        {
            int eff_row = bottom ? (row_size - row) : row;
            double ly = eff_row * self->height_y / row_size;
            double w = 2 * M_PI * self->get_radius_by_ly(ly);
            double dw = (w0 - w) / col_size / 2;

            double x = (col+1)*w0/col_size - dw;

            result.push_back(lxy_coords(-1, x, ly));
        }
    }

    result.push_back(result.front());
}

}
