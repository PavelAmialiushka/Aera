//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites\unittest.h"
#include "utilites\application.h"

class UNITTEST_SUITE(application_test)
{
//  UNITTEST( exefilename )
//  {
//    std::string name=utils::get_exefile_name();
//    std::string exe=name.leaf();
//    assertTrue(exe.find(".exe")!=std::string::npos);
//    assertTrue(exe.find("utilites_test")!=std::string::npos);
//  }
//
//	UNITTEST( test_make_local_path )
//	{
//		using utils::make_local_path;
//		assertEqual("file1", make_local_path("C:\\abc\\file1", "c:\\abc\\file2"));
//		assertEqual("..\\file1", make_local_path("C:\\abc\\file1", "c:\\abc\\def\\file2"));
//		assertEqual("def\\file1", make_local_path("C:\\abc\\def\\file1", "c:\\abc\\file2"));
//		assertEqual("..\\..\\..\\abc\\def\\klm\\file1", make_local_path("c:\\abc\\def\\klm\\file1", "c:\\klm\\abc\\def\\file2"));
//
//		assertEqual("d:\\abc\\file1", make_local_path("d:\\abc\\file1", "c:\\abc\\file2"));
//
//		// does not work
//		//assertEqual("\\\\abc\\file1", make_local_path("\\\\abc\\file1", "\\\\def\\file2"));
//	}

} INSTANCE;
