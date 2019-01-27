#pragma once

//////////////////////////////////////////////////////////////////////////

template<typename> struct hash {  };

template<> struct hash<int>
{
    size_t operator()(int i) const
    {
        return static_cast<size_t>(i);
    }
};

template<> struct hash<unsigned>
{
    size_t operator()(unsigned i) const
    {
        return static_cast<size_t>(i);
    }
};

template<> struct hash<std::string>
{
    size_t operator()(std::string x) const
    {
        size_t value = 1;
        for (std::string::const_iterator it= x.begin(); it != x.end(); ++it)
        {
            value *= 26;
            value += static_cast<size_t>(*it);
        }
        return value;
    }
};

template<class T>
struct inner_hash
{
    size_t operator()(T const& t) const
    {
        return t.hash();
    }
};

//////////////////////////////////////////////////////////////////////////

namespace details
{
static const size_t prime_list[] =
{
    53ul, 97ul, 193ul, 389ul, 769ul,
    1543ul, 3079ul, 6151ul, 12289ul, 24593ul,
    49157ul, 98317ul, 196613ul, 393241ul, 786433ul,
    1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul,
    50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul,
    1610612741ul, 3221225473ul, 4294967291ul
};

inline size_t next_prime(size_t n)
{
    size_t const *bound;
    bound = std::lower_bound(prime_list,prime_list + 28, n);
    if (bound == prime_list + 28)
        bound--;
    return *bound;
}

}
