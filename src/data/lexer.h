#ifndef ___filterlexer__aera_h___
#define ___filterlexer__aera_h___

#include <boost/optional.hpp>

namespace filtering
{

enum lextype_t { apunctuator, anumber, asymbol, areference, } ;

typedef std::map<std::string, int> vars_t;

//////////////////////////////////////////////////////////////////////////
class lexer;

/// finite state

typedef class state_t (*state_ptr_t)(lexer *, int c);

class state_t
{
    state_ptr_t f_;
public:
    state_t(state_ptr_t f) : f_(f) {}
    operator state_ptr_t()
    {
        return f_;
    }
};

//////////////////////////////////////////////////////////////////////////

class lexeme
{
public:

    lexeme(std::string s=std::string());
    lexeme(lextype_t t, std::string s);

    void set_string(const std::string &s);
    void set_reference(int r);
    void set_number(double n);
    void set_punctuator();

    void analyze(const vars_t &map);

    std::string get_string() const;
    lextype_t   get_type() const;
    int				  get_reference() const;
    double		  get_number() const;
    int         get_hash() const;

    bool operator==(const lexeme &other) const;
    bool operator!=(const lexeme &other) const
    {
        return !operator==(other);
    }

    friend std::ostream &operator<<(std::ostream &str, const lexeme &self);

    friend bool operator==(const char *str, const lexeme &lex)
    {
        return lex.operator==(str);
    }

    friend bool operator!=(const char *str, const lexeme &lex)
    {
        return lex.operator!=(str);
    }

    bool operator==(const char *str) const
    {
        return string_==str;
    }

    bool operator!=(const char *str) const
    {
        return string_!=str;
    }

private:
    lextype_t type_;
    int				reference_;
    double    number_;
    std::string string_;
    int				hash_;
};


//////////////////////////////////////////////////////////////////////////

class lexer
{
public:
    typedef boost::optional<state_ptr_t> def_t;

    lexer() {}
    lexer(std::string str, const def_t &def=def_t());

    void process(std::string str, const def_t &def=def_t());
    void process(std::istream &str, const def_t &def=def_t());

    void apply(const vars_t &map);

    bool fits(char c, const char **, const char **);
    void operator+=(char c);

    void push_lexeme(lextype_t t);

    std::string get_current_lexeme() const;
    std::vector<lexeme>::const_iterator begin() const;
    std::vector<lexeme>::const_iterator end() const;
    unsigned size() const;

    std::string string() const;

private:
    std::string current_lexeme_;
    std::vector<lexeme> lexemes_;
};

//////////////////////////////////////////////////////////////////////////

}

#endif
