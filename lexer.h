#ifndef LEXER_H
#define LEXER_H
#pragma once

#include <QString>
#include <variant>
#include <QRegularExpression>
#include <QVector>
#include <QtMath>
#include <QLocale>

namespace TokenType {

    struct Number{
        long long value;
        int begin_idx;
        int end_idx;
    };

    struct Real{
        double value;
        int begin_idx;
        int end_idx;
    };

    struct LanguageKey{
        QString value;
        int begin_idx;
        int end_idx;
    };

    struct LinkKey{
        QString value;
        int begin_idx;
        int end_idx;
    };

    struct Char{
        QChar value;
        int begin_idx;
        int end_idx;
    };

    struct Variable{
        QString value;
        int begin_idx;
        int end_idx;
    };

    struct Operation{
        QChar value;
        int begin_idx;
        int end_idx;
    };

    struct Function{
        QString value;
        int begin_idx;
        int end_idx;
    };
}

using Token = std::variant<
  TokenType::Number,
  TokenType::Real,
  TokenType::LanguageKey,
  TokenType::LinkKey,
  TokenType::Char,
  TokenType::Operation,
  TokenType::Function,
  TokenType::Variable
>;


class Lexer
{
public:
    Lexer();

    template <typename Siterator>
    Siterator word_begin(Siterator from){
        return std::find_if_not(from, code.end(), [](const QChar symbol){
            return symbol == ' ';
        });
    }

    template <typename Siterator>
    Siterator word_end(Siterator from){
        auto it = std::find_if(from, code.end(), [](const QChar symbol){
            return QRegularExpression(R"([:|,|;|=|\s|+|\-|*|\/|&|\||!])").match(symbol).hasMatch();
        });

        return (from == it) ? ++it : it;
    }

    template <typename Siterator>
    std::tuple<int, int, QString> ConvertToken(Siterator begin, Siterator end){
        int idx_begin = begin - code.begin();
        int idx_end = end - code.begin();

        if(CheckIfNumber(begin, end)){
            if(!IsOctalDigit(begin ,end))
                return {idx_begin, idx_end, "Возможны только числа восьмеричного формата"};

            bool ok = true; long long result = BuildString(begin, end).toLongLong(&ok);

            if(!ok)
                return {idx_begin, idx_end, "Превышен лимит размера числа"};

            auto qwe = Token(TokenType::Number{result, idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfReal(begin, end)){
            if(!IsOctalDigit(begin ,end))
                return {idx_begin, idx_end, "Возможны только числа восьмеричного формата"};

            bool s_ok, f_ok = true; double result = ConvertToDouble(begin, end, s_ok, f_ok);

            if(!s_ok || !f_ok)
                return {begin - code.begin(), end - code.begin(), "Превышен лимит размера числа"};

            auto qwe = Token(TokenType::Real{result, idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfVariable(begin, end)){
            auto qwe = Token(TokenType::Variable{BuildString(begin, end), idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfOperation(begin)){
            auto qwe = Token(TokenType::Operation{*begin, idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfChar(begin)){
            auto qwe = Token(TokenType::Char{*begin, idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfLanguageKey(begin, end)){
            auto qwe = Token(TokenType::LanguageKey{BuildString(begin, end), idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfLinkKey(begin, end)){
            auto qwe = Token(TokenType::LinkKey{BuildString(begin, end), idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        if(CheckIfFunction(begin, end)){
            auto qwe = Token(TokenType::Function{BuildString(begin, end), idx_begin, idx_end});
            tokens.push_back(qwe);
            return {-1, -1, "OK"};
        }

        return {idx_begin, idx_end, "Неизвестный терминал: " + BuildString(begin, end)};
    }

    std::tuple<int, int, QString> Parse();
    void SetCode(const QString& input_code);
    void PrepareCode();
    void ClearTokens();

private:
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
    bool CheckIfLanguageKey(Siterator begin, Siterator end) const{
        return language_keys.contains(BuildString(begin, end));
    }

    template <typename Siterator>
    bool CheckIfLinkKey(Siterator begin, Siterator end) const{
        return link_keys.contains(BuildString(begin, end));
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
    double ConvertToDouble(Siterator begin, Siterator end, bool& f_ok, bool& s_ok) const{
        auto it = std::find(begin, end, '.');

        int integer = BuildString(begin, it).toLongLong(&f_ok);
        int fraction = BuildString(++it, end).toLongLong(&s_ok);

        return integer + fraction / qPow(10, end - ++it);
    }

    template <typename Siterator>
    QString BuildString(Siterator begin, Siterator end) const{
        QString result;

        for(; begin != end; ++begin)
            result += *begin;

        return result;
    }

    bool IsOctalDigit(const QChar symbol) const;

    template <typename Siterator>
    bool IsOctalDigit(Siterator begin, Siterator end) const{
        while (begin != end && (IsOctalDigit(*begin) || *begin == '.'))++begin;
        return begin == end;
    }

    QString code;
    QVector<Token> tokens;
    QVector<QString> language_keys = {"start", "stop"};
    QVector<QString> link_keys = {"первое", "второе", "third", "fourth"};
    QVector<QChar> chars = {';', ':', ',', '='};
    QVector<QChar> operations = {'+', '-', '*', '/', '&', '|', '!'};
    QVector<QString> functions = {"sin", "cos", "tg", "ctg"};
};

#endif // LEXER_H
