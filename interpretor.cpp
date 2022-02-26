#include "interpretor.h"

Interpretor::Interpretor()
{

}

std::tuple<int, int, QString> Interpretor::InterpretCode(QString&& code)
{
    var_results.clear();

    auto [begin_idx, end_idx, error] = an.AnalyzeCode(std::move(code));

    if(error != "OK")
            return {begin_idx, end_idx, error};

    for(const auto& expression : an.GetExpr()){
        if(auto [begin_idx, end_idx, error] = EvaluateExpr(expression); error != "OK")
            return {begin_idx, end_idx, error};
    }

    return {-1, -1, "OK"};
}

QString Interpretor::OutputValues()
{
    QString result = "Результаты вычислений: \n";

    for(auto i = var_results.begin(); i != var_results.end(); ++i){
        result = result + i.key() + " = " + QString::number(i.value(), 'f', 10);
        auto idx = result.lastIndexOf(QRegularExpression(R"([^0])"));
        result.remove((result[idx] == '.') ? idx : idx + 1, result.length() - idx);
        result += "\n";
    }
    return result;
}

std::tuple<int, int, QString> Interpretor::EvaluateExpr(const std::vector<Token>& expression)
{
    bool is_unary = true; bool var_name = true;
    std::stack<double> values;
    std::stack<QString> operations;
    QString variable;

    for(const Token& token : expression){
        if(var_name && token->type == TokenType::Types::Variable){
            variable = token->value;
            var_name = false;
        }
        else if(token->type == TokenType::Types::Real){
            values.push(token->value.toDouble());
            is_unary = var_name = false;
        }
        else if(token->type == TokenType::Types::Variable){
            if(!var_results.contains(token->value))
                return {token->begin_idx, token->end_idx, "Interpretor Error! Использование неинициализированной переменной. " + token->value};
            values.push(var_results[token->value]);
            is_unary = var_name = false;
        }
        else if(token->type == TokenType::Types::Function){
            while(!operations.empty() && Opriority(operations.top()) > Opriority(token->value)){
                if(QString error = Evaluate(values, operations.top()); error != QString())
                    return {0, 0, "Interpretor Error! В процессе вычисления переменной " + variable + error};
                operations.pop();
            }
            operations.push(token->value);
            is_unary = var_name = false;
        }
        else if(token->type == TokenType::Types::Operation){
            QString operation = token->value;
            if(is_unary && operation == "-")
                operation = "--";
            while(!operations.empty() && Opriority(operations.top()) >= Opriority(token->value)){
                if(QString error = Evaluate(values, operations.top()); error != QString())
                    return {0, 0, "Interpretor Error! В процессе вычисления переменной " + variable + error};
                operations.pop();
            }
            operations.push(operation);
            is_unary = var_name = false;
        }
    }

    while(!operations.empty()){
        if(QString error = Evaluate(values, operations.top()); error != QString())
            return {0, 0, "Interpretor Error! В процессе вычисления переменной " + variable + error};
        operations.pop();
    }

    var_results[variable] = values.top();
    return {-1, -1, "OK"};
}

int Interpretor::Opriority(const QString &op)
{
    if(op == "--")
        return 3;
    if(op == "sin" || op == "cos" || op == "tg" || op == "ctg")
        return 5;
    switch(QChar c = op[0]; c.unicode()){
        case '+' :
            return 0;
        break;
        case '-' :
            return 0;
        break;
        case '*' :
            return 1;
        break;
        case '/' :
            return 1;
        break;
        case '&' :
            return 2;
        break;
        case '|' :
            return 2;
        break;
        case '!' :
            return 4;
        break;
        default  :
        break;
    }
    return 0;
}

QString Interpretor::Evaluate(std::stack<double> &values, const QString &op)
{
    if(op == "!"){
        double value = values.top(); values.pop();
        values.push(ToOctal(InverseValue(FromOctal(value))));
    }
    else if(op == "--"){
        double value = values.top(); values.pop();
        values.push(-value);
    }
    else if(op == "sin"){
        double value = values.top(); values.pop();
        values.push(ToOctal(sin(FromOctal(value))));
    }
    else if(op == "cos"){
        double value = values.top(); values.pop();
        values.push(ToOctal(cos(FromOctal(value))));
    }
    else if(op == "tg"){
        double value = values.top(); values.pop();
        values.push(ToOctal(tan(FromOctal(value))));
    }
    else if(op == "ctg"){
        double value = values.top(); values.pop();
        values.push(ToOctal(1 / tan(FromOctal(value))));
    }
    else{

        double r_value = FromOctal(values.top()); values.pop();
        double l_value = FromOctal(values.top()); values.pop();
        constexpr long long MAX_SIZE = 77777767700000;

        switch(QChar c = op[0]; c.unicode()){
            case '+' :
                 values.push(ToOctal(l_value + r_value));
            break;
            case '-' :
                 values.push(ToOctal(l_value - r_value));
            break;
            case '*' :
                if(auto result = ToOctal(l_value * r_value); result > MAX_SIZE)
                    return " произошло переполнение переменной.";
                else
                    values.push(result);
            break;
            case '/' :
                 if(r_value == 0)
                     return " возникло деление на ноль.";
                 values.push(ToOctal(l_value / r_value));
            break;
            case '&' :
                 values.push(ToOctal(ConjuctValues(l_value, r_value)));
            break;
            case '|' :
                 values.push(ToOctal(DisjunctValues(l_value, r_value)));
            break;
            default  :
            break;
        }
    }
    return QString();
}

double Interpretor::InverseValue(const double value)
{
    QString binary_value = ToBinary(value);

    for(int i = 0; i < binary_value.size(); ++i){
        if(binary_value[i] == '1')
            binary_value[i] = '0';
        else if(binary_value[i] == '0')
            binary_value[i] = '1';
    }

    return FromBinary(binary_value);
}

double Interpretor::ConjuctValues(const double l_value, const double r_value)
{
    QString binary_l_value = ToBinary(l_value);
    QString binary_r_value = ToBinary(r_value);

    QChar l_value_sign = binary_l_value[0];
    QChar r_value_sign = binary_r_value[0];

    binary_l_value.remove(0, 1);
    binary_r_value.remove(0, 1);

    QStringList l_parts = binary_l_value.split(".");
    QStringList r_parts = binary_r_value.split(".");

    QString integer_l_value = l_parts[0];
    QString decimal_l_value = l_parts[1];

    QString integer_r_value = r_parts[0];
    QString decimal_r_value = r_parts[1];

    int integer_dif = std::max(integer_l_value.length(), integer_r_value.length()) -
                           std::min(integer_l_value.length(), integer_r_value.length());

    for(int i = 0; i < integer_dif; ++i){
        if(integer_l_value.length() > integer_r_value.length())
            integer_r_value = '0' + integer_r_value;
        else
            integer_l_value = '0' + integer_l_value;
    }

    int decimal_dif = std::max(decimal_l_value.length(), decimal_r_value.length()) -
                           std::min(decimal_l_value.length(), decimal_r_value.length());

    for(int i = 0; i < decimal_dif; ++i){
        if(decimal_l_value.length() > decimal_r_value.length())
            decimal_r_value = decimal_r_value + '0';
        else
            decimal_l_value = decimal_l_value + '0';
    }

    integer_l_value = l_value_sign + integer_l_value;
    integer_r_value = r_value_sign + integer_r_value;

    QString integer_result;

    for(int i = 0; i < integer_l_value.length(); ++i){
        if(integer_l_value[i] == '1' && integer_r_value[i] == '1')
            integer_result += '1';
        else
            integer_result += '0';
    }

    QString decimal_result;

    for(int i = 0; i < decimal_l_value.length(); ++i){
        if(decimal_l_value[i] == '1' && decimal_r_value[i] == '1')
            decimal_result += '1';
        else
            decimal_result += '0';
    }

    return FromBinary(integer_result + "." + decimal_result);

}

double Interpretor::DisjunctValues(const double l_value, const double r_value)
{
    QString binary_l_value = ToBinary(l_value);
    QString binary_r_value = ToBinary(r_value);

    QChar l_value_sign = binary_l_value[0];
    QChar r_value_sign = binary_r_value[0];

    binary_l_value.remove(0, 1);
    binary_r_value.remove(0, 1);

    QStringList l_parts = binary_l_value.split(".");
    QStringList r_parts = binary_r_value.split(".");

    QString integer_l_value = l_parts[0];
    QString decimal_l_value = l_parts[1];

    QString integer_r_value = r_parts[0];
    QString decimal_r_value = r_parts[1];

    int integer_dif = std::max(integer_l_value.length(), integer_r_value.length()) -
                           std::min(integer_l_value.length(), integer_r_value.length());

    for(int i = 0; i < integer_dif; ++i){
        if(integer_l_value.length() > integer_r_value.length())
            integer_r_value = '0' + integer_r_value;
        else
            integer_l_value = '0' + integer_l_value;
    }

    int decimal_dif = std::max(decimal_l_value.length(), decimal_r_value.length()) -
                           std::min(decimal_l_value.length(), decimal_r_value.length());

    for(int i = 0; i < decimal_dif; ++i){
        if(decimal_l_value.length() > decimal_r_value.length())
            decimal_r_value = decimal_r_value + '0';
        else
            decimal_l_value = decimal_l_value + '0';
    }

    integer_l_value = l_value_sign + integer_l_value;
    integer_r_value = r_value_sign + integer_r_value;

    QString integer_result;

    for(int i = 0; i < integer_l_value.length(); ++i){
        if(integer_l_value[i] != '1' && integer_r_value[i] != '1')
            integer_result += '0';
        else
            integer_result += '1';
    }

    QString decimal_result;

    for(int i = 0; i < decimal_l_value.length(); ++i){
        if(integer_l_value[i] != '1' && integer_r_value[i] != '1')
            decimal_result += '0';
        else
            decimal_result += '1';
    }

    return FromBinary(integer_result + "." + decimal_result);
}

QString Interpretor::ToBinary(const double value)
{
    QString result;

    long long integer = (long long)value;
    double decimal = value - integer;

    if(integer == 0)
        result += '0';

    while(integer!=0) {result=(integer%2==0 ?"0":"1")+result; integer/=2;}

    result = (value >= 0) ? "0" + result: "1" + result; result += ".";
    (value < 0) ? decimal *= -1 : decimal *= 1;

    for(int i = 0; i < 20; ++i) {
        decimal = decimal * 2;
        int holder = (int)decimal;
        result += QString::number(holder);
        decimal -= holder;
    }

    auto end_idx_1 = result.lastIndexOf("1");
    auto end_idx_2 = result.lastIndexOf(".");

    if(end_idx_2 < end_idx_1)
        result.remove(end_idx_1 + 1, result.length() - end_idx_1);
    else
        result.remove(end_idx_2 + 1, result.length() - end_idx_2);

    return result;
}

double Interpretor::FromBinary(QString value)
{
    double result;

    QStringList parts = value.split(".");
    QString integer = parts[0];
    QString decimal = parts[1];

    int i_base = 1;
    long long integer_result = 0;

    for(int i = integer.length() - 1; i > 0; --i){
        switch(integer[i].unicode()){
            case '0' :
                i_base *= 2;
            break;
            case '1' :
                integer_result += i_base;
                i_base *= 2;
            break;
        }
    }

    double d_base = 0.5;
    double decimal_result = 0;

    for(int i = 0; i < decimal.length(); ++i){
        switch(decimal[i].unicode()){
            case '0' :
                d_base *= 0.5;
            break;
            case '1' :
                decimal_result += d_base;
                d_base *= 0.5;
            break;
        }
    }

    result = integer_result + decimal_result;

    (integer[0] == '0') ? result *= 1 : result *= -1;

    return result;

}

double Interpretor::ToOctal(const double value)
{
    QString str_result;

    long long integer = (long long)value;
    double decimal = value - integer;

    (value < 0) ? integer *= -1 : integer *= 1;
    (value < 0) ? decimal *= -1 : decimal *= 1;

    while(integer != 0) {
        str_result = QString::number(integer % 8) + str_result;
        integer /= 8;
    }

    if(decimal != 0)
    {
        str_result += ".";

        while(str_result.right(2).toInt() <= 10) {
            decimal = decimal * 8;
            int holder = (int)decimal;
            str_result += QString::number(holder);
            decimal -= holder;
        }
    }

    double result = str_result.toDouble();
    (value < 0) ? result *= -1 : result *= 1;

    return result;
}

double Interpretor::FromOctal(const double value)
{
    double result;
    QString octal_value = QString::number(value, 'f', 10);
    QChar value_sign;

    if(octal_value[0] == '-'){
        value_sign = '-';
        octal_value.remove(0, 1);
    }

    QStringList parts = octal_value.split(".");

    double decimal_result = 0;

    if(parts.size() == 2){
        QString decimal = parts[1];
        double d_base = 0.125;

        for(int i = 0; i < decimal.length(); ++i){
            decimal_result = decimal_result + d_base * QString(decimal[i]).toInt();
            d_base *= 0.125;
        }
    }

    QString integer = parts[0];
    int i_base = 1;
    long long integer_result = 0;

    for(int i = integer.length() - 1; i >= 0; --i){
        integer_result = integer_result + i_base * QString(integer[i]).toInt();
        i_base *= 8;
    }

    result = integer_result + decimal_result;

    (value_sign == '-') ? result *= -1 : result *= 1;

    return result;
}
