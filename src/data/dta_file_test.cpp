#include "StdAfx.h"

#include "data\loader.h"
#include "data\dta_saver.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\testfile.h"
#include "utilites\unittest.h"
#include "utilites/strings.h"

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( dta_file_test )
{
public:

    nodes::loader *loader;
    std::string temporal_path;

    void setUp()
    {
        loader=new nodes::loader;
        loader->open( get_test_filename(0) );

        temporal_path = utils::get_temp_filename();
    }

    void tearDown()
    {
        delete loader; loader=0;

        utils::remove_file(temporal_path);
    }

    UNITTEST( test_save )
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);

        data::dta_saver saver(temporal_path);
        data::pslice slice = r->raw->merge(
                             r->ae->merge(
                             r->tdd));
        for(unsigned index=0; index < slice->size(); ++index)
            saver.write(slice, index);

        std::ifstream primary(get_test_filename(0).c_str(), std::ios::binary);
        std::ifstream secondary(temporal_path.c_str(), std::ios::binary);

        std::istreambuf_iterator<char>
        secondary_begin=std::istreambuf_iterator<char>(secondary),
        primary_begin=std::istreambuf_iterator<char>(primary),
        end=std::istreambuf_iterator<char>();

        for (unsigned index=0; index<0x79D && secondary_begin!=end && primary_begin!=end; ++index)
        {
            int a=*primary_begin++;
            int b=*secondary_begin++;
            assertTrueMessage( a==b, strlib::strf("%02x!=%02x, at %08x", a, b, index) );
        }
    }

} INSTANCE;



