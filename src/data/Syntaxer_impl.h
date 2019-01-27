#pragma once

#include "Lexer.h"

namespace filtering
{

class syntaxer
{
public:

private:



//  E
//  E ::= T T2
//  T2::= +T | -T | .
//  T ::= F F2
//  F2 ::= *F | /F | .
//  F ::= ( E ) | s | n


    // a+b+c
    // E => TX => FYX => aYX => aX => a+TX =>
    // => a+bX


    void R(); /** R := W \n W */
    void R2();

    void W(); /** W ::= Q && Q */
    void W2();

    void Q(); /** Q ::= E < E */
    void Q2();

    void E(); /** E ::= T + T  */
    void E2();

    void T(); /** T ::= F * F */
    void T2();

    void F(); /** number, variable, (R), -W or !W */

public:
    lexeme top();
    void pop();
    unsigned size() const;
    void out(const lexeme &lex);

    template<class I> syntaxer(I begin, I end)
    {
        process(begin, end);
    }

    template<class I> void  process(I begin, I end)
    {
        try
        {
            output_.clear();
            while (process_tree_.size()) process_tree_.pop();
            reader_.assign(begin, end);

            R();

            syntax_error::assert_unexpected(
                !reader_.size(), "unexpected token", size() ? top() : lexeme("??"));
        } LogExceptionPath("syntaxer::process(I begin, I end) ");
    }

    node get_process_tree() const;
    std::vector<lexeme>::const_iterator begin() const;
    std::vector<lexeme>::const_iterator end() const;
private:
    std::stack<shared_ptr<node> >  process_tree_;
    std::vector<lexeme> reader_;
    std::vector<lexeme> output_;
};

}