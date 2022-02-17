#include "lexer.h"

Lexer::Lexer()
{
}

std::tuple<int, int, QString> Lexer::Parse()
{
    if(code.isEmpty())
        return {-1, -1, "OK"};

    PrepareCode();

    auto begin = code.begin();
    auto end = code.begin();

    while(end != code.end()){
        begin = word_begin(end); end = word_end(begin);

        auto [s_pos, e_pos, error] = ConvertToken(begin, end);

        if(error != "OK")
                return {s_pos, e_pos, error};
    }

    return {-1, -1, "OK"};
}

void Lexer::SetCode(const QString &input_code)
{
    code = input_code;
}

void Lexer::PrepareCode()
{
    code.replace('\n', ' ');

    std::string code_copy = code.toStdString();

    if(size_t found = code_copy.find_last_not_of(' '); found != std::string::npos)
        code_copy.erase(found + 1);

    code = QString::fromStdString(code_copy);

    code = code.toLower();
}

void Lexer::ClearTokens()
{
    tokens.clear();
}

bool Lexer::IsOctalDigit(const QChar symbol) const
{
    return symbol > QChar(47) && symbol < QChar(56);
}
