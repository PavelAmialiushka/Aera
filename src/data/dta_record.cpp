#include "stdafx.h"

#include "dta_loader.h"
#include "dta_record.h"

#include "dta_saver.h"

#include "utilites/Localizator.h"
#include "utilites/strings.h"

#include "vector_reader_writer.h"

namespace data
{

DEFINE_NERROR_OF_TYPE(format_error, incorrect_file, "incorrect data file");

namespace traits
{

//enum _floatSelector { flt=20, };

DtaDecoder::DtaDecoder()
{
    using namespace aera;
    maps.assign((unsigned)-1);
    maps[C_dta_Time]=             6;
    maps[C_dta_Channel]=          1;
    maps[C_dta_Amplitude]=        1;
    maps[C_dta_Duration]=         4;
    maps[C_dta_Counts]=           2;
    maps[C_dta_Frequency]=        2;
    maps[C_dta_Risetime]=         2;
    maps[C_dta_Countstopeak]=     2;
    maps[C_dta_InitiationFreq]=   2;
    maps[C_dta_ReverberationFreq]=2;
    maps[C_dta_CentroidFreq]=     2;
    maps[C_dta_PeakFreq]=         2;
    maps[C_dta_Energy]=           2;
    maps[C_dta_AbsoluteEnergy]=   flt;
    maps[C_dta_SignalStrength]=   4;
    maps[C_dta_ASL]=              1;
    maps[C_dta_RMS]=              1;
    maps[C_dta_RMS16]=            2;
    maps[C_dta_PartialPower]=     2;
    maps[C_dta_Threashold]=       1;
    maps[C_dta_Parametric1]=      3;
    maps[C_dta_Parametric2]=      3;
    maps[C_dta_Parametric3]=      3;
    maps[C_dta_Parametric4]=      3;
    maps[C_dta_Gain]=             1;
    maps[C_dta_Losthits]=         1;
    maps[C_dta_Preamp]=           1;
    maps[C_dta_notpresent]=       0;

    mapc.assign(static_cast<aera::chars>(-1));
    mapc[C_dta_Time]=             C_Time;
    mapc[C_dta_Channel]=          C_Channel;
    mapc[C_dta_Amplitude]=        C_Amplitude;
    mapc[C_dta_Duration]=         C_Duration;
    mapc[C_dta_Counts]=           C_Counts;
    mapc[C_dta_Frequency]=        C_Frequency;
    mapc[C_dta_Risetime]=         C_Risetime;
    mapc[C_dta_Countstopeak]=     C_Countstopeak;
    mapc[C_dta_InitiationFreq]=   C_InitiationFreq;
    mapc[C_dta_ReverberationFreq]=C_ReverberationFreq;
    mapc[C_dta_Energy]=           C_Energy;
    mapc[C_dta_AbsoluteEnergy]=   C_AbsoluteEnergy;
    mapc[C_dta_SignalStrength]=   C_SignalStrength;
    mapc[C_dta_ASL]=              C_ASL;
    mapc[C_dta_RMS]=              C_RMS;
    mapc[C_dta_RMS16]=            C_RMS;
    mapc[C_dta_PartialPower]=     C_PartialPower1;
    mapc[C_dta_Threashold]=       C_Threashold;
    mapc[C_dta_Parametric1]=      C_OriginalParametric1;
    mapc[C_dta_Parametric2]=      C_OriginalParametric2;
    mapc[C_dta_Parametric3]=      C_OriginalParametric3;
    mapc[C_dta_Parametric4]=      C_OriginalParametric4;
    mapc[C_dta_Gain]=             C_Gain;
    mapc[C_dta_Losthits]=         C_Losthits;
    mapc[C_dta_Preamp]=           C_Preamp;
    mapc[C_dta_CentroidFreq]=     C_CentroidFreq;
    mapc[C_dta_PeakFreq]=         C_PeakFreq;
}

size_t get_dta_size(Dtachars c)
{
    size_t result=DtaDecoder::instance()->maps[static_cast<int>(c)];
    assert(result!=(unsigned)-1);
    return result;
}

Dtachars map_char_dta(aera::chars c)
{
    if (c==aera::C_Hits) return C_dta_notpresent;
    unsigned index=std::distance(
                       DtaDecoder::instance()->mapc.begin(),
                       std::find(STL_II(DtaDecoder::instance()->mapc), c));

    assert(index<C_dta_maximum);

    return static_cast<Dtachars>(index);
}

aera::chars map_dta_char(Dtachars c)
{
    DtaDecoder *dta=DtaDecoder::instance();
    aera::chars result=static_cast<aera::chars>(
                           dta->mapc[static_cast<int>(c)]
                       );
    debug::Assert<incorrect_file>(result!=-1, HERE);
    return result;
}

double get_dta_factor(Dtachars c)
{
    switch (c)
    {
    case C_dta_Time:
        return 0.25*1e-6;
    case C_dta_RMS:
        return 1.0/20;
    case C_dta_RMS16:
        return  1.0/5000;
#pragma warning (push)
#pragma warning (disable : 4244)
    case C_dta_SignalStrength:
        return 3.05;
    case C_dta_AbsoluteEnergy:
        return 0.931e-3;
    case C_dta_Parametric1:
    case C_dta_Parametric2:
    case C_dta_Parametric3:
    case C_dta_Parametric4:
        return 1.0/838906;
        break;
#pragma warning (pop)
    }
    return 1.0;
}

aera::chars get_dta_factor(Dtachars c, double &value)
{
    value=value*get_dta_factor(c);
    aera::chars result=DtaDecoder::instance()->mapc[static_cast<int>(c)];
    assert(result!=-1);
    return result;
}

} // namespace traits


//////////////////////////////////////////////////////////////////////////

dta_record::dta_record(dta_loader *ldr)
    : prevtime_(0), test_started_(false), ldr_(ldr), svr_(0), submessage_(false)
    , tdd_record_size_(0)
    , tdd_common_dta_size_(0)
    , tdd_channel_dta_size_(0)
    , tdd_channel_count_(0)
{
}

dta_record::dta_record(dta_saver *svr)
    : prevtime_(0), test_started_(false), svr_(svr), ldr_(0), submessage_(false)
    , tdd_record_size_(0)
    , tdd_common_dta_size_(0)
    , tdd_channel_dta_size_(0)
    , tdd_channel_count_(0)
{
}


//void dta_record::read(std::istream &stream)
//{
//    if (!stream.read((char *)&len_, 2))
//        return;

//    stream.read((char *)&id_, 1);

//    if (self.len_>=1 && stream.gcount())
//    {
//        buffer_.resize(len_-1);

//        if (len_ == 1)
//            return;

//        stream.read(&buffer_[0], len_-1);
//        if (stream.gcount()==len_-1)
//            return;
//    }

//    throw format_error(HERE);
//}

void dta_record::write(std::ostream &stream)
{
//    if (!submessage_)
//    {
//        len_=1+buffer_.size();
//        stream.write((char *)&len_, 2);
//        stream.write((char *)&id_, 1);
//        if (buffer_.size())
//            stream.write(&buffer_[0], buffer_.size());
//    }
//    buffer_.clear();
}


void dta_record::read_tdd_definition(fs::istream& stream)
{
    // 1 number of AE char in set
    // 1 first CHID
    // ...
    // n last CHID

    // 1 number of PID in set
    // 1 first PID
    // ...
    // n last PID

    tdd_common_types_.clear();
    tdd_channel_types_.clear();

    tdd_common_types_.push_back(C_dta_Time);
    tdd_channel_types_.push_back(C_dta_Channel);

    tdd_record_size_ = 0;
    tdd_channel_dta_size_ = traits::get_dta_size(C_dta_Channel);
    // данные для каждого из каналов
    unsigned psize=stream.read_8();

    for (unsigned index=0; index<psize; ++index)
    {
        Dtachars ch = (Dtachars)
                (stream.read_8());
        tdd_channel_types_.push_back(ch);
        tdd_channel_dta_size_ += traits::get_dta_size(ch);
    }

    tdd_common_dta_size_ = traits::get_dta_size(C_dta_Time);
    // общие для всех данные
    unsigned size=stream.read_8();
    for (unsigned index=0; index<size; ++index)
    {
        Dtachars ch = Dtachars(C_dta_Parametric1-1+stream.read_8());
        tdd_common_types_.push_back(ch);
        tdd_common_dta_size_ += traits::get_dta_size(ch);
    }
}

void dta_record::read_tdd_definition(fs::istream& stream, data::pwriter_t writer)
{
    read_tdd_definition(stream);

    // преобразуем характеристики

    std::vector<aera::chars> trk(tdd_common_types_.size());
    std::transform(STL_II(tdd_common_types_), trk.begin(), &traits::map_dta_char);

    std::vector<aera::chars> trk2(tdd_channel_types_.size());
    std::transform(STL_II(tdd_channel_types_), trk2.begin(), &traits::map_dta_char);

    writer->set_tdd_common(&trk[0], tdd_common_types_.size());
    writer->set_tdd_channel(&trk2[0], tdd_channel_types_.size());
}

void dta_record::read_ae_definition(fs::istream& stream)
{
    dtatypes_.push_back(C_dta_Time);
    dtatypes_.push_back(C_dta_Channel);

    unsigned size=stream.read_8();
    for (unsigned index=0; index<size; ++index)
    {
        Dtachars i=(Dtachars)stream.read_8();
        dtatypes_.push_back(i);
    }

    unsigned psize=stream.read_8();
    for (unsigned index=0; index<psize; ++index)
    {
        dtatypes_.push_back(Dtachars(C_dta_Parametric1+index));
    }
}

void dta_record::read_ae_definition(fs::istream& stream, data::pwriter_t writer)
{
    read_ae_definition(stream);

    // преобразуем в свои характеристики

    std::vector<aera::chars> types;
    for (unsigned index=0; index<dtatypes_.size(); ++index)
    {
        aera::chars c=traits::map_dta_char(dtatypes_[index]);
        types.push_back(c);
    }

    types.push_back(aera::C_Hits);
    aera::chars *cs=&types[0];
    writer->set_typestring(cs, types.size());
}

void dta_record::read_hit_record(fs::istream& stream, data::pwriter_t writer)
{
    //double data_array[256];
    //double* data=data_array;
    double *record=writer->allocate_ae_record(dtatypes_.size()+1);
    double *data=record;

    *data++=stream.read_48();
    traits::get_dta_factor(C_dta_Time, data[-1]);
    prevtime_=data[-1];

    char ch=stream.read_8();
    *data++=ch;
    traits::get_dta_factor(C_dta_Channel, data[-1]);
    ldr_->channels_.at(ch-1)=true;

    std::vector<Dtachars>::const_iterator index=dtatypes_.begin()+2,
                                          end=dtatypes_.end();
    for (; index!=end; ++index)
    {
        switch (traits::get_dta_size(*index))
        {
        case 1:
            *data++=stream.read_8();
            break;
        case 2:
            *data++=stream.read_16();
            break;
        case 3:
            *data++=stream.read_24();
            break;
        case 4:
            *data++=stream.read_32();
            break;
        case traits::flt:
            *data++=stream.read(float());
            break;
        case 6:
        {
            *data++=stream.read_48();
            break;
        }
        default:
            throw incorrect_file(HERE);
        }
        traits::get_dta_factor(*index, data[-1]);
    }

    *data++=1.0; // hits
    writer->append_ae_record(record);
}

void dta_record::read_value(fs::istream& stream, Dtachars type, double *&data)
{
    switch (traits::get_dta_size(type))
    {
    case 1:
        *data++=stream.read_8();
        break;
    case 2:
        *data++=stream.read_16();
        break;
    case 3:
        *data++=stream.read_24();
        break;
    case 4:
        *data++=stream.read_32();
        break;
    case traits::flt:
        *data++=stream.read(float());
        break;
    case 6:
    {
        *data++=stream.read_48();
        break;
    }
    default:
        throw incorrect_file(HERE);
    }
    traits::get_dta_factor(type, data[-1]);
}

void dta_record::read_tdd_record(fs::istream& stream, data::pwriter_t writer)
{
    if (!tdd_record_size_)
    {
        tdd_channel_count_ = (len_ - tdd_common_dta_size_) / tdd_channel_dta_size_;
        writer->set_channel_count(tdd_channel_count_);

        tdd_record_size_ = tdd_common_types_.size()
                + tdd_channel_types_.size() * tdd_channel_count_;
    }

    if ( len_ != tdd_common_dta_size_ + tdd_channel_count_ * tdd_channel_dta_size_)
    {
        stream.skip(len_);
        return;
    }

    double *data=writer->allocate_tdd_record(tdd_record_size_);
    double *base = data;

    *data++=stream.read_48();
    traits::get_dta_factor(C_dta_Time, data[-1]);
    prevtime_ = data[-1];

    for (unsigned index=1; index<tdd_common_types_.size(); ++index)
    {
        read_value(stream, tdd_common_types_[index], data);
    }

    for(int count = tdd_channel_count_; count-->0; )
    {
        for (unsigned index=0; index<tdd_channel_types_.size(); ++index)
        {
            read_value(stream, tdd_channel_types_[index], data);
        }
    }

    assert(!!stream);

    unsigned saved = data - base;
    assert(saved == tdd_record_size_);

    writer->append_tdd_record(base);
}


void dta_record::read_complex_record(fs::istream& stream, data::pwriter_t writer)
{
    debug::Assert<fault>(len_>3, HERE);

    submessage_=true;
    while(read_msg(stream, writer))
    {
    }
    submessage_=false;
}

void dta_record::read_raw_record(fs::istream& stream,
                                   data::pwriter_t writer,
                                   const raw_info &info)
{
    // HACK
    const_cast<raw_info &>(info).submessage_=submessage_;

    if (len_)
    {
        const char *ptr=stream.read_buf(len_);
        debug::Assert<fault>(ptr, HERE);

        writer->append_raw_record(ptr, ptr+len_, info);
    }
    else
    {
        writer->append_raw_record(0, 0, info);
    };
}

bool dta_record::read_msg(fs::istream &stream, pwriter_t writer)
{
    len_ = stream.read_16();
    const char* data = stream.reserve(len_);
    if (!data) return false;

    id_ = stream.read_8();
    if (!stream) return false;

    // корректируем на размер идентификатора
    len_ -= 1;

    interpret(stream, writer);

    return !!stream;
}

void dta_record::interpret(fs::istream& stream, data::pwriter_t writer)
{
    using namespace aera::traits;
    try
    {
        switch (id_)
        {
        case 0:
            debug::Assert<incorrect_file>(false, HERE);
        case 1:  // hit data
            read_hit_record(stream.extract(len_), writer);
            stream.skip(len_);
            break;
        case 2:  // time driven data //
        case 3:  // user forced sample data //
            read_tdd_record(stream, writer);
            break;
        case 5:  // HD definition //
        {
            read_ae_definition(stream.extract(len_), writer);
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_));
            break;
        }
        case 6:  // TDD Definition //
        {
            read_tdd_definition(stream.extract(len_), writer);
            read_raw_record(stream, writer,
                              raw_info(id_, prevtime_));
            break;
        }
        case 7:  // user coments/test label //
        {
            const char* data = stream.reserve(len_);
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_TITLE,
                                     len_
                                         ? std::string(data, len_)
                                         : std::string()));
            break;
        }
        case 8:  // Continued file //
        case 11: // unknown data //
        case 15: // Abort data acquisition //
        {
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_ + 0.000001, data::CR_ABORT));
            break;
        }
        case 16: // Alarms //
        case 24: // HDT //
        case 25: // HLT
        case 26: // PDT
        case 27: // Sampling interval
        case 28: // Alarm definition
        case 29: // AE filter definition
        case 30: // delta-t ae filter definition
        case 37: // ini-file protect-password
        case 38: // test information (INI & DTA)
        {
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_));
            break;
        }
        case 41:   // product definition
        {
            const char* data = stream.reserve(len_);
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_SOFTWARE,
                                     std::string(&data[3], len_-5)));
            break;
        }
        case 42:   // hardware complex setup
        {
            fs::istream local_stream = stream.extract(len_);
            char null=local_stream.read_8();
            short version=local_stream.read_16();

            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_COMPLEX_RECORD));

            read_complex_record(local_stream, writer);

            break;
        }

        case 43: // graph definition
        case 44: // location definition
        case 45: // acquishion control information
        case 46: // autorun
        case 48: // filtered file information
        case 49: // special product-specific information
        case 59: // turn-off alarm command
        {
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_));
            break;
        }

        case 99: // time and date of test start
        {
            const char* data = stream.reserve(len_);
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_DATE,
                                     std::string(data, len_)
                                     )
                            );
            break;
        }

        case 100: // begin setup
        case 101: // end of setup
        case 102: // set demand rate
        case 106: // define group
        case 133: // Pulser rate
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_));
            break;
        case 211: // timemark //
        {
            fs::istream local_stream = stream.extract(len_);

            prevtime_=local_stream.read_48();
            traits::get_dta_factor(C_dta_Time, prevtime_);

            int length = local_stream.read_16();
            if (local_stream && length >= 3)
            {
                int gain=local_stream.read_8();
                int channel=local_stream.read_8();
                int threshold=local_stream.read_8();

                std::string temp=strlib::strf(
                                     (_ls("Channel") +" %d, "+
                                      _ls("Gain")+"=%d, "+
                                      _ls("Threshold")+"=%d").c_str(),
                                     channel, gain, threshold);
                read_raw_record(stream, writer,
                                raw_info(id_, prevtime_, data::CR_THRESHOLD, temp.c_str()));
            } else
            {
                read_raw_record(stream, writer,
                                raw_info(id_, prevtime_, data::CR_TIMEMARK, "Timemark"));
            }

            break;
        }
        case 128: // resume or start test //
        {
            test_started_=true;

            fs::istream ls = stream.extract(6);
            prevtime_=ls.read_48();

            traits::get_dta_factor(C_dta_Time, prevtime_);

            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_RESUME, "Resume"));

            break;
        }
        case 129: // stop test //
        {
            fs::istream ls = stream.extract(6);
            prevtime_=ls.read_48();

            traits::get_dta_factor(C_dta_Time, prevtime_);

            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_STOP, "Stop"));

            break;
        }
        case 130: // pause test //
        {
            fs::istream ls = stream.extract(6);
            prevtime_=ls.read_48();

            traits::get_dta_factor(C_dta_Time, prevtime_);

            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_, data::CR_PAUSE, "Pause"));

            break;
        }
        case 173: // fw
        {
            const char* data = stream.reserve(len_);
            if (data[0]==1)
            {
                //record_.erase(record_.begin());
                //read_hit_record(true);
                stream.skip(len_);
            }
            else
            {
                read_raw_record(stream, writer,
                                raw_info(id_, prevtime_));
            }
            break;
        }
        default:
        {
            read_raw_record(stream, writer,
                            raw_info(id_, prevtime_));
            break;
        }
        };
    }
    LogExceptionPath("dta_stream::lookup_message");
}

void dta_record::save(std::vector<char>& buffer, pslice slice, unsigned index)
{
    switch (slice->get_type(index))
    {
    case RAW:
        save_raw_record(buffer, slice, index);
        break;
    case AE:
        save_ae_record(buffer, slice, index);
        break;
    case TDD:
        save_tdd_record(buffer, slice, index);
    }
}

void dta_record::save_to_stream(std::ostream &stream, pslice slice, unsigned index)
{
    std::vector<char> buffer;
    save(buffer, slice, index);

    if (!submessage_)
    {
        len_=1 + buffer.size();
        stream.write((char *)&len_, 2);
        stream.write((char *)&id_, 1);
        if (buffer.size())
            stream.write(&buffer[0], buffer.size());
    }
}


void dta_record::save_raw_record(std::vector<char>& buffer, pslice slice, unsigned index)
{
    raw_record record=slice->get_raw_record(index);
    id_=record.id_;
    submessage_=record.submessage_;

    vector_writer writer(buffer);
    if (record.data_size_)
        writer.write(record.data_size_, record.data_);

    fs::istream stream(buffer);

    switch (id_)
    {
    case 5:
        read_ae_definition(stream);
        break;
    case 6:
        read_tdd_definition(stream);
        break;
    }
}

void dta_record::save_value(vector_writer &writer, Dtachars ch, double value)
{
    writer.write(
        0.5 + value / traits::get_dta_factor(ch),
        traits::get_dta_size(ch));
}

void dta_record::save_ae_record(std::vector<char>& buffer, pslice slice, unsigned index)
{
    ae_record data=&slice->get_value(index);

    vector_writer writer(buffer);

    id_=1;
    for (unsigned index=0; index<dtatypes_.size(); ++index)
    {
        save_value(writer, dtatypes_[index], data[index]);
    }
}

//////////////////////////////////////////////////////////////////////////

void dta_record::save_tdd_record(std::vector<char>& buffer, pslice slice, unsigned index)
{
    const double *data=&slice->get_value(index);

    vector_writer writer(buffer);

    id_=2;

    unsigned jindex=0;
    for (unsigned j=0; j<tdd_common_types_.size(); ++j, ++jindex)
    {
        save_value(writer, tdd_common_types_[j], data[jindex]);
    }

    unsigned channel_count =slice->get_channel_count();
    for (unsigned ch=0; ch<channel_count; ++ch)
    {
        for (unsigned j=0; j<tdd_channel_types_.size(); ++j, ++jindex)
        {
            save_value(writer, tdd_channel_types_[j], data[jindex]);
        }
    }
}

}
