#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites.h"
#include "heap.h"

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( heap_test )
{
public:


	UNITTEST( simple_test )
	{
		std::vector<int> foovector;
		foovector.push_back(9);
		utils::push_heap(STL_II(foovector));

		foovector.push_back(2);
		utils::push_heap(STL_II(foovector));

		foovector.push_back(3);
		utils::push_heap(STL_II(foovector));

		utils::sort_heap(STL_II(foovector));

		assertEqual(foovector[0], 2);
		assertEqual(foovector[1], 3);
		assertEqual(foovector[2], 9);
	}

} INSTANCE;

//////////////////////////////////////////////////////////////////////////
