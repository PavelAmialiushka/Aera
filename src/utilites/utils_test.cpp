//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "tokenizer.h"
#include "strings.h"
#include "unittest.h"

class UNITTEST_SUITE( utilites_test)
{
public:
    UNITTEST( test_tokenizer )
    {
//    assert(false);
//    boost::array<const char*, 6> array={"1", "2", "3", "4", "5", "6", };
//    tokenizer token("1 2 3 4  5    6", " ");
//
//    assertTrue(std::equal(token.begin(), token.end(), array.begin()));
//
//    tokenizer token2("", " ");
//    assertTrue(token2.begin()==token2.end());
//
//    std::string test="1  2";
//    assertEqual("1", tokenizer::head(test, " "));
//    assertEqual(" 2", test);
    }

    UNITTEST( test_join )
    {
        assertEqual("abc", tokenizer::join("a", "b", "c"));
        assertEqual("", tokenizer::join("a", "", "c"));

        assertEqual("a", tokenizer::join("a", "b", ""));
        assertEqual("c", tokenizer::join("", "b", "c"));
    }
} INSTANCE;