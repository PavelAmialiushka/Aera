#ifndef ___plotbuffertest__aera_h___
#define ___plotbuffertest__aera_h___

#include "controls\plot\buffer.h"

namespace plot
{

class UNITTEST_SUITE( darray_test )
{
public:

    static int sum(darray &b)
    {
        return std::accumulate(b.get(), b.get()+b.size(), 0);
    }

    UNITTEST( testA )
    {
        darray b(2, 2);
        assertEqual(0, sum(b));

        b.at(0, 0)=1;
        assertEqual(1, sum(b));

        b.at(b.width()-1, b.height()-1)=-1;
        assertEqual(0, sum(b));
    }
} go;

}

#endif
