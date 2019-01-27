#include "StdAfx.h"

#include "utilites/testfile.h"

#include "data/loader.h"
#include "data/dta_saver.h"

#include "data/sorter.h"

#include "utilites/unittest.h"
#include "utilites/strings.h"

//////////////////////////////////////////////////////////////////////////

using data::pslice;
using nodes::presult;

class UNITTEST_SUITE( loader_slice_test1 )
{
public:
    UNITTEST( dtatest_loader )
    {
        nodes::loader loader;
        loader.open( get_test_filename(0) );

        nodes::presult r= process::get_result<nodes::result>(&loader, true);
        assertEqual(325, r->ae->size());
    }

    UNITTEST( medium_loader )
    {
        nodes::loader loader;
        loader.open( get_test_filename(1) );

        nodes::presult r= process::get_result<nodes::result>(&loader, true);
        assertEqual(3029, r->ae->size());
    }

    UNITTEST( demo_loader )
    {
        nodes::loader loader;
        loader.open( get_test_filename(4) );

        nodes::presult r= process::get_result<nodes::result>(&loader, true);
        assertEqual(73504, r->ae->size());
        assertEqual(2254, r->tdd->size());
    }

    UNITTEST( naf1_loader )
    {
        nodes::loader loader;
        loader.open( get_test_filename(11) );

        nodes::presult r= process::get_result<nodes::result>(&loader, true);
        assertEqual(420, r->ae->size());
        assertEqual(0, r->tdd->size());
    }

    UNITTEST( naf2_loader )
    {
        nodes::loader loader;
        loader.open( get_test_filename(12) );

        nodes::presult r= process::get_result<nodes::result>(&loader, true);
        assertEqual(3, r->ae->size());
        assertEqual(2, r->tdd->size());
    }


} INSTANCE;


class UNITTEST_SUITE( loader_slice_test )
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


    UNITTEST( loader_recreate )
    {
        delete loader;

        loader=new nodes::loader;
        loader->open( get_test_filename(5) );

        nodes::presult r= process::get_result<nodes::result>(loader, true);
        assertEqual(325, r->ae->size());
    }

    UNITTEST( test_save )
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);

        pslice slc=
            r->raw->merge(
                r->ae->merge(
                    r->tdd));

        std::string path_to_save;
        path_to_save.swap( temporal_path );
        {
            data::dta_saver saver(path_to_save);
            for (unsigned index=0; index<slc->size(); ++index)
                saver.write(slc, index);
        }

        std::string filename = get_test_filename(0);
        unsigned filesize = fs::get_filesize(filename);
        assertEqual(filesize,
            fs::get_filesize(path_to_save));

        std::ifstream primary(filename.c_str(), std::ios::binary);
        std::ifstream secondary(path_to_save.c_str(), std::ios::binary);

        std::istreambuf_iterator<char>
            secondary_begin=std::istreambuf_iterator<char>(secondary),
            primary_begin=std::istreambuf_iterator<char>(primary),
            end=std::istreambuf_iterator<char>();

        for (unsigned index=0; index<filesize && secondary_begin!=end && primary_begin!=end; ++index)
        {
            unsigned a=*primary_begin++;
            unsigned b=*secondary_begin++;
            assertTrueMessage( a==b, strlib::strf("%02x!=%02x, at %08x\nComparing `%s` and `%s`)",
                                                  a, b, index, filename, path_to_save));
        }

        path_to_save.swap(temporal_path);
    }

    UNITTEST( ae_slicing )
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);
        pslice slice=r->ae;

        assertEqual(325, slice->size());
        assertDoublesEqual(  12.2352720, slice->get_value(0u, aera::C_Time), 0.00001);
        assertDoublesEqual(4929.0713025, slice->get_value(324, aera::C_Time), 0.00001);
    }

    UNITTEST( tdd_slicing )
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);
        pslice slice=r->tdd;

        assertEqual(987, slice->size());
        assertEqual(8, slice->get_channel_count());

        assertDoublesEqual(   0.010371, slice->get_value(0u, aera::C_Time), 0.00001);
        assertDoublesEqual(  18.0000000, slice->get_value(0u, aera::C_Channel, 0), 0.00001);
        assertDoublesEqual(  52.0000000, slice->get_value(0u, aera::C_Channel, 7), 0.00001);

        assertDoublesEqual( 40.0,  slice->get_value(0u, aera::C_Threashold, 0), 0.01);
        assertDoublesEqual( 15.0,  slice->get_value(0u, aera::C_ASL, 0), 0.01);

        assertDoublesEqual(4930.010038, slice->get_value(986, aera::C_Time), 0.00001);
    }

    UNITTEST( raw_slicing)
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);
        pslice slice=r->raw;
        
        assertEqual(119, slice->size());

        unsigned start=0;
        for (unsigned index=0; index<119; ++index)
        {
            if (data::CR_RESUME==slice->get_raw_record(index).type_)
                start=index;
        }

        assertEqual(106, start);
        assertDoublesEqual(0.0, slice->get_value(start), 0.000001);
        assertEqual(data::CR_ABORT, slice->get_raw_record(118).type_);
    }

    UNITTEST( complex_td_ae )
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);

        pslice slice= r->ae->merge( r->tdd );

        assertEqual(1312, slice->size());
        assertDoublesEqual( 0.010371, slice->get_value(0), 0.00001);
        assertDoublesEqual(12.235272, slice->get_value(3), 0.00001);
        assertDoublesEqual(120.0100375, slice->get_value(29), 0.00001);
        assertDoublesEqual(200.0100370, slice->get_value(50), 0.00001);
        assertEqual(data::TDD, slice->get_type(0));

        pslice other = slice->merge( r->raw );
        assertEqual(1431, other->size());
    }

    UNITTEST( complex_raw_ae )
    {
        nodes::presult r= process::get_result<nodes::result>(loader, true);
        pslice slice=r->raw->merge( r->ae );

        std::vector<aera::chars> chs = slice->get_chars();
        unsigned cnt = std::count(STL_II(chs), aera::C_PeakTime);
        assertEqual( 0, cnt );

        chs = r->ae->get_chars();
        cnt = std::count(STL_II(chs), aera::C_PeakTime);
        assertEqual( 0, cnt);

        assertEqual(444, slice->size());

        assertDoublesEqual(0, slice->get_value(0, aera::C_Time), 1e-6);
        assertDoublesEqual(0, slice->get_value(1, aera::C_Time), 1e-6);
        assertDoublesEqual(0, slice->get_value(106, aera::C_Time), 1e-6);
        assertDoublesEqual(12.235272, slice->get_value(107, aera::C_Time), 1e-6);

        assertEqual(data::RAW, slice->get_type(0));
        assertEqual(data::RAW, slice->get_type(106));
        assertEqual(data::AE, slice->get_type(107));

        assertEqual(data::CR_RESUME, slice->get_raw_record(106).type_);
    }

    UNITTEST( sorting )
    {
        shared_ptr<nodes::sorter> sorter( new nodes::sorter );
        sorter->set_parent(loader);

        nodes::presult r = process::get_result<nodes::result>(sorter.get(), true);
        pslice slice=r->ae;

        assertEqual(325, slice->size());
        assertDoublesEqual(  12.2352720, slice->get_value(0u, aera::C_Time), 0.00001);
        assertDoublesEqual(4929.0713025, slice->get_value(324, aera::C_Time), 0.00001);

//        std::pair<double, double> pr = slice->get_minmax_limits(aera::C_Time);
//        assertDoublesEqual(  12.2352720, pr.first, 0.00001);
//        assertDoublesEqual(4929.0713025, pr.second, 0.00001);

//        pr=slice->get_minmax_limits(aera::C_Gain);
//        assertTrue(std::make_pair(0.0, 0.0)==pr);
    }

} INSTANCE;



