#pragma once

#include "data/hitset.h"
#include "data/slice.h"
#include "data/minmax_map.h"

#include "data/aecollection.h"

#include "LocationSetup.h"
#include "locationClusters.h"

#include <boost/optional.hpp>
//////////////////////////////////////////////////////////////////////////

namespace location
{

using data::ae_record;
//////////////////////////////////////////////////////////////////////////

struct index_t
{
    double time;
    double amplitude;
    double channel;
    double peak;
    double energy;
    ae_record rec;
    unsigned index;
};

//////////////////////////////////////////////////////////////////////////

struct CollectorCache
{
    virtual ~CollectorCache(){}
};

/**
 * @brief ������� �����, ������� �������� �� ���������� �������.
 *
 */
class Collector
{
public:
    /**
     * ������������ �������������� �������,
     * ��� ������ � �����
     */
    enum { hitIgnore, ///< ���������
           hitNext,   ///< ������ ����� �������
           hitJoin    ///< ������������ � �������� �������
         };

public:
    Collector();
    virtual ~Collector() {};

    bool check(double const *);

    void create_event();
    void append_record(data::pslice, unsigned);
    virtual void end_of_file();

    void setup_and_reserve(unsigned);
    virtual void apply_cache(CollectorCache*) {}

    data::pslice make_event_slice(data::pslice);
    data::pslice  make_subhit_slice(data::pslice);

public:

    virtual std::vector<aera::chars>           get_chars() const=0;
    virtual std::map<aera::chars, std::string> get_units() const=0;

    virtual pclusters get_clusters() const;

    data::minmax_map get_minmax_map() const;
    shared_ptr<hits::hitset>          get_hitset() const;

    object_id tag() const;
    void setTag(const object_id &tag);

private:

    virtual int join_hit(index_t const &ox)=0;
    virtual void hit_inserted()=0;
    virtual bool calculate_coords(double*&)=0;
public:
    virtual void create_minmax_map()=0;

protected:

    bool    use_peak_time_;

    object_id tag_;

    /**
     * @brief �������� �����, ������� ��������� �������
     */
    std::vector<index_t> package_;

    /**
     * @brief �������� �����, ������� ������������� ��� ���.
     * (������ � ������� �������)
     */
    std::vector<index_t> ignore_;

    hits::phitset        set_;

    /**
     * @brief ��������������, ������� ������� ��������� � ��������
     */
    std::vector<aera::chars>  pos_chars_;

    /**
     * @brief ���������� �������, ������ ������� �������
     */
    unsigned                  pos_coords_count_;

    /**
     * @brief ������, ����������� � ��������
     */
    data::ae_collection       pos_data_;

    /**
      * @brief ������� ��������� ������
      */
    std::vector<unsigned>     pos_indexes_;

    /**
     * @brief ���������� ������� ��������� �����
     * ��� ������� � ���
     */
    std::vector<unsigned>       neg_indexes_;

    /**
     * @brief ��������������, ������� ��������� ������� � ���������
     * �����
     */
    std::vector<aera::chars>  neg_chars_;

    /**
     * @brief ������, ����������� � ��������� �����
     */
    data::ae_collection       neg_data_;


    data::minmax_map		  minmax_map_;
};

//////////////////////////////////////////////////////////////////////////

}
