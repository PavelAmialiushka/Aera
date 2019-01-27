#pragma once

enum strategy { noauto, normal, summize };


class minmax_value
{
public:

    minmax_value(bool log, double a, double b, strategy s)
        : log_(log),
          set_(false),
          strategy_(s),
          min_(a), max_(b)
    {}

    void examine(double value)
    {
        if (!set_ && strategy_!=noauto)
        {
            min_=value;
            max_=value;
            set_=true;
        }
        if (log_ && value==0)
        {
            switch (strategy_)
            {
            case normal:
                min_=std::min(min_, 0.1);
                max_=std::max(max_, 0.1);
            }
        }
        else
        {
            switch (strategy_)
            {
            case noauto: break;
            case normal:
                min_=std::min(min_, value);
                max_=std::max(max_, value);
                break;
            case summize:
                min_=std::min(min_, value);
                max_+=value;
                break;
            }
        }
    }

    plot::range aquire()
    {
        return plot::range(min_, max_, log_);
    }

public:
    double min_;
    double max_;
    bool log_;
    bool set_;
    strategy strategy_;
};




class histogramma
{
public:
    histogramma(plottype_t t, unsigned size, double a, double b, bool log_scale)
        : size_(size), min_(a), max_(b),
          values_(size), counts_(size), type_(t)
        , log_scale_(log_scale)
    {
    }

    void push(double abscissa, double ordinate)
    {
        unsigned index=log_scale_
                ? scale(log10(std::max(0.1, abscissa)), log10(min_), log10(max_), 0u, size_-1)
                : scale(abscissa, min_, max_, 0u, size_-1);

        if (index >= size_ && abscissa < min_)
        {
            // вне графика, но для графика min-max должны
            // запоминаться значения слева от графика
            if (type_!=minmax) return;
            index=0;
        }

        if (index<size_)
        {
            if (type_==distribution)
            {
                // максимальное значение
                values_[index]=std::max(values_[index], ordinate);
                counts_[index]=1;
            }
            else
            {
                // определяем сумму
                values_[index]+=ordinate;
                ++counts_[index];
            }
        }
    }

    void process(plottype_t type)
    {
        try
        {
            double sum=0;
            switch (type)
            {
            case minmax:
                for (unsigned index=0; index<size_; ++index)
                {
                    counts_[index]=1;
                    values_[index]+=sum;
                    sum=values_[index];
                }
                break;
            case maxmin:
                for (int index=size_-1; index>=0; --index)
                {
                    counts_[index]=1;
                    values_[index]+=sum;
                    sum=values_[index];
                }
                break;
            case summation:
                std::fill(STL_II(counts_), 1);
                break;
            }
        } LogExceptionPath("histogramma::process");
    }

    std::pair<double,double> operator[] (unsigned index) const
    {
        debug::Assert<overflow>(index<size_, HERE);
        double abscissa=
                log_scale_
                ? pow(10, scale(index, 0u, size_-1, log10(min_), log10(max_)))
                : scale(index, 0u, size_-1, min_, max_);
        double ordinate=counts_[index]
                        ? values_[index]/counts_[index] : 0;

        if (type_==summation)
        {
            // от 0 до 100 секунд с 200 делениями
            // даст ординату за 0.5с, т.е
            // ордината*2 в секунду


            // ordinate /= ((max_-min_) / size_);
        }

        return std::make_pair(abscissa, ordinate);
    }

    bool count(unsigned index) const
    {
        debug::Assert<overflow>(index<size_, HERE);
        return counts_[index];
    }

    std::pair<double, double> get_minmax_limits(bool nonull) const
    {
        if (size_)
        {
            double mi, ma;
            mi=ma=operator[](0).second;
            for (unsigned index=0; index<size_; ++index)
            {
                if (!(nonull && count(index)==0))
                {
                    double v=operator[](index).second;
                    mi=std::min(mi, v);
                    ma=std::max(ma, v);
                }
            }
            return std::make_pair(mi, ma);
        }
        else return std::make_pair(0.0, 0.0);
    }

    unsigned size() const
    {
        return size_;
    }

private:
    std::vector<double> values_;
    std::vector<unsigned> counts_;
    double min_, max_;
    unsigned size_;
    bool log_scale_;
    plottype_t type_;
};
