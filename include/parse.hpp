#pragma once

#include <charconv>
#include <concepts>
#include <expected>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types.hpp"

namespace stdx::details {

constexpr std::string_view FMT_INT{"d"};
constexpr std::string_view FMT_UNSIGNED{"u"};
constexpr std::string_view FMT_FLOATING{"f"};
constexpr std::string_view FMT_STRING{"s"};

//
// Семейство функций parse_value, конвертирующих подстроку исходных данных в конкретный тип.
//
template <typename T>
    requires(std::integral<std::remove_cvref_t<T>> || std::floating_point<std::remove_cvref_t<T>>)
std::expected<T, scan_error> parse_value(std::string_view sv) {
    using RawT = std::remove_cvref_t<T>;

    RawT value{};

    auto [_, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec != std::errc{})
        return std::unexpected(scan_error{"parse failed"});

    return value;
}

//
// Функция для парсинга значения с учетом спецификатора формата
//
// Инкапсулирует всю логику преобразования подстроки исходных данных
// в конкретный тип на основе спецификатора конвертации.
//
// Возвращает ошибку scan_error в случае
// несоответствия переданного типа и спецификатора конвертации.
//
template <typename T>
std::expected<T, scan_error> parse_value_with_format(std::string_view input, std::string_view fmt) {
    using RawT = std::remove_cvref_t<T>;

    if constexpr (std::signed_integral<RawT>) {
        if (fmt != FMT_INT)
            return std::unexpected{scan_error{"Signed integral format mismatch"}};
        return parse_value<RawT>(input);

    } else if constexpr (std::unsigned_integral<RawT>) {
        if (fmt != FMT_UNSIGNED)
            return std::unexpected{scan_error{"Unsigned integral format mismatch"}};
        return parse_value<RawT>(input);

    } else if constexpr (std::floating_point<RawT>) {
        if (fmt != FMT_FLOATING)
            return std::unexpected{scan_error{"Floating point format mismatch"}};
        return parse_value<RawT>(input);

    } else if constexpr (std::same_as<RawT, std::string>) {
        if (fmt != FMT_STRING)
            return std::unexpected{scan_error{"String format mismatch"}};
        return std::string{input};

    } else if constexpr (std::same_as<RawT, std::string_view>) {
        if (fmt != FMT_STRING)
            return std::unexpected{scan_error{"String format mismatch"}};
        return std::string_view{input};

    } else {
        return std::unexpected{scan_error{"Format mismatch"}};
    }
}

//
// Функция для проверки корректности входных данных и выделения из обеих строк интересующих данных для парсинга
//
// Возвращает пару массивов подстрок форматирующей и исходной строки.
//
// Элементы массивов с одинаковыми индексами соответствуют
// плейсхолдеру в форматирующей строке и релевантной ему подстроке в строке с исходными данными.
//
template <typename... Ts>
std::expected<std::pair<std::vector<std::string_view>, std::vector<std::string_view>>, scan_error>
parse_sources(std::string_view input, std::string_view format) {
    std::vector<std::string_view> format_parts;  // Части формата между {}
    std::vector<std::string_view> input_parts;
    size_t start = 0;
    while (true) {
        size_t open = format.find('{', start);
        if (open == std::string_view::npos) {
            break;
        }
        size_t close = format.find('}', open);
        if (close == std::string_view::npos) {
            break;
        }

        // Если между предыдущей } и текущей { есть текст,
        // проверяем его наличие во входной строке
        if (open > start) {
            std::string_view between = format.substr(start, open - start);
            const auto pos = input.find(between);
            if (input.size() < between.size() || pos == std::string_view::npos) {
                return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
            }
            if (start != 0) {
                input_parts.emplace_back(input.substr(0, pos));
            }

            input = input.substr(pos + between.size());
        }

        // Сохраняем спецификатор формата (то, что между {})
        format_parts.push_back(format.substr(open + 1, close - open - 1));
        start = close + 1;
    }

    // Проверяем оставшийся текст после последней }
    if (start < format.size()) {
        const std::string_view remaining_format = format.substr(start);
        const auto pos = input.find(remaining_format);
        if (input.size() < remaining_format.size() || pos == std::string_view::npos) {
            return std::unexpected(scan_error{"Unformatted text in input and format string are different"});
        }
        input_parts.emplace_back(input.substr(0, pos));
        input = input.substr(pos + remaining_format.size());
    } else {
        input_parts.emplace_back(input);
    }
    return std::pair{format_parts, input_parts};
}

}  // namespace stdx::details