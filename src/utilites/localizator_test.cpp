#include "stdafx.h"

#include "utilites/unittest.h"
#include "utilites/Localizator.h"

class UNITTEST_SUITE(localizator_test)
{
public:

    void setUp()
    {
        localizator::instance()->set_language("ru");
    }

    UNITTEST( string )
    {
//    assertEqual("�������", _ls("Close"));
//    assertEqual("Abbra", _ls("Abbra"));
//
//    assertEqual("��������", _ls("test"));
//
//    assertEqual("��������\tCtrl", _ls("test\tCtrl"));
//    assertEqual("��������", _ls("test#some english string"));

    }
} INSTANCE;
