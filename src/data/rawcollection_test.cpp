#include "StdAfx.h"

#include "data/rawcollection.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

//////////////////////////////////////////////////////////////////////////

using namespace data;

class UNITTEST_SUITE( raw_collection_test )
{
public:
    void setUp ()
    {
        col=shared_ptr<raw_collection>( new raw_collection );

        char string[]="hello";
        col->append_record(string, strchr(string, 0),
                           raw_info(3, 1.0, data::CR_TITLE, "HELLO"));

        for (unsigned index=0; index<20; ++index)
            col->append_record(string, ENDOF(string), raw_info(3, rand()));

        assertEqual(21, col->size());
    }

private:
    shared_ptr<raw_collection> col;
} INSTANCE;


//////////////////////////////////////////////////////////////////////////


