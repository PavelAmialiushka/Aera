//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites/singleton.h"
#include "utilites/utilites.h"

#include "utilites/counter.h"
#include "utilites/log.h"

#include <boost/format.hpp>

class performance_counter
    : public utils::singleton<performance_counter>
{
public:
    struct info
    {
        __int64  time;
        unsigned count;

        info(): time(0), count(0) {}
    };

    ~performance_counter()
    {
        boost::mutex::scoped_lock lk(mutex_);

        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        double frequency=static_cast<double>(li.QuadPart/1000);

        aeraINFO("Perfomance counter: ==========");
        std::vector<debug::SourcePos> lt_vector;
        for (std::map<debug::SourcePos, info>::iterator index=map_.begin();
                index!=map_.end(); ++index)
        {
            lt_vector.push_back(index->first);
        }

        std::sort(STL_II(lt_vector));

        for (unsigned i=0; i<lt_vector.size(); ++i)
        {
            int count=map_[lt_vector[i]].count;
            double time=(static_cast<double>(map_[lt_vector[i]].time) / frequency);
            aeraINFO(boost::format("%27s: %5d  %8.3f")
                     % debug::SourcePos(lt_vector[i])
                     % count
                     % time
                    );
        }
        aeraINFO("End of perfomance counter: ===");
    }

    void set_time(debug::SourcePos lt, __int64 time)
    {
        boost::mutex::scoped_lock lk(mutex_);
        info &i=map_[lt];
        i.time+=time;
        i.count++;
    }

private:

    std::map<debug::SourcePos, info> map_;
    mutable boost::mutex mutex_;
};


mark::mark(debug::SourcePos lt)
    : tag_(lt)
{
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    time_=-li.QuadPart;
}

mark::~mark()
{
    LARGE_INTEGER li;
    ::QueryPerformanceCounter(&li);
    time_+=li.QuadPart;
    //performance_counter::instance()->set_time(tag_, time_);
}