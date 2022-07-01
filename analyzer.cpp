#include "analyzer.h"

std::tuple<int, int, QString> Analyzer::AnalyzeCode(QString&& code)
{
    auto [begin_idx, end_idx, error] = lexer.Parse(std::move(code));

    expressions.clear();

    if(error != QString())
            return {begin_idx, end_idx, error};

    auto begin = lexer.GetTokens().begin();
    auto end = lexer.GetTokens().end();

    if(begin == end)
        return Logger::SendOk();

    if(!CheckForStart(begin, end))
        return Logger::SendError(begin, end, "Analyzer error! Программа должна начинаться со слова Start.");

    if(auto [b_idx, e_idx, error] = AnalyzeLinks(++begin, end); error != QString())
        return Logger::SendError(b_idx, e_idx, error);

    if(auto [b_idx, e_idx, error] = AnalyzeOperators(begin, end); error != QString())
        return Logger::SendError(b_idx, e_idx, error);

    if(!CheckForStop(begin, end) || ++begin != end)
        return Logger::SendError(begin, end, "Analyzer error! Программа должна заканчиваться словом Stop.");

    return Logger::SendOk();
}

const Analyzer::Expressions &Analyzer::GetExpr() const
{
    return expressions;
}
