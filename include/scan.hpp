#pragma once

#include "parse.hpp"
#include "types.hpp"
#include <expected>
#include <optional>
#include <tuple>
#include <utility>

namespace stdx::details {

template <typename... Ts, size_t... Is>
std::expected<std::tuple<Ts...>, scan_error> parse_tuple(const std::vector<std::string_view> &values,
                                                         const std::vector<std::string_view> &formats,
                                                         std::index_sequence<Is...>) {

    if (values.size() != sizeof...(Ts) || formats.size() != sizeof...(Ts))
        return std::unexpected(scan_error{"Unexpected parsed data length"});

    // Parse all, get all errors
    auto vals_and_errs =
        std::tuple<std::expected<Ts, scan_error>...>{parse_value_with_format<Ts>(values[Is], formats[Is])...};

    // Get the first error to pass it further:

    std::optional<scan_error> first_err;
    auto capture_first_error = [&](auto &val_exp) {
        if (!first_err && !val_exp)
            first_err = val_exp.error();
    };

    (capture_first_error(std::get<Is>(vals_and_errs)), ...);

    if (first_err)
        return std::unexpected{*first_err};

    return std::tuple<Ts...>{(*std::get<Is>(vals_and_errs))...};
}

}  // namespace stdx::details

namespace stdx {

//
// Главная шаблонная функция
//
// Функция в качестве шаблонных параметров принимает набор типов,
// в которые нужно конвертировать исходные данные.
//
// В качестве параметров функция принимает форматирующую строку и строку с исходными данными.
// Функция использует функцию parse_sources,
// чтобы получить разбиение строк,
// агрегировать результаты работы parse_value_with_format в объект типа scan_result
// и возвращать его наружу.
//
// Выполняет:
// - чтение данных из исходной строки,
// - интерпретацию данных на основе форматирующей строки
// - сохранение результатов интерпретации в объекты указанных типов во время выполнения.
//
template <typename... Ts>
std::expected<details::scan_result<Ts...>, details::scan_error> scan(std::string_view input, std::string_view format) {

    using namespace details;

    const auto parsed = parse_sources<Ts...>(input, format);
    if (!parsed)
        return std::unexpected{parsed.error()};

    const auto &[formats, values] = parsed.value();

    const auto result = parse_tuple<Ts...>(values, formats, std::index_sequence_for<Ts...>{});
    if (!result)
        return std::unexpected{result.error()};

    return scan_result<Ts...>{*result};
}

}  // namespace stdx
