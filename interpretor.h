#ifndef INTERPRETOR_H
#define INTERPRETOR_H
#pragma once

#include "analyzer.h"
#include <QMap>
#include <stack>

class Interpretor
{
public:
    Interpretor() = default;

    std::tuple<int, int, QString> InterpretCode(QString&& code);
    QString OutputValues() const;

    static double ToOctal(const double value);
    static double FromOctal(const double value);
    static QString ToBinary(const double value);
    static double FromBinary(QString value);

private:
    std::tuple<int, int, QString> EvaluateExpr(const std::vector<Token>& expression);

    int Opriority(const QString& op) const;
    QString Evaluate(std::stack<double>& values, const QString& op);

    double InverseValue(const double value);

    double ConjuctValues(const double l_value, const double r_value);
    double DisjunctValues(const double l_value, const double r_value);

    int ComputeSignificant(const double value) const;

    Analyzer an;
    QMap<QString, double> var_results;
};

#endif // INTERPRETOR_H
