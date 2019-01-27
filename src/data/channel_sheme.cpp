#include "stdafx.h"

#include "channel_sheme.h"

#include "utilites/foreach.hpp"

bool channel_object::hasinactive() const
{
    bool inactive = false;
    foreach(int channel, channels)
    {
        inactive = inactive | !isactive(channel);
    }

    return inactive;
}

bool channel_object::hasactive() const
{
    bool active = false;
    foreach(int channel, channels)
    {
        active = active | !!isactive(channel);
    }

    return active;
}

bool channel_object::isactive(int id) const
{
    assert((unsigned)id-1 < isactive_.size() && id >=1);
    return isactive_[id-1];
}
void channel_object::setactive(int id, bool s)
{
    assert((unsigned)(id-1) < isactive_.size() && id >=1);
    isactive_[id-1] = s;
}


void channel_object::setactive(bool b)
{
    foreach(int channel, channels)
    {
        setactive(channel, b);
    }
}

channel_sheme::channel_sheme()
{
    channel_object obj;
    obj.name = "";
    for (int i=1; i<=MAX_CHANNEL_NUMBER; ++i)
    {
        obj.channels.push_back( i );
        obj.setactive(i, 1);
    }

    objects.push_back( obj );
}

bool channel_sheme::isactive(int channel) const
{
    foreach(channel_object const &obj, objects)
    {
        foreach(unsigned ch, obj.channels)
        {
            if (ch==channel)
            {
                return obj.isactive(ch);
            }
        }
    }
    return false;
}

void channel_sheme::setactive(int channel, bool set)
{
    foreach(channel_object &obj, objects)
    {
        foreach(unsigned ch, obj.channels)
        {
            if (ch==channel)
                obj.setactive(ch, set);
        }
    }
}

channel_sheme channel_sheme::default_sheme()
{
    channel_sheme r;

    return r;
}

