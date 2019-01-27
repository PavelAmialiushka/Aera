#include "stdafx.h"

#include "utilites\unittest.h"
#include "utilites\singleton.h"


int main(int, char **, char **)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF
                   | _CRTDBG_DELAY_FREE_MEM_DF
                   | _CRTDBG_LEAK_CHECK_DF);

    debug::structured_exception::install_unittest();

    int res = UnitTest::run();

    utils::destroy_singletons();
    return res;
}


