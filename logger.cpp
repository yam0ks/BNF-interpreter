#include "logger.h"

std::tuple<int, int, QString> Logger::SendOk()
{
    return {0, 0, QString()};
}

std::tuple<int, int, QString> Logger::SendError(const int b_idx, const int e_idx, const QString &error)
{
    return {b_idx, e_idx, error};
}
