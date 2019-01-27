//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "utilites\utilites.h"
#include "utilites\filesystem.h"

#include "utilites\streams.h"
#include "utilites\unittest.h"

#include "utilites/TestFile.h"

class UNITTEST_SUITE( streams_test )
{
public:
    UNITTEST( testBuffer )
    {
        char buffer[] = { 0, 1, 2, 0 ,3 ,0 ,0 ,0 ,8};
        fs::istream s(buffer, SIZEOF(buffer));

        double a = s.read(unsigned char());
        assertDoublesEqual(a, 0, 1e-7);
        assertTrue(s);

        double b = s.read(unsigned char());
        assertDoublesEqual(b, 1, 1e-7);
        assertTrue(s);

        double c = s.read(unsigned short());
        assertDoublesEqual(c, 2, 1e-7);
        assertTrue(s);

        double d = s.read(unsigned int());
        assertDoublesEqual(d, 3, 1e-7);
        assertTrue(s);

        double e = s.read(unsigned short());
        assertTrue(!s);
    }

    UNITTEST( testFile )
    {
        std::string filename = get_test_filename(2);
        fs::istream s(filename);
        unsigned size = fs::get_filesize(filename);

        unsigned sum = 0;
        unsigned index=0;
        while(true)
        {
            char c = s.read_8();
            if (!s) break;

            ++index;
            sum += c;
        }
        assertEqual(index, size);
        assertEqual(0x4D08154, sum);
    }

} INSTANCE;

