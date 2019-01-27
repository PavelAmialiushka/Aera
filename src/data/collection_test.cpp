#include "StdAfx.h"

#include "collection_writer.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( collection_test )
{
public:

    void setUp()
    {
        col=shared_ptr<data::collection>( new data::collection );

        srand(0);

        data::pwriter_t writer=col->write_access();
        aera::chars chars[]= {aera::C_Time, aera::C_Channel};
        writer->set_typestring(chars, 2);

        boost::array<double, 21> _aes= {0,0,1,7,9,17,11,3,13,15,19,5,1235,24,12348,15648,1846,1464,654,1,0};
        aes.assign(STL_II(_aes));
        for (unsigned index=0; index<aes.size()-1; ++index)
        {
            writer->append_ae_record(&aes[index]);
        }

        boost::array<double, 25> _raws= {0,0,0,0,0,4,6,12,0,14,16,8,10,18,2,123,456,6548,456,548,12,3548,213,156,999};
        raws.assign(STL_II(_raws));
        for (unsigned index=0; index<raws.size(); ++index)
        {
            writer->append_raw_record(0, 0, data::raw_info(0, raws[index], data::CR_RESUME));
        }

        std::sort(STL_II(aes));
        std::sort(STL_II(raws));
    }

    void tearDown()
    {
        aes.clear();
        raws.clear();
    }


private:
    shared_ptr<data::collection> col;
    std::vector<double> aes, raws;
} go;


//////////////////////////////////////////////////////////////////////////



