#include "StdAfx.h"

#include "data/TDDCollection.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

using namespace data;
class UNITTEST_SUITE( tdd_collection_test )
{
public:

    tdd_collection *collection;

    void setUp()
    {
        collection=new tdd_collection;
    }

    void tearDown()
    {
        delete collection;
        collection=0;
    }

    void collection_test()
    {
        aera::chars common[]= { aera::C_Time, aera::C_Parametric1 };
        aera::chars channel[]= { aera::C_Channel, aera::C_Threashold };

        collection->set_common_types(common, SIZEOF(common));
        collection->set_channel_types(channel, SIZEOF(channel));

        double data[]= { 1.0, 30.0, 1.0, 40.0, 2.0, 50.0 };
        collection->copy_append_record(data, SIZEOF(data));
        assertEqual(collection->size(), 1);

        double data2[]= { 2.0, 30.1, 1.0, 45.0, 2.0, 55.0 };
        collection->copy_append_record(data2, SIZEOF(data2));
        assertEqual(collection->size(), 2);
    }

    UNITTEST( selection_test )
    {
//		collection_test();
//
//		aera::chars common[]={aera::C_Parametric1, aera::C_Channel, aera::C_Threashold, };
//		data::selection sl=data::selection(STL_AA(common), collection);
//
//		assertEqual(30.0, sl.get_value(0, collection->get_record(0)));
//		assertEqual(1.0,  sl.get_value(1, collection->get_record(0), 1));
//		assertEqual(2.0,  sl.get_value(1, collection->get_record(0), 2));
//		assertEqual(40.0, sl.get_value(2, collection->get_record(0), 1));
//		assertEqual(50.0, sl.get_value(2, collection->get_record(0), 2));
//
//		assertEqual(30.1, sl.get_value(0, collection->get_record(1)));
//		assertEqual(1.0,  sl.get_value(1, collection->get_record(1), 1));
//		assertEqual(2.0,  sl.get_value(1, collection->get_record(1), 2));
//		assertEqual(45.0, sl.get_value(2, collection->get_record(1), 1));
//		assertEqual(55.0, sl.get_value(2, collection->get_record(1), 2));

//		data::pcursor_t cursor=data::pcursor_t(collection->make_cursor());
//		assertTrue(cursor->is_tdd());
//		assertTrue(cursor->is_valid());
//		assertEqual(2, cursor->rest());
//		assertEqual(collection->get_record(0), cursor->ae_data());
//
//		cursor->next();
//		assertTrue(cursor->is_tdd());
//		assertTrue(cursor->is_valid());
//		assertEqual(1, cursor->rest());
//		assertEqual(collection->get_record(1), cursor->ae_data());
//
//		cursor->next();
//		assertTrue(cursor->is_tdd());
//		assertTrue(!cursor->is_valid());
//		assertEqual(0, cursor->rest());
    }
} INSTANCE;
