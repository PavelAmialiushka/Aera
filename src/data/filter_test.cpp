#include "StdAfx.h"

#include "utilites\TestFile.h"

#include "data\syntaxer.h"
#include "data\syntaxer_impl.h"
#include "data\lexer.h"
#include "data\expranalyz.h"

#include "data/loader.h"
#include "data/filter.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

//////////////////////////////////////////////////////////////////////////

using namespace filtering;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( filter_test )
{
public:

    UNITTEST( prigma )
    {
        std::string test="(1)&&(2)";

        lexer lx(test, analysis::start);
        assertEqual("O ( S 1 O ) O && O ( S 2 O ) ", lx.string());

        node nd=tree_maker(lx);
        assertEqual(test, nd.string());
    }

    UNITTEST( prigma11 )
    {
        std::string test="(1)and(2)";

        lexer lx(test, analysis::start);
        assertEqual("O ( S 1 O ) S and O ( S 2 O ) ", lx.string());

        node nd=tree_maker(lx);
        assertEqual(test, nd.string());
    }

    UNITTEST( prigma2 )
    {
        std::string test="!(((((17929<=T&&T<=19730)&&(555<=D&&D<=70398))||((17851<=T&&T<=21804)&&(36<=E&&E<=33653)))&&!((32.5<=Ch)))&&!((Ch<=20.8)))";
        lexer lx(test/*, analysis::start*/);
        node nd=tree_maker(lx);

        analysis::info info=analysis::get_contents(*nd.right());

//        assertEqual(4, info.size());
//        assertEqual("32.5<=Ch", info.exprs[2]);
//        assertEqual(false, info.state[2]);
    }

    UNITTEST( prigma22 )
    {
        std::string test="not (((((17929<=T and T<=19730) and (555<=D and D<=70398))or((17851<=T and T<=21804) and (36<=E and E<=33653)))and not((32.5<=Ch)))and not((Ch<=20.8)))";
        lexer lx(test/*, analysis::start*/);
        node nd=tree_maker(lx);

        analysis::info info=analysis::get_contents(*nd.right());

//        assertEqual(4, info.size());
//        assertEqual("32.5<=Ch", info.exprs[2]);
//        assertEqual(false, info.state[2]);
    }

    UNITTEST( regression_tests )
    {
        //analysis::tree_maker("((A-50)*(D-100)>201)&&(D>30)");
    }

    UNITTEST( test_lexer )
    {
        lexer lx("a+0.9a=cdx+1e+10");
        assertEqual(8u, lx.size());
        assertEqual(lexeme(asymbol, "a"), *lx.begin());
        assertEqual(lexeme(asymbol, "cdx"), *(lx.begin()+5));
        assertEqual(lexeme(anumber, "1e+10"), *(lx.end()-1));

        lx.process("<><==&&&");

        assertEqual(5u, lx.size());

        lx.process("0.9.1");
        assertEqual(2u, lx.size());
        std::ostringstream strl;
        std::copy(lx.begin(), lx.end(), std::ostream_iterator<lexeme>(strl, " "));
        assertEqual("N 0.9 N .1 ", strl.str());
    }

    UNITTEST( test_basic_sn_test )
    {
        vars_t vr; vr["a"], vr["b"], vr["c"], vr["d"], vr["e"];
        lexer lx("a+b");
        lx.apply(vr);
        syntaxer sn(lx.begin(), lx.end());

        std::ostringstream strl;
        std::copy(sn.begin(), sn.end(), std::ostream_iterator<lexeme>(strl, " "));
        assertEqual(std::string("R a R b O + "), strl.str());
        //std::cerr << std::endl;

        lx.process("a+b+c");
        lx.apply(vr);
        sn.process(lx.begin(), lx.end());
        strl.str(""); std::copy(sn.begin(), sn.end(), std::ostream_iterator<lexeme>(strl, " "));
        assertEqual(std::string("R a R b O + R c O + "), strl.str());
        //std::cerr << std::endl;

        lx.process("a+b*c");
        lx.apply(vr);
        sn.process(lx.begin(), lx.end());
        strl.str(""); std::copy(sn.begin(), sn.end(), std::ostream_iterator<lexeme>(strl, " "));
        assertEqual(std::string("R a R b R c O * O + "), strl.str());
        //std::cerr << std::endl;

        lx.process("a*b+c");
        lx.apply(vr);
        sn.process(lx.begin(), lx.end());
        strl.str(""); std::copy(sn.begin(), sn.end(), std::ostream_iterator<lexeme>(strl, " "));
        assertEqual(std::string("R a R b O * R c O + "), strl.str());
        //std::cerr << std::endl;

        lx.process("a*(b+c)-d/e");
        lx.apply(vr);
        sn.process(lx.begin(), lx.end());
        strl.str(""); std::copy(sn.begin(), sn.end(), std::ostream_iterator<lexeme>(strl, " "));
        assertEqual(std::string("R a R b R c O + O () O * R d R e O / O - "), strl.str());
        //std::cerr << std::endl;
    }

    UNITTEST( test_proc_tree )
    {
        double data[]= {2, 7, 0};
        std::map<std::string, int> vars;
        vars["v2"]=0;
        vars["v7"]=1;
        vars["v0"]=2;

        lexer lx("v0-1*v2+6/3*(v7-v2*3)+0+0+-0");
        lx.apply(vars);
        syntaxer sn(lx.begin(), lx.end());
        node tree=sn.get_process_tree();
        assertEqual(0.0, tree.get_value(data));
    }

    UNITTEST( test_tree_maker )
    {
        std::string ss[]= {"a", "d", "r"};
        vars_t map; for (unsigned index=0; index<SIZEOF(ss); ++index)
        {
            map[ss[index]]=index;
        }
        double data[][3]=
        {
            55, 200, 20,
            40,   1,  1,
            80,1000,  3
        };

        assertEqual(0, tree_maker("a<50", map).get_value(data[0]));
        assertEqual(1, tree_maker("a<50", map).get_value(data[1]));
        assertEqual(0, tree_maker("a<50", map).get_value(data[2]));

        assertEqual(0, tree_maker("d+a>300", map).get_value(data[0]));
        assertEqual(0, tree_maker("d+a>300", map).get_value(data[1]));
        assertEqual(1, tree_maker("d+a>300", map).get_value(data[2]));

        assertEqual(-55, tree_maker("-a", map).get_value(data[0]));

        assertEqual(0, tree_maker("!(-60<=a && a<=100)", map).get_value(data[0]));

        assertEqual(0, tree_maker("-1+1").get_value(0));
        assertEqual(0, tree_maker("-1*-1-1").get_value(0));
        assertEqual(0, tree_maker("(-1<1)-1").get_value(0));
        assertEqual(0, tree_maker("(-1 && 1) -1").get_value(0));
    }

    UNITTEST( test_comarision )
    {
        std::string ss[]= {"a", "d", "r"};
        vars_t map; for (unsigned index=0; index<SIZEOF(ss); ++index)
        {
            map[ss[index]]=index;
        }
        double data[][3]=
        {
            55, 200, 20,
            40,   1,  1,
            80,1000,  3
        };
        assertTrue(tree_maker("a<0 || d>100", map).get_value(data[0]));
        assertTrue(tree_maker("", map).get_value(data[0]));

        assertTrue(0==tree_maker("a<0 && d>100", map).get_value(data[0]));
        assertTrue(0==tree_maker("a<0\nd>100", map).get_value(data[0]));

        assertTrue(tree_maker("0<=a && a<=100", map).get_value(data[0]));
        assertTrue(tree_maker("(0<=a)", map).get_value(data[0]));
        assertTrue(tree_maker("(0<=a && a<=100)", map).get_value(data[0]));
        assertTrue(!tree_maker("!(0<=a && a<=100)", map).get_value(data[0]));

        // multi-line
        assertTrue(tree_maker("!(0>=a)\n!(a>=100)\n!(-a==0)", map).get_value(data[0]));
        assertTrue(tree_maker("", map).get_value(data[0]));

        assertTrue(!tree_maker("!(0<=a)\n!(a<=100)", map).get_value(data[0]));
        assertTrue(!tree_maker("!(0<=a)\n!(a>=100)", map).get_value(data[0]));
        assertTrue(!tree_maker("!(0>=a)\n!(a<=100)", map).get_value(data[0]));
        assertTrue(tree_maker("!(0>=a)\n!(a>=100)", map).get_value(data[0]));
    }

    UNITTEST( test_errors )
    {
        vars_t map;

        assertBlock
        {
            tree_maker("+", map);
        } assertException(filtering::syntax_error);

        assertBlock
        {
            tree_maker("1+", map);
        } assertException(filtering::syntax_error);

        assertBlock
        {
            tree_maker("*1", map);
        } assertException(filtering::syntax_error);

        assertBlock
        {
            tree_maker("1**2", map);
        } assertException(filtering::syntax_error);

        assertBlock
        {
            tree_maker("1<<2", map);
        } assertException(filtering::syntax_error);

        assertBlock
        {
            tree_maker("!", map);
        } assertException(filtering::syntax_error);

    }

    UNITTEST( test_to_string )
    {
        std::string base="!(0>=1)\n!(1>=100)\n!(1==0)";
        std::string result=tree_maker(base).string();
    }
} INSTANCE;

class UNITTEST_SUITE(filter_node_test)
{
    nodes::loader* loader;
    nodes::filter* filter;
    void setUp()
    {
        loader = new nodes::loader;
        loader->open( get_test_filename(0) );

        filter = new nodes::filter;
        filter->set_parent(loader);
    }

    void tearDown()
    {
        delete filter;
        delete loader;
    }

    UNITTEST( filter_node1 )
    {
        filter->set_filter_string("A>=60");
        nodes::presult r = process::get_result<nodes::result>(filter, true);
        data::pslice slice = r->ae;

        assertEqual(2, slice->size());
        assertDoublesEqual( 382.8134795, slice->get_value(0, aera::C_Time), 1e-6);
        assertDoublesEqual(4279.4279555, slice->get_value(1, aera::C_Time), 1e-6);
    }

    UNITTEST( filter_node2 )
    {
        filter->set_filter_string("CH=20 && A=49");
        nodes::presult r = process::get_result<nodes::result>(filter, true);
        data::pslice slice = r->ae;

        assertEqual(3, slice->size());
        assertDoublesEqual(1250.0916467, slice->get_value(0, aera::C_Time), 1e-6);
        assertDoublesEqual(2443.1625703, slice->get_value(1, aera::C_Time), 1e-6);
        assertDoublesEqual(4598.8760365, slice->get_value(2, aera::C_Time), 1e-6);
    }

} INSTANCE;

//////////////////////////////////////////////////////////////////////////
