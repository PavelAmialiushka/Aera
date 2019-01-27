//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "utilites\unittest.h"

#include "utilites\Exception.h"
#include "utilites\log.h"

#include "utilites\cmdline.h"

class UNITTEST_SUITE( cmdline_test )
{
public:
    UNITTEST( test )
    {
        CmdLine line("\"exe.name\" /a /b /c \"filename.exe\" c:\\\"program files\"\\aera\\\"test file\".ext --help /out=outfile");
        assertEqual(7, line.size());
        assertEqual(5, line.option_size());
        assertEqual(2, line.file_size());

        assertEqual("filename.exe", line.file(0));
        assertEqual("c:\\program files\\aera\\test file.ext", line.file(1));
        assertEqual("a", line.option(0));
        assertEqual("", line.option_addenda(0));
        assertEqual("b", line.option(1));
        assertEqual("c", line.option(2));
        assertEqual("-help", line.option(3));
        assertEqual("out", line.option(4));
        assertEqual("outfile", line.option_addenda(4));
    }

    UNITTEST( test_exception )
    {
        CmdLine line("exe filename.exe \"other filename");
        assertEqual("other filename", line.file(1));
        assertEqual(2, line.file_size());
    }

} INSTANCE;
