//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace stdex
{

template<typename _In, typename _Out, typename _P>
inline
void copy_if(_In first, _In last, _Out dest, _P pred)
{
    for (; first!=last; ++first)
    {
        if (pred(*first))
        {
            *dest=*first;
            ++dest;
        }
    }
}

template<typename _In, typename _Out, typename _P, typename _P2>
inline
void transform_if(_In first, _In last, _Out dest, _P pred_if, _P2 pred_transform)
{
    for (; first!=last; ++first)
    {
        if (pred_if(*first))
        {
            *dest=pred_transform(*first);
            ++dest;
        }
    }
}

}
