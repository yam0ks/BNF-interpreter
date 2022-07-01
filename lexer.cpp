#include "lexer.h"

std::tuple<int, int, QString> Lexer::Parse(QString&& code)
{
    if(code.isEmpty())
        return SendError(0, 0, "Parser Error! На вход подана пустая программа.");

    PrepareCode(code);
    ClearTokens();

    auto begin = code.begin();
    auto end = code.begin();

    while(end != code.end()){
        begin = word_begin(end, code.end()); end = word_end(begin, code.end());

        auto [s_pos, e_pos, error] = ConvertToken(begin, end, code);

        if(error != QString())
            return SendError(s_pos, e_pos, error);
    }

    return SendOk();
}

const std::vector<Token> &Lexer::GetTokens() const
{
    return tokens;
}

std::tuple<int, int, QString> Lexer::SendOk(){
    return {0, 0, QString()};
}

std::tuple<int, int, QString> Lexer::SendError(const int b_idx, const int e_idx, const QString &error){
    return {b_idx, e_idx, error};
}

void Lexer::PrepareCode(QString& code)
{
    code.replace('\n', ' ');

    auto end_idx = code.lastIndexOf(QRegularExpression(R"(\S)"));
    code.remove(end_idx + 1, code.length() - end_idx);
}

void Lexer::ClearTokens()
{
    tokens.clear();
}

QString Lexer::CheckLimits(const QString &value)
{
    QStringList parts = value.split(".");

    if(parts.size() == 1 && parts[0].length() > 8)
        return "Parser Error! Превышен допустимый размер целого числа. ";

    if(parts[0].length() > 8)
        return "Parser Error! Превышен допустимый размер целой части вещественного числа. ";
    if(parts.size() == 2 && parts[1].length() > 6)
        return "Parser Error! Превышен допустимый размер целой дробной вещественного числа. ";
    return QString();
}

bool Lexer::IsOctalDigit(const QChar symbol) const
{
    return symbol > QChar(47) && symbol < QChar(56);
}
