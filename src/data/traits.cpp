#include "StdAfx.h"

#include "data\traits.h"

#include "data\parametric.h"
#include "data\nodeFactory.h"

#include "utilites/Localizator.h"
#include "data/LocationUnit.h"

namespace unit
{
using namespace location;
}

namespace aera
{

namespace traits
{

struct traits_info_t
{
    chars c;
    const char *wide_name;
    const char *short_name;
    const char *type_name;
    int         unit_name;
    bool operator ==(char a) const
    {
        return c==a;
    }
} traits_info[]=
{
    {C_Time,              _lis("Time"),                    "T",   "%13.7f", unit::sec  },
    {C_PeakTime,          _lis("Time peak"),               "TP",  "%13.7f", unit::sec  },
    {C_Channel,           _lis("Channel"),                 "Ch",  "%3.0f",  unit::none },
    {C_Amplitude,         _lis("Amplitude"),               "A",   "%3.0f",  unit::dB   },
    {C_Duration,          _lis("Duration"),                "D",   "%7.0f",  unit::usec },
    {C_Counts,            _lis("Counts"),                  "C",   "%6.0f",  unit::none },
    {C_Frequency,         _lis("Frequency"),               "F",   "%5.0f",  unit::none },
    {C_Risetime,          _lis("Rise Time"),               "RT",  "%7.0f",  unit::usec },
    {C_Countstopeak,      _lis("Counts to Peak"),          "CtP", "%6.0f",  unit::none },
    {C_InitiationFreq,    _lis("Initiation frequence"),    "iF",  "%4.0f",  unit::none },
    {C_ReverberationFreq, _lis("Reverberation frequence"), "rF",  "%4.0f",  unit::none },
    {C_CentroidFreq,      _lis("Centroid frequence"),      "cF",  "%4.0f",  unit::none },
    {C_PeakFreq,          _lis("Peak frequence"),          "pF",  "%4.0f",  unit::none },
    {C_Energy,            _lis("Energy"),                  "E",   "%7.0f",  unit::none },
    {C_AbsoluteEnergy,    _lis("Absolute energy"),         "aE",  "%9.1f",  unit::none },// *0.000931
    {C_SignalStrength,    _lis("Signal strength"),         "SS",  "%9.1f",  unit::none },   // *3.05
    {C_ASL,               _lis("ASL"),                     "ASL", "%3.0f",  unit::dB   },
    {C_RMS,               _lis("RMS"),                     "RMS", "%4.3f",  unit::none },// *0.05
    {C_Threashold,        _lis("Threshold"),               "Th",  "%2.0f",  unit::dB   },
    {C_PartialPower1,     _lis("Partial Power 1"),         "PP1", "%3.1f",  unit::none },
    {C_PartialPower2,     _lis("Partial Power 2"),         "PP2", "%3.1f",  unit::none },
    {C_PartialPower3,     _lis("Partial Power 3"),         "PP3", "%3.1f",  unit::none },
    {C_PartialPower4,     _lis("Partial Power 4"),         "PP4", "%3.1f",  unit::none },
    {C_Parametric1,       _lis("Parametric 1"),            "P1",  "%5.2f",  unit::none },
    {C_Parametric2,       _lis("Parametric 2"),            "P2",  "%5.2f",  unit::none },
    {C_Parametric3,       _lis("Parametric 3"),            "P3",  "%5.2f",  unit::none },
    {C_Parametric4,       _lis("Parametric 4"),            "P4",  "%5.2f",  unit::none },
    {C_Gain,              _lis("Gain"),                    "Gn",  "%5.0f",  unit::dB   },
    {C_Losthits,          _lis("Losthits"),                "Lh",  "%5.0f",  unit::none },
    {C_Preamp,            _lis("Preamp"),                  "PA",  "%5.0f",  unit::none },
    {C_Hits,              _lis("Hits"),                    "Ht",  "%2.0f",  unit::none },
    {C_CoordX,            _lis("X coord"),                 "X",   "%10.4f", unit::none },
    {C_CoordY,            _lis("Y coord"),                 "Y",   "%10.4f", unit::none },
    {C_CoordSigma,        _lis("Coord sigma"),             "Sigm","%10.4f", unit::none },
    {C_RelTime,           _lis("Time (relative)"),         "RelT","%10.7f",  unit::sec },
    {C_RelPeakTime,       _lis("Time peak (relative)"),    "RelTP","%10.7f", unit::sec },
    {C_SubHitCount,       _lis("Sub hits count"),          "SHC", "%4.0f",   unit::none },
    {C_IgnoredHitCount,   _lis("Ignored hits count"),      "IHC", "%4.0f",   unit::none },
    {C_FirstSubHit,       "",/*hidden*/                    "_",   "%1.0f",   unit::none },
    {C_FirstHit,          "",/*hidden*/                    "_",   "%1.0f",   unit::none },
    {C_OriginalParametric1,_lis("Raw parametric 1"),           "RP1",  "%5.2f",  unit::volt },
    {C_OriginalParametric2,_lis("Raw parametric 2"),           "RP2",  "%5.2f",  unit::volt },
    {C_OriginalParametric3,_lis("Raw parametric 3"),           "RP3",  "%5.2f",  unit::volt },
    {C_OriginalParametric4,_lis("Raw parametric 4"),           "RP4",  "%5.2f",  unit::volt },

};
bool  initialized_=false;
boost::array<int, 256>  indexer_;

inline
traits_info_t &info_of(chars c)
{
    if (!initialized_)
    {
        for (unsigned i=0; i<indexer_.size(); ++i)
        {
            traits_info_t
            *end=traits_info+sizeof(traits_info)/sizeof(*traits_info),
             *info=
                 std::find(traits_info, end, static_cast<chars>(i));
            indexer_[i]=info-traits_info;
        }
        initialized_=true;
    }
    traits_info_t &info=traits_info[indexer_[c]];

    if (info.c != c) return info_of(aera::C_Time);
    debug::Assert<traits_error>(info.c==c, HERE);
    return info;
}

std::string get_format(chars c)
{
    return info_of(c).type_name;
}

std::string get_wide_name(chars c)
{
    unsigned index=c-C_Parametric1;
    if (index < 4)
    {
        boost::array<parametric, 4> vector;
        // TODO: uncomment
         nodes::factory().get_parametric_data(&vector[0]);
        return vector[index].name;
    }
    return _lis2( info_of(c).wide_name );
}

std::string get_short_name(chars c)
{
    return info_of(c).short_name;
}

std::string get_unit_name(chars c)
{
    unsigned index=c-C_Parametric1;
    if (index < 4)
    {
        boost::array<parametric, 4> vector;
        // TODO uncomment
        nodes::factory().get_parametric_data(&vector[0]);
        return vector[index].unit_name;
    }
    return unit::get_short_name( (unit::Unit) info_of(c).unit_name );
}

bool is_tdd_common_data(chars c)
{
    return c==C_Time ||
          (C_Parametric1 <= (int)c && (int)c <=C_Parametric4) ||
          (C_OriginalParametric1 <= (int)c && (int)c <=C_OriginalParametric4);
}

}
}
