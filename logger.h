#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <tuple>

class Logger
{
public:
    Logger() = default;

    static std::tuple<int, int, QString> SendOk();

    static std::tuple<int, int, QString> SendError(const int b_idx, const int e_idx, const QString& error);

    template <typename Viterator>
    static std::tuple<int, int, QString> SendError(Viterator begin, Viterator end, const QString& error);
};

template <typename Viterator>
std::tuple<int, int, QString> Logger::SendError(Viterator begin, Viterator end, const QString& error){
    if(begin == end)
        return {(std::prev(begin))->get()->begin_idx, (std::prev(begin))->get()->end_idx, error};

    return {begin->get()->begin_idx, begin->get()->end_idx, error};
}

#endif // LOGGER_H
