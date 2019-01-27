#include "stdafx.h"

#include "data/Syntaxer.h"
#include "data/Syntaxer_impl.h"

namespace filtering
{

//////////////////////////////////////////////////////////////////////////

void syntaxer::R()
{
    // R = W (\n W)*
    //
    try
    {
        W(); R2();
    } LogExceptionPath("R");
}

void syntaxer::R2()
{
    lexeme set[]= {"\n"};
    try
    {
        if (size())
        {
            if (std::count(set, ENDOF(set), top()))
            {
                lexeme opera=top(); pop();
                if (size())
                {
                    W();
                    out(lexeme("&&"));
                    R2();
                }
            }
        }
    } LogExceptionPath("R2");
}

void syntaxer::W()
{
    // W = Q (&& Q)*
    try
    {
        Q();
        W2();
    } LogExceptionPath("W");
}

void syntaxer::W2()
{
    lexeme set[]= {"&&", "||", "and", "or"};
    try
    {
        if (size())
        {
            if (std::count(set, ENDOF(set), top()))
            {
                lexeme opera=top(); pop();                
                opera.set_punctuator();
                Q();
                out(opera);
                W2();
            }
        }
    } LogExceptionPath("W2");
}

void syntaxer::Q()
{
    // Q = E (< E)*
    try
    {
        E();
        Q2();
    } LogExceptionPath("Q");
}


void syntaxer::Q2()
{
    lexeme set[]= {"<", ">", "<=", ">=", "==", "!=", "<>", "="};
    try
    {
        if (size())
        {
            if (std::count(set, ENDOF(set), top()))
            {
                lexeme opera=top(); pop();
                E();
                out(opera);
            }
        }
    } LogExceptionPath("Q2");
}


void syntaxer::E()
{
    // E = T (+ T)*
    try
    {
        T(); E2();
    } LogExceptionPath("E");
}

void syntaxer::E2()
{
    try
    {
        if (size())
            if (top()==lexeme("+") || top()==lexeme("-"))
            {
                lexeme opera=top(); pop();
                T();
                out(opera);
                E2();
            }
    } LogExceptionPath("X");
}

void syntaxer::T()
{
    // T = F (* F)
    try
    {
        F(); T2();
    } LogExceptionPath("T");
}

void syntaxer::T2()
{
    try
    {
        if (size())
            if (top()==lexeme("*") || top()==lexeme("/"))
            {
                lexeme opera=top(); pop();
                F();
                out(opera);
                T2();
            }
    } LogExceptionPath("Y");
}


void syntaxer::F()
{
    // F = ( R )
    // F = !R
    // F = -R
    try
    {

        syntax_error::assert_expected(size(), "expected value", lexeme());

        if (top()==lexeme("("))
        {
            pop();
            R();
            syntax_error::assert_expected(top()==lexeme(")"), "')' expected", top());
            pop();
            out( lexeme("()") );
        }
        else if (top()==lexeme("!") || top()==lexeme("not"))
        {
            lexeme opera=top(); pop();
            opera.set_punctuator();
            F();
            out(opera);
        }
        else if (top()==lexeme("-"))
        {
            pop();
            F();
            out( lexeme("-u") );
        } else
        {
            out(top()); pop();
        }
    } LogExceptionPath("F");
}

lexeme syntaxer::top()
{
    syntax_error::assert_expected(reader_.size(), "lexeme expected", lexeme("eof"));
    return reader_.front();
}

void syntaxer::pop()
{
    syntax_error::assert_expected(reader_.size(), "lexeme expected", lexeme("eof"));
    reader_.erase(reader_.begin());
}

unsigned syntaxer::size() const
{
    return reader_.size();
}

void syntaxer::out(const lexeme &lex)
{
    output_.push_back(lex);
    if (lex.get_type()==apunctuator)
    {
        if (lex==lexeme("!") || lex==lexeme("not") || lex==lexeme("-u") || lex==lexeme("()"))
        {
            syntax_error::assert_expected(process_tree_.size(), "token expected after", lex);

            shared_ptr<node> r=process_tree_.top(); process_tree_.pop();
            process_tree_.push( shared_ptr<node>( new node(lex, shared_ptr<node>(), r)) );
        }
        else
        {
            syntax_error::assert_expected(process_tree_.size()>=2, "expected after", lex);

            shared_ptr<node> r=process_tree_.top(); process_tree_.pop();
            shared_ptr<node> l=process_tree_.top(); process_tree_.pop();
            process_tree_.push( shared_ptr<node>( new node(lex, l, r)) );
        }
    }
    else
    {
        process_tree_.push( shared_ptr<node>( new node(lex)) );
    }
}

node syntaxer::get_process_tree() const
{
    debug::Assert<fault>(process_tree_.size(), HERE);
    return *process_tree_.top();
}

std::vector<lexeme>::const_iterator syntaxer::begin() const
{
    return output_.begin();
}

std::vector<lexeme>::const_iterator syntaxer::end() const
{
    return output_.end();
}


//////////////////////////////////////////////////////////////////////////


namespace
{

typedef double (*operator_t)(double, double);

double add(double a, double b)
{
    return a+b;
}
double sub(double a, double b)
{
    return a-b;
}
double mul(double a, double b)
{
    return a*b;
}
double div(double a, double b)
{
    return a/b;
}
double le(double a, double b)
{
    return a<b;
}
double leq(double a, double b)
{
    return a<=b;
}
double gr(double a, double b)
{
    return a>b;
}
double grq(double a, double b)
{
    return a>=b;
}
double eq(double a, double b)
{
    return a==b;
}
double ne(double a, double b)
{
    return a!=b;
}
double an(double a, double b)
{
    return a&&b;
}
double orr(double a, double b)
{
    return a||b;
}

double nott(double, double b)
{
    return !b;
}
double neg(double, double b)
{
    return -b;
}
double par(double, double b)
{
    return b;
}

struct operators_ini
{
    void push(std::string str, operator_t op)
    {
        lexeme lex(str);
        unsigned hash=lex.get_hash();
        if (operators_.size() <= hash) operators_.resize(hash+1);
        assert(operators_[hash]==0);
        operators_[hash] = op;
    }

    operators_ini()
    {
        push("+", add);
        push("-", sub);
        push("*", mul);
        push("/", div);
        push("<", le);
        push("<=", leq);
        push(">", gr);
        push(">=", grq);
        push("==", eq);
        push("=",  eq);
        push("!=", ne);
        push("<>", ne);
        push("&&", an);
        push("and", an);
        push("||", orr);
        push("or", orr);

        push("!", nott);
        push("not", nott);
        push("-u", neg);
        push("()", par);
    }

    double get(unsigned hash, double a, double b)
    {
        assert(hash<operators_.size());
        assert(operators_[hash]);
        return operators_[hash](a, b);
    }

private:
    std::vector<operator_t> operators_;

} calculator;

} // namespace


node::node(lexeme op, shared_ptr<node> l, shared_ptr<node> r)
    : lexeme_(op), left_(l), right_(r)
{
}

node::node(lexeme op, shared_ptr<node> r)
    : lexeme_(op), right_(r)
{
}

node::node(lexeme l)
    : lexeme_(l)
{

}

double node::get_value(const double *data)
{
    switch (lexeme_.get_type())
    {
    case apunctuator:
    {
        double lv=left_ ? left_->get_value(data) : 0;
        double rv=right_ ? right_->get_value(data) : 0;

        return calculator.get(lexeme_.get_hash(), lv, rv);
    }
    case areference:
        return data[lexeme_.get_reference()];
    case anumber:
        return lexeme_.get_number();
    case asymbol:
        return 0;
    default:
        debug::Assert<fault>(false, HERE); return 0;
    }
}

double node::get_value(data::slice *slc, unsigned index)
{
    switch (lexeme_.get_type())
    {
    case apunctuator:
    {
        double lv=left_ ? left_->get_value(slc, index) : 0;
        double rv=right_ ? right_->get_value(slc, index) : 0;

        return calculator.get(lexeme_.get_hash(), lv, rv);
    }
    case areference:
        return slc->get_value(index, (aera::chars)lexeme_.get_reference());
    case anumber:
        return lexeme_.get_number();
    case asymbol:
        return 0;
    default:
        debug::Assert<fault>(false, HERE); return 0;
    }
}


std::string node::string() const
{
    return
        lexeme_.get_string()=="()"
        ? "(" + right_->string()+")"
        : (left_ ? left_->string() : "") + lexeme_.get_string()
        + (right_ ? right_->string() : "");
}

lexeme node::get_lexeme() const
{
    return lexeme_;
}


//////////////////////////////////////////////////////////////////////////

node tree_maker(lexer &lx)
{
    syntaxer sn(STL_II(lx));
    return sn.get_process_tree();
}

node tree_maker(std::string string)
{
    lexer lx(string.size() ? string : "1");
    return tree_maker(lx);
}

node tree_maker(std::string string, vars_t &map)
{
    lexer lx(string.size() ? string : "1");
    lx.apply(map);
    return tree_maker(lx);
}

//////////////////////////////////////////////////////////////////////////

}
