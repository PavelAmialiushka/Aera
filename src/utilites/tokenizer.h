//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

class tokenizer
{
public:
    typedef std::vector<std::string>  container;
    typedef container::iterator       iterator;
    typedef container::const_iterator const_iterator;

public:
    tokenizer(std::string string, std::string sep=" ", bool empty=false)
    {
        while (string.size())
        {
            std::string token=head(string, sep);
            if (empty || !token.empty())
            {
                vector_.push_back(token);
            }
        }
    }

    static
    std::string head(std::string &source, std::string sep=" ")
    {
        std::string result;
        unsigned int k=source.find_first_of(sep);
        if (k<source.size())
        {
            result=source.substr(0, k);
            source.erase(0, k+1);
        }
        else
        {
            result=source;
            source.erase(0, std::string::npos);
        }
        return result;
    }

    static
    std::string tail(std::string &source, std::string sep=" ")
    {
        std::string result;
        unsigned k=source.find_last_of(sep);
        if (k<source.size())
        {
            result=source.substr(k+1);
            source.erase(k);
        }
        return result;
    }

//	template<class T>
//	static
//	std::string join(T begin, T end, std::string sep)
//	{
//		if (begin==end) return std::string();
//		std::string result= *begin++;
//		for(;begin!=end; ++begin)	result += sep + *begin;
//		return result;
//	}

    static std::string join(std::string start, std::string sep, std::string end)
    {
        if (sep.empty())   return "";
        if (end.empty())   return start;
        if (start.empty()) return end;
        return start + sep + end;
    }

    std::string at(unsigned index) const
    {
        return vector_.at(index);
    }

    const_iterator begin() const
    {
        return vector_.begin();
    }

    const_iterator end() const
    {
        return vector_.end();
    }

    unsigned size() const
    {
        return vector_.size();
    }

private:

    container   vector_;
};
