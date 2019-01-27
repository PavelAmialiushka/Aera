#include "stdafx.h"

#include "naf_loader.h"
#include "naf_record.h"

#include "utilites/Localizator.h"
#include "utilites/strings.h"

namespace data
{

DEFINE_NERROR_OF_TYPE(format_error, incorrect_file, "incorrect naf file");


//////////////////////////////////////////////////////////////////////////

static aera::chars default_record_contents[] =
{
    aera::C_Time,
    aera::C_Channel,
    aera::C_Amplitude,
    aera::C_Threashold,
    aera::C_ASL,
    aera::C_RMS,
    aera::C_Risetime,
    aera::C_Duration,
    aera::C_Countstopeak,
    aera::C_Counts,
    aera::C_Energy,
    aera::C_Hits,
};

naf_record::naf_record(naf_loader *ldr)
    : prevtime_(0), ldr_(ldr), basetime_ae_(0), basetime_tdd_(0),
      naftypes_initiated_(0),
      paramtypes_initiated_(0)
{
}

void naf_record::read_buffer(std::istream& s, unsigned size)
{
    buffer_.resize(size);
    s.read(static_cast<char*>(&buffer_[0]), size);
}

std::istream &operator >>(std::istream &stream, naf_record &self)
{

    int toRead = self.len_ - 8;
    if (toRead>=0 && stream.gcount())
    {
        self.buffer_.resize(toRead);

        stream.read(&self.buffer_[0], toRead);
        if (stream.gcount()==toRead)
            return stream;
    }

    throw format_error(HERE);
}

void naf_record::scan_for_basetime(fs::istream &stream)
{
    while(stream)
    {
        int len = stream.read_32();
        int id = stream.read_32();
        if (!stream) break;

        fs::istream record = stream.extract(3*4 + 8);
        if (stream && (id == 0 || id == 2))
        {
            int count = record.read_32(); // records count
            int _sz = record.read_32(); // record size
            int _c = record.read_32(); // channel or PAE
            __int64 time = record.read_64();

            if (count)
            {
                switch(id)
                {
                case 0: // ae
                    if (!basetime_ae_ || basetime_ae_ > time)
                        basetime_ae_ = time;
                    break;
                case 2: // tdd
                    if (!basetime_tdd_ || basetime_tdd_ > time)
                        basetime_tdd_ = time;
                    break;
                }
            }
        }

        if (len <= 8) break; // чтобы не зависнуть
        stream.skip(len-8);
    }

    if (fabs((basetime_ae_ - basetime_tdd_)/1e9) < 500)
    { // будем считать, что времена одинаковые

        basetime_ae_ = basetime_tdd_ = std::min(basetime_ae_, basetime_tdd_ );
    }

    stream.seek(0);
}

void naf_record::append_raw_record(data::pwriter_t writer, raw_info const& info)
{
    writer->append_raw_record(0, 0, info);
}

void naf_record::interpret_hit_record_table(fs::istream& stream, data::pwriter_t writer)
{
    if (!naftypes_initiated_)
    {
        naftypes_initiated_ = 1;
        naftypes_.insert(naftypes_.begin(), STL_AA(default_record_contents) );
        aera::chars* beg = &naftypes_[0];
        writer->set_typestring(beg, naftypes_.size());

        append_raw_record(writer, raw_info(-1, 0, CR_RESUME, "Resume"));
    }

    double *base=writer->allocate_ae_record(naftypes_.size()+1);
    double *data=base;

    int channel = stream.read_32();
    __int64 time = stream.read_64();

    if (basetime_ae_ == 0)
        basetime_ae_ = time;

    prevtime_ = (time - basetime_ae_) / 1e9;
    *data++ = prevtime_;    // time
    *data++ = channel;      // channel

    double amp = *data++ = stream.read_double(); // amp
    double prev = stream.read_double(); // Pprev
    double th = amp - prev;
    *data++ = th;

    *data++ = stream.read_double(); // ASL
    *data++ = stream.read_double(); // RMS
    *data++ = stream.read_32(); // RiseTime
    *data++ = stream.read_32(); // Duration
    *data++ = stream.read_32(); // CntP
    *data++ = stream.read_32(); // Count

    double energy = stream.read_double(); // Energy
    energy = pow(10, energy/20); // из дЅ (энергетических) к джоул€м
    *data++ = energy;

    *data++=1.0; // hits

    // пропускаем хиты-дубли
    if (loaded_records_.contains( base ))
        return;

    int ch_id = channel - 1;
    debug::Assert<>( ch_id >=0 && (unsigned)ch_id < ldr_->channels_.size(), "incorrect channel", HERE );
    ldr_->channels_.at(ch_id)=true;

    loaded_records_.append( base );
    writer->append_ae_record(base);
}

void naf_record::interpret_tdd_record_table(fs::istream& stream, data::pwriter_t writer)
{
    if (!paramtypes_initiated_)
    {
        paramtypes_initiated_ = 1;
        paramtypes_.push_back( aera::C_Time );

        int param_count = (ldr_->record_size_ - 12) / 8;
        for(int index=0; index< param_count; ++index)
            paramtypes_.push_back( static_cast<aera::chars>(aera::C_Parametric1 + index) );

        writer->set_tdd_common(&paramtypes_[0], paramtypes_.size());
        writer->set_tdd_channel(0, 0);
    }

    double *record=writer->allocate_tdd_record(paramtypes_.size());
    double *data=record;

    int channel = stream.read_32();
    __int64 time = stream.read_64();

    if (basetime_tdd_ == 0)
        basetime_tdd_ = time;

    prevtime_ = (time - basetime_tdd_) / 1e9;
    *data++ = prevtime_;            // time
    *data++ = stream.read_double(); // value

    writer->append_tdd_record(record);
}

void naf_record::interpret(data::pwriter_t writer)
{
//    try
//    {
//        switch (id_)
//        {
//        case 0:  // hit data
//            interpret_hit_record_table(streawriter);
//            break;
//        case 2:  // time driven data //
//            interpret_tdd_record_table(writer);
//            break;
//        };
//    }
//    LogExceptionPath("naf_reader::lookup_message");
}

}

