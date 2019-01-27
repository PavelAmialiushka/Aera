#pragma once

#include "lexer.h"
#include "syntaxer.h"



struct analysis
{
    typedef filtering::lexer	 lexer;
    typedef filtering::state_t state_t;
    typedef filtering::node    node;

    static state_t start(lexer *lex, int c)
    {
        if (c==EOF) return start;
        if (isspace(c)) return start;
        *lex+=c;

        if (strchr("()&|!", c)) return punctuator;
        return symbol;
    }

    static state_t punctuator(lexer *lex, int c)
    {
        static const char *set[]= {"&&", "!", "||", ")", "("};

        if (c!=EOF && lex->fits(c, STL_AA(set))) return *lex+=c, punctuator;

        lex->push_lexeme(filtering::apunctuator);
        return start(lex, c);
    }

    static state_t symbol(lexer *lex, int c)
    {
        if (c!=EOF && strchr("&|!()", c)==0) return *lex+=c, symbol;

        lex->push_lexeme(filtering::asymbol);
        return start(lex, c);
    }

    struct info
    {
        info() {}
        info(std::string string, bool st)
            : exprs(1, string), state(1, st)
        {}

        std::vector<std::string> exprs;
        std::vector<bool>				 state;

        unsigned size() const
        {
            return exprs.size();
        }

        void append(const info &other)
        {
            std::copy(STL_II(other.exprs), std::back_inserter(exprs));
            std::copy(STL_II(other.state), std::back_inserter(state));
        }
    };

    static info get_contents(node &nd, bool flag=true)
    {
        if (nd.get_lexeme()=="()" || nd.right()->get_lexeme()=="!" || nd.get_lexeme()=="not")
        {
            return get_contents(*nd.right(), flag);
        }
        else if (nd.get_lexeme()=="||" || nd.get_lexeme()=="or")
        {

            info result=get_contents(*nd.left(), flag);
            result.append( get_contents(*nd.right(), flag) );
            return result;

        }
        else if (nd.get_lexeme()=="&&" || nd.get_lexeme()=="and")
        {

            info result=get_contents(*nd.left(), flag);
            result.append( get_contents(*nd.right(), false));
            return result;

        }
        else
        {
            return info(nd.string(), flag);
        }
    }

    static node tree_maker(std::string str)
    {
        lexer lx(str/*, start*/);
        return filtering::tree_maker(lx);
    }
};


