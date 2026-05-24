#pragma once

#include <string>
#include <tuple>
namespace stdx::details {

// Класс для хранения ошибки неуспешного сканирования

struct scan_error {
    std::string message;
};

// Шаблонный класс для хранения результатов успешного сканирования

template <typename... Ts>
struct scan_result {
    // здесь ваш код
    std::tuple<Ts...> values() const { return values_; }

private:
    std::tuple<Ts...> values_;
};

}  // namespace stdx::details
