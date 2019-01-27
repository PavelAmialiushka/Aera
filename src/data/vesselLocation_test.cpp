#include "stdafx.h"

#include "utilites\TestFile.h"

#include "loader.h"
#include "locator.h"
#include "grader.h"

//////////////////////////////////////////////////////////////////////////

#include "LocationSetup.h"
#include "LocationUnit.h"

#include "VesselSetup.h"

#include "Collector.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/unittest.h"

using namespace location;
using namespace data;

#include "fake_node.h"


//class UNITTEST_SUITE( vessel_location_test )
//{
//public:
//    shared_ptr<nodes::loader> loader;
//    shared_ptr<nodes::locator> locator;
//    shared_ptr<nodes::grader> grader;
//    shared_ptr<nodes::fake_node> fakenode;

//    void setUp()
//    {
//        loader.reset( new nodes::loader() );
//        loader->open( get_test_filename(0) );

//        locator.reset( new nodes::locator );
//        locator->set_parent(loader.get());
//        locator->restart();

//        grader.reset( new nodes::grader );
//        grader->set_parent(locator.get());
//    }

//    void tearDown()
//    {
//        loader.reset();
//        locator.reset();
//        grader.reset();
//        fakenode.reset();
//    }

//    void activateLocation(location::Setup loc, std::vector<double> &data)
//    {
//        fakenode.reset ( new nodes::fake_node(&data[0], data.size()) );
//        locator->set_parent(fakenode.get());

//        locator->set_location(loc);

//        grader->set_location( loc.get_id() );
//    }

//    UNITTEST( vessel_location_test2 )
//    {
//        thrd::finished_lock lock( grader.get() );
//    }

//} INSTANCE;
