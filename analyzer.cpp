#include "analyzer.h"

Analyzer::Analyzer()
{

}

std::tuple<int, int, QString> Analyzer::AnalyzeCode(QString&& code)
{
    auto [begin_idx, end_idx, error] = lexer.Parse(std::move(code));

    expressions.clear();

    if(error != "OK")
            return {begin_idx, end_idx, error};

    auto begin = lexer.GetTokens().begin();
    auto end = lexer.GetTokens().end();

    if(begin == end)
        return SendOk();

    if(!CheckForStart(begin, end))
        return SendError(begin, end, "Analyzer error! Программа должна начинаться со слова Start.");

    if(auto [b_idx, e_idx, error] = AnalyzeLinks(++begin, end); error != "OK")
        return SendError(b_idx, e_idx, error);

    if(auto [b_idx, e_idx, error] = AnalyzeOperators(begin, end); error != "OK")
        return SendError(b_idx, e_idx, error);

    if(!CheckForStop(begin, end) || ++begin != end)
        return SendError(begin, end, "Analyzer error! Программа должна заканчиваться словом Stop.");

    return SendOk();
}

const Analyzer::Expressions &Analyzer::GetExpr()
{
    return expressions;
}
