#include "StdAfx.h"

#include "data/AECollection.h"
#include "data/aeslice.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

using boost::shared_ptr;

//////////////////////////////////////////////////////////////////////////

aera::chars chars[]= {aera::C_Time, aera::C_Channel, aera::C_Amplitude};

double datas[]=
{
    0.1, 1, 45,
    1234, 2, 50,
    2345, 3, 97,
    1334, 3, 32,    // 3
    7, 3, 4,
    6, 5, 4,
    9, 5, 4,
    10,5,4,
    9.5, 0, 0,
    11.5, 0, 0,
};

using data::ae_collection;

class UNITTEST_SUITE( test_ae_collection )
{
public:
    int size;

    void setUp()
    {
        collection=shared_ptr<ae_collection>( new ae_collection ) ;
        collection->set_typestring(chars, SIZEOF(chars));

        utils::array2 array(datas, SIZEOF(chars), SIZEOF(datas));
        size=array.size();

        for (unsigned index=0; index<array.size(); ++index)
            collection->copy_append_record(array[index]);

        assertEqual(array.size(), collection->size());

        for (unsigned index=0; index<array.size(); ++index)
        {
            const double *copy=collection->get_record(index);
            assertTrue(std::equal(array[index], array[index]+array.width(), copy));
        }
    }

    UNITTEST( simpl_slice_test1 )
    {
        shared_ptr<data::ae_slice> asl1;

        asl1.reset( new data::ae_slice );
        asl1->add_layer(collection.get());
        assertEqual(asl1->size(), collection->size());

        assertDoublesEqual(asl1->get_value(0u, aera::C_Time, 0), 0.1, 1e-6);
        assertDoublesEqual(asl1->get_value(3, aera::C_Time, 0), 1334, 1e-6);
        assertDoublesEqual(asl1->get_value(3, aera::C_Channel, 0), 3, 1e-6);


    }

    UNITTEST(simple_slice_test2)
    {
        shared_ptr<data::ae_slice> asl2;

        asl2.reset( new data::ae_slice(collection.get()) );
        assertEqual(asl2->size(), collection->size());

        assertDoublesEqual(asl2->get_value(0u, aera::C_Time, 0), 0.1, 1e-6);
        assertDoublesEqual(asl2->get_value(3, aera::C_Time, 0), 1334, 1e-6);
        assertDoublesEqual(asl2->get_value(3, aera::C_Channel, 0), 3, 1e-6);
    }

    UNITTEST(two_layer_slice_test)
    {
        shared_ptr<data::ae_slice> asl1;

        asl1.reset( new data::ae_slice );
        asl1->add_layer(collection.get());
        assertEqual(asl1->size(), collection->size());

        aera::chars types[] = {aera::C_CoordX, aera::C_CoordY};
        shared_ptr<data::ae_collection> collection2;
        collection2.reset( new data::ae_collection );
        collection2->set_typestring(types, SIZEOF(types));
        for(unsigned index=0; index < asl1->size(); ++index)
        {
            double temp[2];
            temp[0] = index*10 + 3.3;
            temp[1] = index*100 + 5.5;
            collection2->copy_append_record(temp);
        }
        assertEqual(collection2->size(), asl1->size());

        asl1->add_layer(collection2.get());
        assertEqual(collection2->size(), asl1->size());

        assertDoublesEqual(asl1->get_value(0u, aera::C_Time, 0), 0.1, 1e-6);
        assertDoublesEqual(asl1->get_value(3u, aera::C_Time, 0), 1334, 1e-6);
        assertDoublesEqual(asl1->get_value(3u, aera::C_Channel, 0), 3, 1e-6);

        assertDoublesEqual(asl1->get_value(0u, aera::C_CoordX, 0), 3.3, 1e-6);
        assertDoublesEqual(asl1->get_value(0u, aera::C_CoordY, 0), 5.5, 1e-6);
        assertDoublesEqual(asl1->get_value(7u, aera::C_CoordX, 0), 73.3, 1e-6);
        assertDoublesEqual(asl1->get_value(7u, aera::C_CoordY, 0), 705.5, 1e-6);

        std::vector<unsigned> vector;
        vector.push_back(0);
        vector.push_back(7);
        data::ae_slice *asl_ix = new data::ae_slice;
        asl_ix->set_indexed_source(asl1, vector);

        assertEqual(2, asl_ix->size());
        assertDoublesEqual(asl_ix->get_value(0u, aera::C_Time, 0), 0.1, 1e-6);
        assertDoublesEqual(asl_ix->get_value(0u, aera::C_CoordX, 0), 3.3, 1e-6);
        assertDoublesEqual(asl_ix->get_value(0u, aera::C_CoordY, 0), 5.5, 1e-6);

        assertDoublesEqual(asl_ix->get_value(1u, aera::C_CoordX, 0), 73.3, 1e-6);
        assertDoublesEqual(asl_ix->get_value(1u, aera::C_CoordY, 0), 705.5, 1e-6);
    }


private:

    shared_ptr<ae_collection> collection;
} go;


//////////////////////////////////////////////////////////////////////////
