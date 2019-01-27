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
//    assertEqual("«акрыть", _ls("Close"));
//    assertEqual("Abbra", _ls("Abbra"));
//
//    assertEqual("проверка", _ls("test"));
//
//    assertEqual("проверка\tCtrl", _ls("test\tCtrl"));
//    assertEqual("проверка", _ls("test#some english string"));

    }
} INSTANCE;
