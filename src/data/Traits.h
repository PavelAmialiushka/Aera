#pragma once

//////////////////////////////////////////////////////////////////////////

namespace aera
{

DEFINE_ERROR(traits_error, "traits error");

enum chars
{
    C_Risetime         =0,
    C_Begin            =C_Risetime,
    C_Countstopeak     =1,
    C_Counts           =2,
    C_Energy           =3,
    C_Duration         =4,
    C_Amplitude        =5,
    C_RMS              =6,
    C_ASL              =7,
    C_Gain             =8,
    C_Threashold       =9,
    C_Preamp           =10,
    C_Losthits         =11,
    C_Frequency        =12,
    C_Time             =13,
    C_Channel          =14,
    C_ReverberationFreq=15,
    C_InitiationFreq   =16,
    C_SignalStrength   =17,
    C_AbsoluteEnergy   =18,
    C_PartialPower1    =19,
    C_PartialPower2    =20,
    C_PartialPower3    =21,
    C_PartialPower4    =22,
    C_Parametric1      =23,
    C_Parametric2      =24,
    C_Parametric3      =25,
    C_Parametric4      =26,
    C_Hits             =27,

    C_CentroidFreq     =28,
    C_PeakFreq         =29,

    // связаны с локацией и событиями

    C_CoordX			=30,
    C_CoordY			=31,
    C_CoordSigma		=32,
    C_PeakTime          =33,
    C_RelTime           =34,
    C_RelPeakTime       =35,
    C_SubHitCount       =36,
    C_IgnoredHitCount   =37,
    C_FirstSubHit       =38,
    C_FirstHit          =39,

    C_OriginalParametric1       =40,
    C_OriginalParametric2       =41,
    C_OriginalParametric3       =42,
    C_OriginalParametric4       =43,

    C_preend,
    C_End              =C_preend-1,
};

namespace traits
{
std::string get_format(chars);
std::string get_wide_name(chars);
std::string get_short_name(chars);
std::string get_unit_name(chars);
bool is_tdd_common_data(chars);
}
}


//////////////////////////////////////////////////////////////////////////
