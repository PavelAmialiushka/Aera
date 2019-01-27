#pragma once

namespace filtering
{

namespace state
{


state_t start(lexer *, int c);
state_t number(lexer *, int c);
state_t number_mantisa(lexer *, int c);
state_t symbol(lexer *, int c);
state_t punctuator(lexer *, int c);

//////////////////////////////////////////////////////////////////////////

state_t start(lexer *lex, int c)
{
    if ((c==EOF || isspace(c)) && c != '\n')
    {
        return start;
    }

    *lex+=c;

    if (isdigit(c) || c=='.') return number;
    if (isalpha(c)) return symbol;
    return punctuator;
}

state_t number(lexer *lex, int c)
{
    if (c==EOF || !(isdigit(c) || c=='.' || c=='e' || c=='E'))
    {
        lex->push_lexeme(anumber);
        return start(lex, c);
    }

    if (c=='e' || c=='E') return *lex+=c, number_mantisa;

    if (c=='.' && lex->get_current_lexeme().find(".")!=std::string::npos)
    {
        lex->push_lexeme(anumber);
        return number(lex, c);
    }

    return *lex+=c, number;
}

state_t number_mantisa(lexer *lex, int c)
{
    if (c==EOF || !(isdigit(c) || c=='+' || c=='-'))
    {
        lex->push_lexeme(anumber);
        return start(lex, c);
    }

    return *lex+=c, number_mantisa;
}

state_t symbol(lexer *lex, int c)
{
    if (c==EOF || !(isalnum(c)))
    {
        lex->push_lexeme(asymbol);
        return start(lex, c);
    }

    return *lex+=c, symbol;
}

state_t punctuator(lexer *lex, int c)
{
    static const char *opers[]=
    {"&&", "||", "<=", ">=", "!=", "<>", "=="};

    if (c==EOF || isalnum(c))
    {
        lex->push_lexeme(apunctuator);
        return start(lex, c);
    }

    if (lex->fits(c, STL_AA(opers)))
        return *lex+=c, punctuator;

    lex->push_lexeme(apunctuator);
    return start(lex, c);
}

}
}