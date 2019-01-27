//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "utilites\exception.h"

#include "utilites\unittest.h"

class UNITTEST_SUITE( location_test)
{
public:
    UNITTEST( common )
    {
        debug::SourcePos l1=HERE, l11=HERE;
        debug::SourcePos l2=HERE;
        debug::SourcePos a1=l1;
        debug::SourcePos a2; a2=l2;
        assertEqual(l1, l11);
        assertEqual(l1, a1);
        assertEqual(l2, a2);
        assertTrue(l1!=l2);
        assertTrue(!(l1==l2));
        assertTrue((l1 < l2) || (l2 < l1));
    }
#ifdef _DEBUG
    UNITTEST( dbg_location )
    {
        //debug::function lc;
        //assertTrue(std::string(lc.get_string()).find("dbg_location")!=std::string::npos);
    }

#endif
} go;

