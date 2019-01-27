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
 * @brief базовый класс, который отвечает за выполнение локации.
 *
 */
class Collector
{
public:
    /**
     * Перечисление представляющее Решение,
     * что делать с хитом
     */
    enum { hitIgnore, ///< отбросить
           hitNext,   ///< начать новое событие
           hitJoin    ///< присоединить к текущему событию
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
     * @brief перечень хитов, которые формируют событие
     */
    std::vector<index_t> package_;

    /**
     * @brief перечень хитов, которые отбрасываются как шум.
     * (входят в отсечку события)
     */
    std::vector<index_t> ignore_;

    hits::phitset        set_;

    /**
     * @brief характеристики, которые локация добавляет к событиям
     */
    std::vector<aera::chars>  pos_chars_;

    /**
     * @brief количество ординат, которе создает локация
     */
    unsigned                  pos_coords_count_;

    /**
     * @brief данные, добавленные к событиям
     */
    data::ae_collection       pos_data_;

    /**
      * @brief индексы выбранных данных
      */
    std::vector<unsigned>     pos_indexes_;

    /**
     * @brief запоминаем индексы вторичных хитов
     * для доступа к ним
     */
    std::vector<unsigned>       neg_indexes_;

    /**
     * @brief характеристики, которые добавляет локация к удаленным
     * хитам
     */
    std::vector<aera::chars>  neg_chars_;

    /**
     * @brief данные, добавленные к удаляемым хитам
     */
    data::ae_collection       neg_data_;


    data::minmax_map		  minmax_map_;
};

//////////////////////////////////////////////////////////////////////////

}
