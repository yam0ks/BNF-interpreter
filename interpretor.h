#ifndef INTERPRETOR_H
#define INTERPRETOR_H

#include "analyzer.h"
#include <QMap>
#include <stack>

class Interpretor
{
public:
    Interpretor();

    std::tuple<int, int, QString> InterpretCode(QString&& code);

    QString OutputValues();

    double ToOctal(const double value);
    double FromOctal(const double value);

    QString ToBinary(const double value);
    double FromBinary(QString value);

private:
    std::tuple<int, int, QString> EvaluateExpr(const std::vector<Token>& expression);

    int Opriority(const QString& op);
    QString Evaluate(std::stack<double>& values, const QString& op);

    double InverseValue(const double value);

    double ConjuctValues(const double l_value, const double r_value);
    double DisjunctValues(const double l_value, const double r_value);





    Analyzer an;
    QMap<QString, double> var_results;
};

#endif // INTERPRETOR_H
