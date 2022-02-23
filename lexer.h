#ifndef LEXER_H
#define LEXER_H
#pragma once

#include <QString>
#include <QRegularExpression>
#include <vector>
#include <memory>
#include <QtMath>
#include <QLocale>

namespace TokenType {

enum class Types{
    Number,
    Real,
    Start,
    Stop,
    First,
    Second,
    Third,
    Fourth,
    Char,
    Variable,
    Operation,
    Function
};

    struct BaseToken{
        BaseToken(const int b_idx, const int e_idx, Types t, const QString& val)
            : begin_idx(b_idx), end_idx(e_idx), type(t), value(val){}

        int begin_idx;
        int end_idx;
        Types type;
        QString value;
    };

    struct Number  : public BaseToken{
        Number(const int b_idx, const int e_idx, Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Real : public BaseToken{
        Real(const int b_idx, const int e_idx, Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Start : public BaseToken{
        Start(const int b_idx, const int e_idx, Types type)
            : BaseToken(b_idx, e_idx, type, "Start"){}
    };

    struct Stop : public BaseToken{
        Stop(const int b_idx, const int e_idx, Types type)
            : BaseToken(b_idx, e_idx, type, "Stop"){}
    };

    struct First : public BaseToken{
        First(const int b_idx, const int e_idx, Types type)
            : BaseToken(b_idx, e_idx, type, "Первое"){}
    };

    struct Second : public BaseToken{
        Second(const int b_idx, const int e_idx, Types type)
            : BaseToken(b_idx, e_idx, type, "Второе"){}
    };

    struct Third : public BaseToken{
        Third(const int b_idx, const int e_idx, Types type)
            : BaseToken(b_idx, e_idx, type, "Third"){}
    };

    struct Fourth : public BaseToken{
        Fourth(const int b_idx, const int e_idx, Types type)
            : BaseToken(b_idx, e_idx, type, "Fourth"){}
    };

    struct Char : public BaseToken{
        Char(const int b_idx, const int e_idx, Types type, const QChar val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Variable : public BaseToken{
        Variable(const int b_idx, const int e_idx, Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Operation : public BaseToken{
        Operation(const int b_idx, const int e_idx, Types type, const QChar val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Function : public BaseToken{
        Function(const int b_idx, const int e_idx, Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };
}

using Token = std::shared_ptr<TokenType::BaseToken>;

enum class HighLiteOption{
    Between,
    Right,
    Left,
    Both,
    None
};

class Lexer
{
public:
    Lexer();

    template <typename Siterator>
    std::tuple<int, int, QString> ConvertToken(Siterator begin, Siterator end, const QString& code){
        int idx_begin = begin - code.begin();
        int idx_end = end - code.begin();

        if(CheckIfNumber(begin, end)){
            if(*begin == '0' && std::next(begin) != end)
                return SendError(idx_begin, idx_end, "Parser error! Целое число не может начинаться с нуля.");

            if(!IsOctalDigit(begin ,end))
                return SendError(idx_begin, idx_end, "Parser error! Возможны только числа восьмеричного формата");

            bool ok = true; BuildString(begin, end).toLongLong(&ok);

            if(!ok)
                return SendError(idx_begin, idx_end, "Parser error! Превышен лимит размера числа");

            tokens.push_back(std::make_shared<TokenType::Number>(idx_begin, idx_end, TokenType::Types::Number, BuildString(begin, end)));
            return SendOk();
        }

        if(CheckIfReal(begin, end)){
            if(*begin == '0' && *std::next(begin) != '.')
                return SendError(idx_begin, idx_end, "Parser error! Неверное количество разрядов в вещественном числе с отсутствующей целой частью.");

            if(!IsOctalDigit(begin ,end))
                return SendError(idx_begin, idx_end, "Parser error! Возможны только числа восьмеричного формата");

            bool ok = true; BuildString(begin, end).toDouble(&ok);

            if(!ok)
                return SendError(idx_begin, idx_end, "Parser error! Превышен лимит размера числа");

            tokens.push_back(std::make_shared<TokenType::Real>(idx_begin, idx_end, TokenType::Types::Real, BuildString(begin, end)));
            return SendOk();
        }

        if(CheckIfOperation(begin)){
            tokens.push_back(std::make_shared<TokenType::Operation>(idx_begin, idx_end, TokenType::Types::Operation, *begin));
            return SendOk();
        }

        if(CheckIfChar(begin)){
            tokens.push_back(std::make_shared<TokenType::Char>(idx_begin, idx_end, TokenType::Types::Char, *begin));
            return SendOk();
        }

        if(CheckIfStart(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Start>(idx_begin, idx_end, TokenType::Types::Start));
            return SendOk();
        }

        if(CheckIfStop(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Stop>(idx_begin, idx_end, TokenType::Types::Stop));
            return SendOk();
        }

        if(CheckIfFirst(begin, end)){
            tokens.push_back(std::make_shared<TokenType::First>(idx_begin, idx_end, TokenType::Types::First));
            return SendOk();
        }

        if(CheckIfSecond(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Second>(idx_begin, idx_end, TokenType::Types::Second));
            return SendOk();
        }

        if(CheckIfThird(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Third>(idx_begin, idx_end, TokenType::Types::Third));
            return SendOk();
        }

        if(CheckIfFourth(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Fourth>(idx_begin, idx_end, TokenType::Types::Fourth));
            return SendOk();
        }

        if(CheckIfFunction(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Function>(idx_begin, idx_end, TokenType::Types::Function, BuildString(begin, end)));
            return SendOk();
        }

        if(CheckIfVariable(begin, end)){
            tokens.push_back(std::make_shared<TokenType::Variable>(idx_begin, idx_end, TokenType::Types::Variable, BuildString(begin, end)));
            return SendOk();
        }

        return SendError(idx_begin, idx_end, "Parser error! Неизвестный терминал: " + BuildString(begin, end));
    }

    std::tuple<int, int, QString> Parse(QString&& code);

    const std::vector<Token>& GetTokens() const;

private:
    template <typename Siterator>
    Siterator word_begin(Siterator from, Siterator to){
        return std::find_if_not(from, to, [](const QChar symbol){
            return QRegularExpression(R"([\s\t])").match(symbol).hasMatch();;
        });
    }

    template <typename Siterator>
    Siterator word_end(Siterator from, Siterator to){
        auto it = std::find_if(from, to, [](const QChar symbol){
            return QRegularExpression(R"([:,;=\s+\-*\/&|!\t])").match(symbol).hasMatch();
        });

        return (from == it) ? ++it : it;
    }

    template <typename Siterator>
    bool CheckIfNumber(Siterator begin, Siterator end) const{
        while (begin != end && (*begin).isDigit())++begin;
        return begin == end;
    }

    template <typename Siterator>
    bool CheckIfReal(Siterator begin, Siterator end) const{
        if(*std::prev(end) == '.' || *begin == '.')
            return false;

        int pointscount = 0;

        for(; begin != end; ++begin){
            if(!(*begin).isDigit() && *begin != '.') return false;
            if(*begin == '.') ++pointscount;
        }

        return (pointscount == 1) ? true : false;
    }

    template <typename Siterator>
    bool CheckIfStart(Siterator begin, Siterator end) const{
        return BuildString(begin, end) == "Start";
    }

    template <typename Siterator>
    bool CheckIfStop(Siterator begin, Siterator end) const{
        return BuildString(begin, end) == "Stop";
    }

    template <typename Siterator>
    bool CheckIfFirst(Siterator begin, Siterator end) const{
        return BuildString(begin, end) == "Первое";
    }

    template <typename Siterator>
    bool CheckIfSecond(Siterator begin, Siterator end) const{
        return BuildString(begin, end) == "Второе";
    }

    template <typename Siterator>
    bool CheckIfThird(Siterator begin, Siterator end) const{
        return BuildString(begin, end) == "Third";
    }

    template <typename Siterator>
    bool CheckIfFourth(Siterator begin, Siterator end) const{
        return BuildString(begin, end) == "Fourth";
    }

    template <typename Siterator>
    bool CheckIfChar(Siterator begin) const{
        return chars.contains(*begin);
    }

    template <typename Siterator>
    bool CheckIfOperation(Siterator begin) const{
        return operations.contains(*begin);
    }

    template <typename Siterator>
    bool CheckIfFunction(Siterator begin, Siterator end) const{
        return functions.contains(BuildString(begin, end));
    }

    template <typename Siterator>
    bool CheckIfVariable(Siterator begin, Siterator end) const{
        QString str = BuildString(begin, end);

        if(QChar(str[0]) < QChar(65)   || QChar(str[0]) > QChar(1105)  ||
          (QChar(str[0]) > QChar(90)   && QChar(str[0]) < QChar(97))   ||
          (QChar(str[0]) > QChar(122)  && QChar(str[0]) < QChar(1025)) ||
          (QChar(str[0]) > QChar(1025) && QChar(str[0]) < QChar(1040)) ||
          (QChar(str[0]) > QChar(1103) && QChar(str[0]) < QChar(1105))) return false;

        for(int i = 1; i < str.size(); ++i){
            if(QChar(str[i]) < QChar(48)   || QChar(str[i]) > QChar(1105)  ||
              (QChar(str[i]) > QChar(55)   && QChar(str[i]) < QChar(65))   ||
              (QChar(str[i]) > QChar(90)   && QChar(str[i]) < QChar(97))   ||
              (QChar(str[i]) > QChar(122)  && QChar(str[i]) < QChar(1025)) ||
              (QChar(str[i]) > QChar(1025) && QChar(str[i]) < QChar(1040)) ||
              (QChar(str[i]) > QChar(1103) && QChar(str[i]) < QChar(1105))) return false;
        }

        return true;
    }

    template <typename Siterator>
    QString BuildString(Siterator begin, Siterator end) const{
        QString result;

        result.resize(end - begin);
        std::copy(begin, end, result.begin());

        return result;
    }

    bool IsOctalDigit(const QChar symbol) const;

    template <typename Siterator>
    bool IsOctalDigit(Siterator begin, Siterator end) const{
        while (begin != end && (IsOctalDigit(*begin) || *begin == '.'))++begin;
        return begin == end;
    }

    void PrepareCode(QString& code);
    void ClearTokens();

    std::tuple<int, int, QString> SendOk(){
        return {-1, -1, "OK"};
    }

    std::tuple<int, int, QString> SendError(const int b_idx, const int e_idx, const QString& error){
        return {b_idx, e_idx, error};
    }

    std::vector<Token> tokens;
    QVector<QChar> chars = {';', ':', ',', '='};
    QVector<QChar> operations = {'+', '-', '*', '/', '&', '|', '!'};
    QVector<QString> functions = {"sin", "cos", "tg", "ctg"};
};

#endif // LEXER_H
