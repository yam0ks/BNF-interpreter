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
        BaseToken(const int b_idx, const int e_idx, const Types t, const QString& val)
            : begin_idx(b_idx), end_idx(e_idx), type(t), value(val){}

        int begin_idx;
        int end_idx;
        Types type;
        QString value;
    };

    struct Number  : public BaseToken{
        Number(const int b_idx, const int e_idx, const Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Real : public BaseToken{
        Real(const int b_idx, const int e_idx, const Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Start : public BaseToken{
        Start(const int b_idx, const int e_idx, const Types type)
            : BaseToken(b_idx, e_idx, type, "Start"){}
    };

    struct Stop : public BaseToken{
        Stop(const int b_idx, const int e_idx, const Types type)
            : BaseToken(b_idx, e_idx, type, "Stop"){}
    };

    struct First : public BaseToken{
        First(const int b_idx, const int e_idx, const Types type)
            : BaseToken(b_idx, e_idx, type, "Первое"){}
    };

    struct Second : public BaseToken{
        Second(const int b_idx, const int e_idx, const Types type)
            : BaseToken(b_idx, e_idx, type, "Второе"){}
    };

    struct Third : public BaseToken{
        Third(const int b_idx, const int e_idx, const Types type)
            : BaseToken(b_idx, e_idx, type, "Third"){}
    };

    struct Fourth : public BaseToken{
        Fourth(const int b_idx, const int e_idx, const Types type)
            : BaseToken(b_idx, e_idx, type, "Fourth"){}
    };

    struct Char : public BaseToken{
        Char(const int b_idx, const int e_idx, const Types type, const QChar val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Variable : public BaseToken{
        Variable(const int b_idx, const int e_idx, const Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Operation : public BaseToken{
        Operation(const int b_idx, const int e_idx, const Types type, const QChar val)
            : BaseToken(b_idx, e_idx, type, val){}
    };

    struct Function : public BaseToken{
        Function(const int b_idx, const int e_idx, const Types type, const QString& val)
            : BaseToken(b_idx, e_idx, type, val){}
    };
}

using Token = std::shared_ptr<TokenType::BaseToken>;

class Lexer
{
public:
    Lexer() = default;

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
    std::tuple<int, int, QString> ConvertToken(Siterator begin, Siterator end, const QString& code){
        const int idx_begin = begin - code.begin();
        const int idx_end = end - code.begin();

        if(auto [b_offset, error] = CheckIfWrongFunction(begin, end); error != QString())
            return SendError(idx_begin + b_offset, idx_end, error + BuildString(begin, end));
        if(QString error = CheckIfWrongVariable(begin, end); error != QString())
            return SendError(idx_begin, idx_end, error + BuildString(begin, end));

        if(CheckIfNumber(begin, end)){
            if(*begin == '0' && std::next(begin) != end)
                return SendError(idx_begin, idx_end, "Parser error! Целое число не может начинаться с нуля. " + BuildString(begin, end));

            if(!IsOctalDigit(begin, end))
                return SendError(idx_begin, idx_end, "Parser error! Возможны только числа восьмеричного формата. " + BuildString(begin, end));

            if(QString error = CheckLimits(BuildString(begin, end)); error != QString())
                return SendError(idx_begin, idx_end, error + BuildString(begin, end));

            tokens.push_back(std::make_shared<TokenType::Number>(idx_begin, idx_end, TokenType::Types::Number, BuildString(begin, end)));
            return SendOk();
        }

        if(CheckIfReal(begin, end)){
            if(*begin == '0' && *std::next(begin) != '.')
                return SendError(idx_begin, idx_end, "Parser error! Неверное количество разрядов в вещественном числе с отсутствующей целой частью. "
                                                                                                                + BuildString(begin, end));

            if(!IsOctalDigit(begin, end))
                return SendError(idx_begin, idx_end, "Parser error! Возможны только числа восьмеричного формата. " + BuildString(begin, end));

            if(QString error = CheckLimits(BuildString(begin, end)); error != QString())
                return SendError(idx_begin, idx_end, error + BuildString(begin, end));

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
            QVector<QString> reserved_words = {"sin", "cos", "tg", "ctg", "Start", "Stop", "Первое", "Второе", "Third", "Fourth"};

            for(const auto& word : reserved_words){
                if(auto variable = BuildString(begin, end); variable.contains(word))
                    return SendError(idx_begin, idx_end, "Parser Error! Переменная не может содержать зарезервированное слово \"" + word + "\". " + variable);
            }

            tokens.push_back(std::make_shared<TokenType::Variable>(idx_begin, idx_end, TokenType::Types::Variable, BuildString(begin, end)));
            return SendOk();
        }

        return SendError(idx_begin, idx_end, "Parser error! Неизвестный терминал: " + BuildString(begin, end));
    }

    std::tuple<int, int, QString> Parse(QString&& code);
    const std::vector<Token>& GetTokens() const;

private:
    template <typename Siterator>
    Siterator word_begin(Siterator from, Siterator to) const{
        return std::find_if_not(from, to, [](const QChar symbol){
            return QRegularExpression(R"([\s\t])").match(symbol).hasMatch();;
        });
    }

    template <typename Siterator>
    Siterator word_end(Siterator from, Siterator to) const{
        auto it = std::find_if(from, to, [](const QChar symbol){
            return QRegularExpression(R"([:,;=\s+\-*\/&|!\t])").match(symbol).hasMatch();
        });

        return (from == it) ? ++it : it;
    }

    template <typename Siterator>
    bool CheckIfNumber(Siterator begin, Siterator end) const{
        while (begin != end && (*begin).isDigit()) ++begin;
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
    std::tuple<int, QString> CheckIfWrongFunction(Siterator begin, Siterator end) const{

        auto it = begin;

        if(BuildString(begin, end).indexOf(QRegularExpression(R"(sin|cos|ctg)")) == 0)
            it = std::next(it, 3);
        else if(BuildString(begin, end).indexOf(QRegularExpression(R"(tg)")) == 0)
            it = std::next(it, 2);
        else return {0, QString()};

            if(it == end)
                return {0, QString()};

            if(CheckIfReal(it, end)     ||
               CheckIfVariable(it, end))
                return {it - begin, "Parser Error! Аргумент функции должен подаваться отдельно от функции. "};
            if(CheckIfNumber(it, end))
                return {it - begin, "Parser Error! Аргумент функции не может быть целым числом. "};

        return {0, QString()};
    }

    template <typename Siterator>
    QString CheckIfWrongVariable(Siterator begin, Siterator end) const{
        auto it = begin;

        if(QRegularExpression(R"(\d)").match(*begin).hasMatch())
            it = std::find_if_not(begin, end, [](const QChar symbol){
                return QRegularExpression(R"([\d\.])").match(symbol).hasMatch();
        });
        else if(QRegularExpression(R"(\w)").match(*begin).hasMatch())
            it = std::find_if(begin, end, [](const QChar symbol){
                return QRegularExpression(R"(\d)").match(symbol).hasMatch();
        });
        else return QString();

        if(it == end)
            return QString();

        if((CheckIfReal(begin, it) && CheckIfVariable(it, end)) ||
           (CheckIfVariable(begin, it) && CheckIfReal(it, end)))
            return "Parser Error! Имя переменной не может содержать вещественное число. ";
        if(CheckIfNumber(begin, it) && CheckIfVariable(it, end))
            return "Parser Error! Имя переменной не может начинаться с цифры. ";
        return QString();
    }

    template <typename Siterator>
    QString BuildString(Siterator begin, Siterator end) const{
        QString result;

        result.resize(end - begin);
        std::copy(begin, end, result.begin());

        return result;
    }

    template <typename Siterator>
    bool IsOctalDigit(Siterator begin, Siterator end) const{
        while (begin != end && (IsOctalDigit(*begin) || *begin == '.')) ++begin;
        return begin == end;
    }

    std::tuple<int, int, QString> SendOk(){
        return {0, 0, QString()};
    }

    std::tuple<int, int, QString> SendError(const int b_idx, const int e_idx, const QString& error){
        return {b_idx, e_idx, error};
    }

    bool IsOctalDigit(const QChar symbol) const;
    void PrepareCode(QString& code);
    void ClearTokens();
    static QString CheckLimits(const QString& value);

    std::vector<Token> tokens;
    QVector<QChar> chars = {';', ':', ',', '='};
    QVector<QChar> operations = {'+', '-', '*', '/', '&', '|', '!'};
    QVector<QString> functions = {"sin", "cos", "tg", "ctg"};
};

#endif // LEXER_H
