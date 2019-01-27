#pragma once

#include "Traits.h"

namespace data
{

struct lohi_value 
{ 
	double lo, hi;

	lohi_value()
	{
		widen_range_begin(lo, hi);
	}

	lohi_value(double x)
		: lo(x), hi(x)
	{
	}

	lohi_value(double x, double y)
		: lo(x), hi(y)
	{
	}

    bool empty() const
    {
        return _isnan(lo);
    }

	void update(double value)
	{
		widen_range(lo, hi, value);
	}

    void update(lohi_value const& v)
    {
        widen_range(lo, hi, v.lo);
        widen_range(lo, hi, v.hi);
    }
};

class minmax_map
{
	std::vector<unsigned> indexes_;
	std::vector<lohi_value> values_;
public:
	minmax_map()
	{
            indexes_.resize(256, -1);
	}

    void merge(minmax_map const& other)
    {
        // сканируем чужие индексы
        for(unsigned f = 0; f < other.indexes_.size(); ++f)
        {
            unsigned index = other.indexes_[f];
            if (index != (unsigned)-1)
            {
                lohi_value const & v = other.values_[index];
                add_value((aera::chars)f, v.lo);
                add_value((aera::chars)f, v.hi);
            }
        }
    }

	void clear()
	{
		std::vector<unsigned> temp(256, -1);
		indexes_.swap(temp);

		values_.clear();
	}

	bool empty() const
	{
		return values_.empty();
	}

	int append_char(aera::chars it, lohi_value lh = lohi_value())
	{
		int index = values_.size();
		values_.push_back( lh );
		indexes_[(unsigned)it] = index;
		return index;
	}

	void add_value(aera::chars it, double value)
	{
		assert(unsigned(it) < indexes_.size());
		int index = indexes_[(unsigned)it];
		if (index == -1)
			append_char(it, lohi_value(value));
		else
			values_[index].update(value);
	}

    const lohi_value* find(aera::chars it) const
    {
        assert(unsigned(it) < indexes_.size());
        int index = indexes_[(unsigned)it];
        if (index == -1)
            return 0;
        return &values_[index];
    }
	lohi_value* find(aera::chars it)
	{
		assert(unsigned(it) < indexes_.size());
		int index = indexes_[(unsigned)it];
		if (index == -1)
			return 0;
		return &values_[index];
	}

};

}
