#ifndef ___filtersintaxer__aera_h___
#define ___filtersintaxer__aera_h___

#include "lexer.h"
#include "slice.h"

namespace filtering
{

class syntax_error : public debug::aera_error
{
public:
    syntax_error(std::string a, debug::SourcePos l) : aera_error(a.c_str(), l)
    {
    }

    static void assert_expected(bool assertion, std::string message, lexeme lex)
    {
        if (!assertion)
        {
            std::ostringstream ss;
            ss << message << " before \"" << lex.get_string() << "\"";
            throw syntax_error(ss.str(), HERE);
        }
    }

    static void assert_unexpected(bool assertion, std::string message, lexeme lex)
    {
        if (!assertion)
        {
            std::ostringstream ss;
            ss << message << "\"" << lex.get_string() << "\"";
            throw syntax_error(ss.str(), HERE);
        }
    }

    virtual ~syntax_error() throw() {}

    virtual const char *what() const throw()
    {
        return string_.c_str();
    }

private:
    std::string string_;
};

//////////////////////////////////////////////////////////////////////////

class node
{
public:
    node(lexeme op, shared_ptr<node> l, shared_ptr<node> r);
    node(lexeme op, shared_ptr<node> l);
    node(lexeme l=lexeme());

    double get_value(const double *data);
    double get_value(data::slice *, unsigned);

    std::string string() const;
    lexeme get_lexeme() const;

    shared_ptr<node> left() const
    {
        return left_;
    }
    shared_ptr<node> right() const
    {
        return right_;
    }

private:
    lexeme lexeme_;
    shared_ptr<node>  left_;
    shared_ptr<node>  right_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

node tree_maker(lexer &lx);
node tree_maker(std::string string, vars_t &map);
node tree_maker(std::string string);


}


#endif
