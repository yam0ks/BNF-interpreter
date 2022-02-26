#ifndef ANALYZER_H
#define ANALYZER_H
#pragma once

#include <lexer.h>

class Analyzer
{

using Expressions = std::vector<std::vector<Token>>;

public:
    Analyzer() = default;

    std::tuple<int, int, QString> AnalyzeCode(QString&& code);
    const Expressions& GetExpr();

private:
    template <typename Viterator>
    bool CheckForStart(Viterator token_it, Viterator end){
        if(token_it == end)
            return false;
        return token_it->get()->type == TokenType::Types::Start;
    }

    template <typename Viterator>
    bool CheckForStop(Viterator token_it, Viterator end){
        if(token_it == end)
            return false;
        return token_it->get()->type == TokenType::Types::Stop;
    }

    template <typename Viterator>
    std::tuple<int, int, QString> AnalyzeLinks(Viterator& begin, Viterator end){
        bool HasLink = false;

        while(true){
            if(Expect(begin, end, TokenType::Types::First)){
                HasLink = true;
                if(++begin == end)
                    return SendError(begin, end, "Analyzer Error! После ключевого слова \"Первое\" должно идти вещественное число.");
                else if(!Expect(begin, end, TokenType::Types::Real))
                    return SendError(begin, end, "Analyzer Error! После ключевого слова \"Первое\" не может идти " + TypeToStr(begin) + ". "
                                                                        + prev(begin)->get()->value + " " + begin->get()->value);
            }

            else if(Expect(begin, end, TokenType::Types::Second)){
                HasLink = true;
                bool FirstIteration = true;
                while(true){
                    if(next(begin) == end && FirstIteration)
                        return SendError(next(begin), end, "Analyzer Error! После ключевого слова \"Второе\" должно идти вещественное число.");
                    else if(!Expect(next(begin), end, TokenType::Types::Real) && FirstIteration)
                        return SendError(next(begin), end, "Analyzer Error! После ключевого слова \"Второе\" не может идти " + TypeToStr(next(begin)) + ". "
                                                                        + begin->get()->value + " " + next(begin)->get()->value);
                    else if(!Expect(next(begin), end, TokenType::Types::Real) && !FirstIteration)
                        break;
                    FirstIteration = false;
                    ++begin;
                }
            }

            else if(Expect(begin, end, TokenType::Types::Third)){
                HasLink = true;
                if(++begin == end)
                    return SendError(begin, end, "Analyzer Error! После ключевого слова \"Third\" должно идти целое число.");
                if(!Expect(begin, end, TokenType::Types::Number))
                    return SendError(begin, end, "Analyzer Error! После ключевого слова \"Third\" не может идти " + TypeToStr(begin) + ". "
                                                                        + prev(begin)->get()->value + " " + begin->get()->value);
            }

            else if(Expect(begin, end, TokenType::Types::Fourth)){
                HasLink = true;
                bool FirstIteration = true;
                while(true){
                    if(next(begin) == end  && FirstIteration)
                        return SendError(next(begin), end, "Analyzer Error! После ключевого слова \"Fourth\" должно идти целое число.");
                    else if(!Expect(next(begin), end, TokenType::Types::Number) && FirstIteration)
                        return SendError(next(begin), end, "Analyzer Error! После ключевого слова \"Fourth\" не может идти " + TypeToStr(next(begin)) + ". "
                                                                        + begin->get()->value + " " + next(begin)->get()->value);
                    else if(!Expect(next(begin), end, TokenType::Types::Number) && !FirstIteration){
                        if(Expect(next(begin), end, ',')){
                            if(next(begin, 2) == end)
                                return SendError(next(begin), end, "Analyzer Error! После запятой в конце звена ожидается целое число, либо очередное звено.");
                            if(!Expect(next(begin, 2), end, TokenType::Types::Number) && !Expect(next(begin, 3), end, ','))
                                break;
                        }
                        else break;
                    }
                    else if(Expect(next(begin), end, TokenType::Types::Number))
                        if(!Expect(next(begin, 2), end, ',')){
                             ++begin; break;
                        }

                    FirstIteration = false;
                    ++begin;
                }
            }

            if(!HasLink){
                if(begin == end)
                    return SendError(begin, end, "Analyzer Error! После ключевого слова \"Start\" ожидается звено.");
                else
                    return SendError(begin, end, "Analyzer Error! После ключевого слова \"Start\" не может идти " + TypeToStr(begin) + ". "
                                                                          + begin->get()->value);
            }
            if(HasLink && !Expect(++begin, end, ','))
                return SendOk();
            else if(!Expect(next(begin), end, TokenType::Types::First)  &&
                    !Expect(next(begin), end, TokenType::Types::Second) &&
                    !Expect(next(begin), end, TokenType::Types::Third)  &&
                    !Expect(next(begin), end, TokenType::Types::Fourth)){
                if(next(begin) == end)
                    return SendError(next(begin), end, "Analyzer Error! После запятой в конце звена ожидается очередное звено.");
                else
                    return SendError(next(begin), end, "Analyzer Error! После запятой в конце звена не может идти "
                                                          + TypeToStr(next(begin)) + ". "
                                                          + begin->get()->value + " " + next(begin)->get()->value);
            }
            ++begin;
        }
    }

    template <typename Viterator>
    std::tuple<int, int, QString> AnalyzeOperators(Viterator& begin, Viterator end){
        bool HasOperator = false;

        while(true){

            if(begin == end && !HasOperator)
                return SendError(begin, end, "Analyzer Error! После звеньев ожидается оператор.");
            if(Expect(begin, end, TokenType::Types::Stop) && !HasOperator)
                return SendError(begin, end, "Analyzer Error! После звеньев ожидается оператор. "
                                                                            + begin->get()->value);

            if(Expect(begin, end, TokenType::Types::Number)){
                while(true){
                    if(++begin == end)
                        return SendError(begin, end, "Analyzer Error! После метки ожидается очередная метка, либо двоеточие.");
                    else if(!Expect(begin, end, TokenType::Types::Number) && !Expect(begin, end, ':'))
                        return SendError(begin, end, "Analyzer Error! После метки не может идти " + TypeToStr(begin) + ". "
                                                                                + prev(begin)->get()->value + " " + begin->get()->value);
                    else if(Expect(begin, end, ':')){
                        if(next(begin) == end)
                            return SendError(next(begin), end, "Analyzer Error! После двоеточия ожидается переменная.");
                        else
                            if(!Expect(next(begin), end, TokenType::Types::Variable))
                                return SendError(begin, end, "Analyzer Error! После двоеточия не может идти " + TypeToStr(next(begin)) + ". "
                                                                                + begin->get()->value + " " + next(begin)->get()->value);
                        break;
                    }
                }
                ++begin;
            }

            else if(!Expect(begin, end, TokenType::Types::Variable))
                return SendError(begin, end, "Analyzer Error! Метка должна являться целым числом. "
                                                                            + begin->get()->value);

            std::vector<Token> expression; bool FirstIteration = true;

            expression.push_back(*begin);

            if(++begin == end)
                return SendError(begin, end, "Analyzer Error! После переменной ожидается знак равенства.");

            else if(!Expect(begin, end, '='))
                return SendError(begin, end, "Analyzer Error! После переменной не может идти " + TypeToStr(begin) + ". "
                                                            + prev(begin)->get()->value + " " + begin->get()->value);

            while(true){
                if(next(begin) == end && FirstIteration)
                    return SendError(begin, end, "Analyzer Error! Выражение должно начинаться либо с вещественного числа, либо с функции, "
                                                 "либо с переменной, либо с операции инвертирования, либо с операции отрицания.");

                else if((!Expect(next(begin), end, TokenType::Types::Real) &&
                    !Expect(next(begin), end, TokenType::Types::Function)  &&
                    !Expect(next(begin), end, TokenType::Types::Variable)  &&
                    !Expect(next(begin), end, '!')                         &&
                    !Expect(next(begin), end, '-')) && FirstIteration)
                            return SendError(next(begin), end, "Analyzer Error! После знака равенства не может идти " + TypeToStr(next(begin)) + ". "
                                                                                                + next(begin)->get()->value);

                else if(Expect(next(begin), end, TokenType::Types::Real)){
                    if(next(begin, 2) == end)
                        return SendError(next(begin), end, "Analyzer Error! После вещественного числа ожидается операция, "
                                                           "либо ключевое слово \"Stop\".");

                    else if((!Expect(next(begin, 2), end, TokenType::Types::Operation) &&
                       !Expect(next(begin, 2), end, ';')                          &&
                       !Expect(next(begin, 2), end, TokenType::Types::Stop))      ||
                        Expect(next(begin, 2), end, '!')){
                            return SendError(next(begin, 2), end, "Analyzer error! После вещественного числа не может идти " + TypeToStr(next(begin, 2)) + ". "
                                                                            + next(begin)->get()->value + " " + next(begin, 2)->get()->value);
                    }
                    FirstIteration = false;
                    expression.push_back(*next(begin));
                }

                else if(Expect(next(begin), end, TokenType::Types::Variable)){
                    if(next(begin, 2) == end)
                        return SendError(next(begin, 2), end, "Analyzer Error! После переменной ожидается операция."
                                                              "либо ключевое слово \"Stop\".");

                    else if((!Expect(next(begin, 2), end, TokenType::Types::Operation) &&
                       !Expect(next(begin, 2), end, ';')                          &&
                       !Expect(next(begin, 2), end, TokenType::Types::Stop))      ||
                        Expect(next(begin, 2), end, '!')){
                            return SendError(next(begin, 2), end, "Analyzer error! После переменной не может идти " + TypeToStr(next(begin, 2)) + ". "
                                                                            + next(begin)->get()->value + " " + next(begin, 2)->get()->value);
                    }
                    FirstIteration = false;
                    expression.push_back(*next(begin));
                }

                else if(Expect(next(begin), end, TokenType::Types::Function)){
                    if(next(begin, 2) == end)
                        return SendError(next(begin, 2), end, "Analyzer Error! После функции ожидается вещественное число, либо переменная, "
                                                              "либо другая функция, либо операции \"!\" и \"-\".");

                    else if(!Expect(next(begin, 2), end, TokenType::Types::Function) &&
                       !Expect(next(begin, 2), end, TokenType::Types::Real)          &&
                       !Expect(next(begin, 2), end, TokenType::Types::Variable)      &&
                       !Expect(next(begin, 2), end, TokenType::Types::Function)){
                        return SendError(next(begin, 2), end, "Analyzer error! После функции не может идти " + TypeToStr(next(begin, 2)) + ". "
                                                                        + next(begin)->get()->value + " " + next(begin, 2)->get()->value);
                    }
                    FirstIteration = false;
                    expression.push_back(*next(begin));
                }

                else if(Expect(next(begin), end, TokenType::Types::Operation)){
                    QString str_type;
                    switch(QChar c = next(begin)->get()->value[0]; c.unicode()){
                    case '+' :
                        str_type = "операции сложения";
                    break;
                    case '-' :
                        str_type =  "знака минус";
                    break;
                    case '/' :
                        str_type =  "операции деления";
                    break;
                    case '*' :
                        str_type =  "операции умножения";
                    break;
                    case '!' :
                        str_type =  "операции инвертирования";
                    break;
                    case '&' :
                        str_type =  "операции поразрядной конъюнкции";
                    break;
                    case '|' :
                        str_type =  "операции поразрядной дизъюнкции";
                    break;
                    default  :
                    break;
                    }

                    if(next(begin, 2) == end){
                        return (Expect(next(begin), end, '!')) ?
                                    SendError(next(begin, 2), end, "Analyzer Error! После " + str_type +
                                              " ожидается функция, либо переменная, либо вещественное число.") :
                                    SendError(next(begin, 2), end, "Analyzer Error! После " + str_type +
                                              " ожидается функция, либо переменная, либо вещественное число, либо операция инвертирования.");
                    }


                    else if(Expect(next(begin), end, '!')){
                        if(!Expect(next(begin, 2), end, TokenType::Types::Function) &&
                           !Expect(next(begin, 2), end, TokenType::Types::Real)     &&
                           !Expect(next(begin, 2), end, TokenType::Types::Variable))
                            return SendError(next(begin, 2), end, "Analyzer Error! После операции инвертирования не может идти " + TypeToStr(next(begin, 2)) + ". "
                                                                                            + next(begin)->get()->value + " " + next(begin, 2)->get()->value);
                    }

                    else if(!Expect(next(begin, 2), end, TokenType::Types::Function) &&
                            !Expect(next(begin, 2), end, TokenType::Types::Real)     &&
                            !Expect(next(begin, 2), end, TokenType::Types::Variable) &&
                            !Expect(next(begin, 2), end, '!')){
                            return SendError(next(begin, 2), end, "Analyzer Error! После " + str_type + " не может идти " + TypeToStr(next(begin, 2)) + ". "
                                                                                        + next(begin)->get()->value + " " + next(begin, 2)->get()->value);
                    }
                    FirstIteration = false;
                    expression.push_back(*next(begin));
                }

                else if(Expect(next(begin), end, ';')){
                    if(next(begin, 2) == end)
                        return SendError(next(begin), end, "Analyzer error! После точки с запятой ожидается очередной оператор.");
                    else if(!Expect(next(begin, 2), end, TokenType::Types::Number) && !Expect(next(begin, 2), end, TokenType::Types::Variable))
                        return SendError(next(begin), end, "Analyzer error! После точки с запятой в конце оператора не может идти " + TypeToStr(next(begin, 2)) + ". "
                                                                                                    + next(begin, 2)->get()->value);
                    ++begin;
                    break;
                }
                else
                    break;

                HasOperator = true;
                ++begin;
            }
            FirstIteration = true;
            expressions.push_back(expression);
            ++begin;

            if((begin == end || Expect(begin, end, TokenType::Types::Stop)) && HasOperator)
                return SendOk();
        }
    }

    template <typename Viterator>
    bool Expect(Viterator token_it, Viterator end, TokenType::Types type){
        if(token_it == end)
            return false;
        return token_it->get()->type == type;
    }

    template <typename Viterator>
    bool Expect(Viterator token_it, Viterator end, const QChar pattern){
        if(token_it == end)
            return false;
        return token_it->get()->value == pattern;
    }

    template <typename Viterator>
    bool Expect(Viterator token_it, Viterator end, const QString& pattern){
        if(token_it == end)
            return false;
        return token_it->get()->value == pattern;
    }

    std::tuple<int, int, QString> SendOk(){
        return {0, 0, "OK"};
    }

    template <typename Viterator>
    std::tuple<int, int, QString> SendError(Viterator begin, Viterator end, const QString& error, HighLiteOption option = HighLiteOption::Right){
        if(begin == end)
            return {(std::prev(begin))->get()->begin_idx, (std::prev(begin))->get()->end_idx, error};
        if(next(begin) == end)
            return {(begin)->get()->begin_idx, begin->get()->end_idx, error};

        switch(option){
            case HighLiteOption::Between :
                return {std::prev(begin)->get()->end_idx, begin->get()->begin_idx, error};
            break;
            case HighLiteOption::Left :
                return {std::prev(begin)->get()->begin_idx, std::prev(begin)->get()->end_idx, error};
            break;
            case HighLiteOption::Right :
                return {begin->get()->begin_idx, begin->get()->end_idx, error};
            break;
            case HighLiteOption::Both :
                return {std::prev(begin)->get()->begin_idx, begin->get()->end_idx, error};
            break;
            case HighLiteOption::None :
                return {0, 0, error};
            break;
        }
        return {begin->get()->begin_idx, begin->get()->end_idx, error};
    }

    std::tuple<int, int, QString> SendError(const int b_idx, const int e_idx, const QString& error){
        return {b_idx, e_idx, error};
    }

template <typename Viterator>
QString TypeToStr(Viterator token_it){
    switch(token_it->get()->type){
        case TokenType::Types::Char :
            switch(QChar symbol = token_it->get()->value[0]; symbol.unicode()){
                case ';' :
                    return "точка с запятой";
                break;
                case ':' :
                    return "двоеточие";
                break;
                case ',' :
                    return "запятая";
                break;
                case '=' :
                    return "знак равенства";
                break;
                default :
                    return QString();
                break;
            }
        break;
        case TokenType::Types::Real :
            return "вещественное число";
        break;
        case TokenType::Types::Stop :
            return "ключевое слово \"Stop\"";
        break;
        case TokenType::Types::First :
            return "ключевое слово \"Первое\"";
        break;
        case TokenType::Types::Start :
            return "ключевое слово \"Start\"";
        break;
        case TokenType::Types::Third :
            return "ключевое слово \"Third\"";
        break;
        case TokenType::Types::Fourth :
            return "ключевое слово \"Fourth\"";
        break;
        case TokenType::Types::Number :
            return "целое число";
        break;
        case TokenType::Types::Second :
            return "ключевое слово \"Второе\"";
        break;
        case TokenType::Types::Function :
            return "функция";
        break;
        case TokenType::Types::Variable :
            return "переменная";
        break;
        case TokenType::Types::Operation :
            switch(QChar symbol = token_it->get()->value[0]; symbol.unicode()){
                case '+' :
                    return "операция сложения";
                break;
                case '-' :
                    return "операция вычитания";
                break;
                case '/' :
                    return "операция деления";
                break;
                case '*' :
                    return "операция умножения";
                break;
                case '!' :
                    return "операция инвертирования";
                break;
                case '&' :
                    return "операция поразрядной конъюнкции";
                break;
                case '|' :
                    return "операция поразрядной дизъюнкции";
                break;
                default  :
                    return QString();
                break;
            }
        break;
    }
    return QString();
}

    Lexer lexer;
    Expressions expressions;
};

#endif // ANALYZER_H
