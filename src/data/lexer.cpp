#include "stdafx.h"

#include <boost/lexical_cast.hpp>

#include "lexer.h"
#include "lexer_impl.h"

namespace filtering
{

lexeme::lexeme(std::string s) : type_(apunctuator)
{
    if (!s.size()) return;
    if (isdigit(s[0])) type_=anumber;
    if (isalpha(s[0])) type_=asymbol;

    set_string(s);
}

lexeme::lexeme(lextype_t t, std::string s) : type_(t)
{
    set_string(s);
}

void lexeme::set_string(const std::string &s)
{
    string_=s;
    if (type_==anumber) set_number( boost::lexical_cast<double>(string_) );

    hash_=0;
    for (const char *ptr=s.c_str(); *ptr!=0; ++ptr)
    {
        hash_ = 131 * hash_ + *ptr;
    }
}

std::string lexeme::get_string() const
{
    return string_;
}

lextype_t lexeme::get_type() const
{
    return type_;
}

int lexeme::get_reference() const
{
    return reference_;
}

void lexeme::set_reference(int r)
{
    reference_=r; type_=areference;
}

double lexeme::get_number() const
{
    return number_;
}

int    lexeme::get_hash() const
{
    return hash_;
}

void lexeme::set_number(double n)
{
    number_=n;
    type_=anumber;
}

void lexeme::set_punctuator()
{
    type_=apunctuator;
}

bool lexeme::operator==(const lexeme &other) const
{
    if (string_!=other.string_) return false;
    return true;
    //if (type_!=other.type_) return false;
    //if (hash_!=other.hash_) return false;
}

std::ostream &operator<<(std::ostream &str, const lexeme &self)
{
    char c='?';
    switch (self.type_)
    {
    case apunctuator: c='O'; break;
    case anumber: c='N'; break;
    case asymbol: c='S'; break;
    case areference: c='R'; break;
    }
    return str << c << " " << self.string_;
}

void lexeme::analyze(const vars_t &map)
{
    switch (get_type())
    {
    case asymbol:
    {
        vars_t::const_iterator index=map.find(string_);
        if (index!=map.end())
            set_reference(index->second);
        break;
    }
    }
}

//////////////////////////////////////////////////////////////////////////


lexer::lexer(std::string str, const boost::optional<state_ptr_t> &def)
{
    process(str, def);
}

void lexer::process(std::string str, const boost::optional<state_ptr_t> &def)
{
    std::istringstream ss(str);
    process(ss, def);
}

void lexer::process(std::istream &str, const boost::optional<state_ptr_t> &def)
{
    lexemes_.clear();

    state_ptr_t begin = def ? def.get() : state::start;

    for (; str; )
    {
        begin=begin(this, str.get());
    }
}

void lexer::apply(const vars_t &map)
{
    std::for_each(STL_II(lexemes_), bind(&lexeme::analyze, _1, boost::ref(map)));
}

bool lexer::fits(char c, const char **begin, const char **end)
{
    std::string bcd=current_lexeme_+c;
    for (; begin!=end; ++begin)
        if (bcd==*begin) return true;
    return false;
}

void lexer::operator+=(char c)
{
    current_lexeme_+=c;
}

std::string lexer::get_current_lexeme() const
{
    return current_lexeme_;
}

void lexer::push_lexeme(lextype_t t)
{
    lexemes_.push_back(lexeme(t, current_lexeme_));
    current_lexeme_=std::string();
}

std::vector<lexeme>::const_iterator lexer::begin() const
{
    return lexemes_.begin();
}

std::vector<lexeme>::const_iterator lexer::end() const
{
    return lexemes_.end();
}

unsigned lexer::size() const
{
    return lexemes_.size();
}

std::string lexer::string() const
{
    std::ostringstream stream;
    std::copy(begin(), end(), std::ostream_iterator<lexeme>(stream, " "));
    return stream.str();
}

}
