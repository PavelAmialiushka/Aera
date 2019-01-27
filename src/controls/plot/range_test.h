#include "controls\plot\range.h"

#if _MSC_VER > 1200
#define STL_Z
#else
#define STL_Z "0"
#endif

using plot::range;

class UNITTEST_SUITE(plotrange_test)
{
public:

    UNITTEST( single_test )
    {
        assertEqual(range(range::nocalc, 0.1, 0.9), range(0.1, 0.9));
        assertEqual(range(range::nocalc, 1, 9), range(1, 9));
        assertEqual(range(range::nocalc, 0, 100), range(1, 99));
        assertEqual(range(range::nocalc, 1, 12), range(1, 12));
        assertEqual(range(range::nocalc, 0, 20), range(1, 19));
        assertEqual(range(range::nocalc, 1, 21), range(1, 21));
        assertEqual(range(range::nocalc, 0, 1), range(0, 1));
        assertEqual(range(range::nocalc, 1, 42), range(1, 42));
    }

    UNITTEST( testmarks )
    {
        range r(1,9);
        assertEqual(range::marks(1, 9, 1.0), r.get_marks(0));
        assertEqual(range::marks(1, 9, 0.2), r.get_marks(2));
        assertEqual(range::marks(1, 9, 0.05),r.get_marks(4));

        assertEqual(9u, range::marks(1,9,1).size());

        assertEqual(std::string("1 2 3 4 5 6 7 8 9 "), range(1, 9).get_marks(0).get_all_marks());

        assertEqual(std::string("1"),   range(1,2).get_marks(0)[0]); // 1
        assertEqual(std::string("1.0"), range(1,2).get_marks(1)[0]); // 0.5
        assertEqual(std::string("1.0"), range(1,2).get_marks(2)[0]); // 0.2
        assertEqual(std::string("1.00"),range(1,2).get_marks(4)[0]); // 0.05

        assertEqual(1u, range(1,2).get_marks(0).get_max_length());
        assertEqual(3u, range(1,2).get_marks(1).get_max_length());
        assertEqual(3u, range(1,2).get_marks(2).get_max_length());
        assertEqual(4u, range(1,2).get_marks(4).get_max_length());
    }

    UNITTEST( test_negative )
    {
        range amp(1.8, 9.6);
        assertEqual(range::marks(1, 10, 1), amp.get_marks(0));
        assertEqual("1 2 3 4 5 6 7 8 9 10 ", amp.get_marks(0).get_all_marks()); // 1
        assertEqual("2 4 6 8 10 ", amp.get_marks(-1).get_all_marks()); // 2
        assertEqual("5 10 ", amp.get_marks(-2).get_all_marks()); // 5
        assertEqual("10 ", amp.get_marks(-3).get_all_marks()); // 10
    }

    UNITTEST( test_logscale )
    {
        range ran(1, 888, true);
        range::marks marks=ran.get_marks(0);
        assertEqual(range::marks(1, 1000, 1), marks);

        assertEqual(4u, marks.size());
        assertEqual(std::string("10.0"), marks[1]);
        assertEqual(std::string("100"), marks[2]);
    }

    UNITTEST( test_map_unmap )
    {
        range ran(0, 10);
        assertEqual(100u, ran.map_to(10, 100));
        assertEqual(50u, ran.map_to(5, 100));
        assertEqual(0u, ran.map_to(0, 100));

        assertEqual(std::string("0.1"), ran.unmap(1, 100));

        range ranlog(0.01, 1.67121e+07, true);
        assertEqual(100u, ranlog.map_to(100000000., 100));
        assertEqual(0u, ranlog.map_to(0.01, 100));

        assertEqual(std::string("1.0e+0" STL_Z "8"), ranlog.unmap(100, 100));
        assertEqual(std::string("1.0e+0" STL_Z "6"), ranlog.unmap(80, 100));
        assertEqual(std::string("100000"), ranlog.unmap(70, 100));
        assertEqual(std::string("10000"), ranlog.unmap(60, 100));
        assertEqual(std::string("1000"), ranlog.unmap(50, 100));
        assertEqual(std::string("0.10"), ranlog.unmap(10, 100));
        assertEqual(std::string("0.010"), ranlog.unmap(0, 100));

        range::marks mlog=ranlog.get_marks(0);
        assertEqual(11u, mlog.size());
        assertEqual(std::string("0.010"), mlog[0]);
        assertEqual(std::string("1.0e+0" STL_Z "8"), mlog[10]);
    }
} go;
