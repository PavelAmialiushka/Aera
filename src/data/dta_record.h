#pragma once

#include "data/collection_writer.h"
#include "data/traits.h"
#include "data/slice.h"

#include "utilites/streams.h"

namespace data
{

enum Dtachars
{
    C_dta_unknown=0,
    C_dta_Risetime=1,
    C_dta_Countstopeak=2,
    C_dta_Counts=3,
    C_dta_Energy=4,
    C_dta_Duration=5,
    C_dta_Amplitude=6,
    C_dta_RMS=7,
    C_dta_ASL=8,
    C_dta_Gain=9,
    C_dta_Threashold=10,
    C_dta_Preamp=11,
    C_dta_Losthits=12,
    C_dta_Frequency=13,
    C_dta_Time=15,
    C_dta_RMS16=17,
    C_dta_ReverberationFreq=18,
    C_dta_InitiationFreq=19,
    C_dta_SignalStrength=20,
    C_dta_AbsoluteEnergy=21,
    C_dta_PartialPower=22,
    C_dta_CentroidFreq=23,
    C_dta_PeakFreq=24,

    C_dta_Parametric1=256,
    C_dta_Parametric2,
    C_dta_Parametric3,
    C_dta_Parametric4,
    C_dta_Channel,

    C_dta_notpresent,
    C_dta_maximum,
};

namespace traits
{

class DtaDecoder
    : public utils::singleton<DtaDecoder>
{
    friend class utils::singleton<DtaDecoder> ;
    DtaDecoder();
public:

    boost::array<size_t, C_dta_maximum> maps;
    boost::array<aera::chars, C_dta_maximum> mapc;
};

//////////////////////////////////////////////////////////////////////////

aera::chars map_dta_char(Dtachars c);
size_t get_dta_size(Dtachars c);
aera::chars get_dta_factor(Dtachars c, double &fpn);

} // namespace traits

//////////////////////////////////////////////////////////////////////////

class dta_record
{
public:
    dta_record(class dta_loader *);
    dta_record(class dta_saver *);

    void interpret(fs::istream&, data::pwriter_t writer);
    void save(std::vector<char>& buffer, pslice slice, unsigned index);
    void save_to_stream(std::ostream& stream, pslice slice, unsigned index);

    bool read_msg(fs::istream& reader, data::pwriter_t writer);
private:

    void read_value(fs::istream& reader, Dtachars type, double *&data);
    void read_hit_record(fs::istream& reader, data::pwriter_t writer);
    void read_tdd_record(fs::istream&, data::pwriter_t writer);
    void read_complex_record(fs::istream&, data::pwriter_t writer);

    void read_raw_record(fs::istream&, data::pwriter_t writer, const raw_info &info);

    void read_ae_definition(fs::istream& stream);
    void read_ae_definition(fs::istream& stream, data::pwriter_t writer);

    void read_tdd_definition(fs::istream& stream);
    void read_tdd_definition(fs::istream& stream, data::pwriter_t writer);


    void save_value(class vector_writer &writer, Dtachars ch, double value);
    void save_raw_record(std::vector<char>& buffer, pslice slice, unsigned index);
    void save_ae_record(std::vector<char>& buffer, pslice slice, unsigned index);
    void save_tdd_record(std::vector<char>& buffer, pslice slice, unsigned index);

    void read(std::istream & s);
    void write(std::ostream & s);

private:
    unsigned char  id_;
    unsigned short len_;
    bool           submessage_;

    double prevtime_;
    bool test_started_;

    class dta_loader *ldr_;
    class dta_saver *svr_;

    std::vector<Dtachars> dtatypes_;
    std::vector<Dtachars> tdd_common_types_;
    std::vector<Dtachars> tdd_channel_types_;

    unsigned tdd_channel_count_;
    unsigned tdd_record_size_;
    unsigned tdd_common_dta_size_;
    unsigned tdd_channel_dta_size_;

    friend std::ostream &operator <<(std::ostream &, dta_record &self);
};


}
