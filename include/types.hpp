#pragma once

#include <string>
#include <tuple>

namespace stdx::details {

//
// Хранит строку ошибки сканирования.
//
struct scan_error {
    std::string message;
};

//
// Шаблонный класс для хранения результатов успешного сканирования
//
// Поле типа std::tuple - для готовых сканированных значений
// Метод values для удобного доступа к сканированным значениям.
//
template <typename... Ts>
struct scan_result {

    std::tuple<Ts...> data;

    auto &values() { return data; }
    const auto &values() const { return data; }
};

}  // namespace stdx::details
