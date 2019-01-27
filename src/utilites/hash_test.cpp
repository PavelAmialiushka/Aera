#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

//////////////////////////////////////////////////////////////////////////

#include "hash.h"
#include "hash_set.h"
#include "hash_map.h"

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( hash_table_test )
{
public:

    UNITTEST( simple_test )
    {
        hash_set<int> ht(1);
        ht.insert(1);
        ht.find(1);
        ht.clear();
    }

    UNITTEST( table_test )
    {
        hash_set<int> ht(1);

        for (unsigned index=0; index<100; ++index)
            ht.insert(index);

        assertEqual( 100, ht.size() );

        for (unsigned index=0; index<100; ++index)
            assertTrue( ht.find(index)!=ht.end() );

        assertTrue( ht.find(101)==ht.end() );

        const hash_set<int> &cr=ht;
        assertTrue( cr.find(5)!=cr.end() );
    }

    UNITTEST( special_cast_test )
    {
        unsigned spec[]=
        {
            617246150  ,            1280154175 ,            136091372  ,            1285308699 ,            332213954  ,
            2057023397 ,            2317396138 ,            1329886318 ,            579434240  ,            2405941407 ,
            3844063261 ,            133809423  ,            3726136048 ,            3793017389 ,            4037993279 ,
            3864327774 ,            2973139839 ,            2140674228 ,            2332049405 ,            2048420009 ,
            3511682543 ,            4133141961 ,            2001834168 ,            2483099351 ,            1647790910 ,
            2663681581 ,            1146499119 ,            1788437092 ,            559996138  ,            217450016  ,
            860827176  ,            1906595664 ,            2982309937 ,            2299564195 ,            202135887  ,
            1927916301 ,            475051027  ,            2819748607 ,            3461329199 ,            2682996369 ,
            2552634386 ,            2348217157 ,            4215932660 ,            1706773390 ,            175589045  ,
            691655297  ,            2568686042 ,            3163867954 ,            3379361896 ,            694075733  ,
            4278444948 ,            1365736105 ,            1424203334 ,            532636632  ,            4267377466 ,
            2413781753 ,            3877680857 ,            3568578380 ,            2236106105 ,            1406658513 ,
            2056822955 ,            1717417329 ,            743439980  ,            3922246026 ,            1696810499 ,
            2627421851 ,            880861276  ,            3263830447 ,            152990229  ,            756986653  ,
            754050409  ,            34385808   ,            4039717290 ,            1192091339 ,            3327515117 ,
            3679492573 ,            2781641942 ,            195035830  ,            831404181  ,            2729935641 ,
            878009653  ,            2965814157 ,            4052994668 ,            3234080012 ,            916405514  ,
            4146994808 ,            3816423274 ,            1569885251 ,            4086419929 ,            3217544255 ,
            1019805817 ,            1548321011 ,            624736307  ,            2252035894 ,            2488458865 ,
            69422782   ,            548467170  ,            562694852  ,            3517147559 ,            748420560  ,
            3444769334 ,            3605621890 ,            1382397340 ,            1551414383 ,            3435141878 ,
            716027096  ,            584060329  ,            1872495334 ,            3648324941 ,            3092400333 ,
            658235250  ,            3856832967 ,            2457300748 ,            313338628  ,            3898179608 ,
            1254926502 ,            1627723151 ,            2306901952 ,            1230961160 ,            1105217970 ,
            1137557373 ,            3851695176 ,            1391822797 ,            3440256109 ,            718268129  ,
            1334489089 ,            2635820877 ,            3188263036 ,            121190696  ,            678218371  ,
            333583515  ,            3740866888 ,            757682833  ,            947487050  ,            1814337629 ,
            2726652774 ,            3718392372 ,            1703400707 ,            2025246722 ,            822568743  ,
            4141576827 ,            2025847152 ,            2450242973 ,            2811969955 ,            3672410928 ,
            3772085187 ,            3181471020 ,            2844183859 ,            3331966399 ,            3040048919 ,
            1226502571 ,            2872593467 ,            1495986443 ,            3746367395 ,            2891633684 ,
            396461507 ,            3173955405,            4049956148,            2012311901,            3720699256,
            2218900554,            474108237 ,            740366879 ,            2994468023,            1345638266,
            3801477772,            2363845868,            2058111625,            2379428266,            2341031617,
            2294159164,            1399960674,            2796205856,            600418456 ,            689091229 ,
            3376034377,            1134116740,            2138198466,            3762742654,            3078860037,
            3922695722,            1044693869,            2240854343,            2464983237,            3810777659,
            1506494661,            536126170 ,            897548441 ,            3114769911,            886818651 ,
            1439307522,            1335082588,            2279611451,            2685436096,            3729442812,
            3032772662,
            // same values
            3032772662,            3032772662,            3032772662,
        };

        unsigned size = sizeof(spec)/sizeof(*spec);
        hash_set<unsigned> ht;
        for(unsigned index=0; index < size; ++index)
            ht.insert(spec[index]);

        assertEqual(ht.size(), size);
    }

    struct string_hash
    {
        unsigned operator()(std::string s) const
        {
            return s[0]-'a';
        }
    };

    UNITTEST( string_table_test )
    {
        hash_set<std::string, string_hash> ht(1);

        ht.insert("alpha");
        ht.insert("beta");
        ht.insert("auto");
        ht.insert("cappa");
        ht.insert("omicron");

        assertEqual(5, ht.size());

        assertTrue( ht.find("alpha")!=ht.end() );
        assertTrue( ht.find("beta")!=ht.end() );
        assertTrue( ht.find("auto")!=ht.end() );
        assertTrue( ht.find("cappa")!=ht.end() );
        assertTrue( ht.find("omicron")!=ht.end() );

        ht.erase("alpha");

        assertTrue( ht.find("alpha")==ht.end() );
        assertTrue( ht.find("beta")!=ht.end() );
        assertTrue( ht.find("auto")!=ht.end() );
        assertTrue( ht.find("cappa")!=ht.end() );
        assertTrue( ht.find("omicron")!=ht.end() );

        ht.insert("utta");
        ht.insert("chha");
        ht.insert("atta");
        ht.insert("btta");

        ht.clear();
    }

    struct bad_hash
    {
        unsigned operator()(const int& val) const
        {
            return 0;
        }
    };

    UNITTEST( bad_hash_test)
    {
        hash_set<int, bad_hash> ht(0);
        for(unsigned index=0; index < 1000; ++index)
            ht.insert( index );

        for (unsigned index=0; index<1000; ++index)
            assertTrue( ht.find(index)!=ht.end() );

        assertTrue(ht.find(1001)==ht.end());

        assertEqual(1000, ht.size());
    }

    struct double_hash
    {
        unsigned operator()(const double &dbl) const
        {
            return 1;
        }
    };

    UNITTEST( performance_test )
    {
        assertBlock
        {
            double start=0.0;
            hash_set<double, double_hash> ht(1);
            for (unsigned index=0; index<10000; ++index)
                ht.insert( start+=1.1 );

            assertDoublesEqual(1.0, ht.collision_factor(), 0.0001);
            ht.clear();
        }
        assertPerformance(24.0);
    }

    struct string_hash2
    {
        size_t operator()(const std::string &_s) const
        {
            return SuperFastHash(_s.c_str(), _s.size());
        }
    };

    static std::string gen_rand_string()
    {
        std::string result(' ', rand()%20);
        for (unsigned index=0; index<result.size(); ++index)
        {
            result[index]='a'+rand() % 26;
        }
        return result;
    }

    UNITTEST( erase_test )
    {
        hash_set<std::string, string_hash2> ht;

        for (unsigned index=0; index<100; ++index)
        {
            srand(index);
            for (unsigned counter=1+rand(); counter; --counter)
            {
                ht.insert( gen_rand_string() );
            }

            ht.clear();
            assertTrueMessage(0==ht.size(),
                              strlib::strf("hash_set::clear() [size()==%d] failed at srand(%d)", ht.size(), index));
        }
    }

    UNITTEST( hash_map_big )
    {
        hash_map<int, int> map;

        for(unsigned index=0; index < 1000; ++index)
            map[map.size()] = rand();

        assertEqual(map.size(), 1000);
    }

    UNITTEST( hash_map_strings )
    {
        hash_map<std::string, std::string> map;
        map["a"] = "b";
        assertEqual(map.size(), 1);

        map["c"] = "d";
        assertEqual(map.size(), 2);

        assertEqual(map["a"], "b");
        assertEqual(map["c"], "d");

        map.insert(std::make_pair(std::string("aa"),
                                  std::string("bb")));
        assertEqual(map["aa"], "bb");
        assertEqual(map["a"], "b");
        assertEqual(map["c"], "d");
        assertEqual(map.size(), 3);

        hash_map<std::string, std::string>::iterator it
                = map.find("a");

        assertEqual(it->first, "a");
        assertEqual(it->second, "b");

        map.erase("c");
        assertEqual(map.size(), 2);

        it = map.find("c");
        assertTrue(it == map.end());
    }

} INSTANCE;

//////////////////////////////////////////////////////////////////////////
