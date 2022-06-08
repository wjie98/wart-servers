#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <span>
#include <string>
#include <memory>
#include <variant>
#include <sstream>
#include <charconv>
#include <numeric>

#include <imports.h>
#include <log_abort.hpp>
#include <types.hpp>

namespace imports {

using value_param = std::variant<bol, i32, i64, f32, f64, std::string_view>;
using vector_param = std::variant<
    std::span<bol>,
    std::span<i32>, std::span<i64>,
    std::span<f32>, std::span<f64>,
    std::span<std::string_view>, std::span<std::string>>;
struct item_param {
    std::string_view key;
    value_param val;
};

struct series_param {
    std::string_view key;
    vector_param val;
};

void __set_string_param(imports_string_t& ret0, const std::string_view param) {
    ret0.ptr = const_cast<char*>(param.data());
    ret0.len = param.length();
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_string_param(imports_list_string_t& ret0, const std::span<std::string_view> param) {
    void* ptr = malloc(sizeof(imports_string_t) * param.size());
    
    ret0.ptr = reinterpret_cast<imports_string_t*>(ptr);
    ret0.len = param.size();

    for (size_t i = 0; i < ret0.len; i++) {
        __set_string_param(ret0.ptr[i], param[i]);
    }
    return {ptr, free};
}

// [[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_string_param(imports_list_string_t& ret0, const std::span<std::string> param) {
//     void* ptr = malloc(sizeof(imports_string_t) * param.size());
    
//     ret0.ptr = reinterpret_cast<imports_string_t*>(ptr);
//     ret0.len = param.size();

//     for (size_t i = 0; i < ret0.len; i++) {
//         __set_string_param(ret0.ptr[i], param[i]);
//     }
//     return {ptr, free};
// }

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_string_param(imports_list_string_t& ret0, const std::initializer_list<std::string_view> param) {
    void* ptr = malloc(sizeof(imports_string_t) * param.size());
    
    ret0.ptr = reinterpret_cast<imports_string_t*>(ptr);
    ret0.len = param.size();

    imports_string_t* p = ret0.ptr;
    for (auto it = param.begin(); it != param.end(); ++it) {
        __set_string_param(*p, *it); p++;
    }
    return {ptr, free};
}

void __set_value_param(imports_value_t& ret0, const value_param param) {
    if (auto it = std::get_if<bol>(&param); it) {
        ret0.tag = IMPORTS_VALUE_BOL;
        ret0.val.bol = *it;
    } else if (auto it = std::get_if<i32>(&param); it) {
        ret0.tag = IMPORTS_VALUE_I32;
        ret0.val.i32 = *it;
    } else if (auto it = std::get_if<i64>(&param); it) {
        ret0.tag = IMPORTS_VALUE_I64;
        ret0.val.i64 = *it;
    } else if (auto it = std::get_if<f32>(&param); it) {
        ret0.tag = IMPORTS_VALUE_F32;
        ret0.val.f32 = *it;
    } else if (auto it = std::get_if<f64>(&param); it) {
        ret0.tag = IMPORTS_VALUE_F64;
        ret0.val.f64 = *it;
    } else if (auto it = std::get_if<std::string_view>(&param); it) {
        ret0.tag = IMPORTS_VALUE_TXT;
        __set_string_param(ret0.val.txt, *it);
    } else {
        LOG_ABORT("invalid value_param")
    }
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_vector_param(imports_vector_t& ret0, const vector_param param) {
    void* ptr = NULL;
    if (auto it = std::get_if<std::span<bol>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_BOL;
        ret0.val.bol.ptr = const_cast<bol*>(it->data());
        ret0.val.bol.len = it->size();
    } else if (auto it = std::get_if<std::span<i32>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_I32;
        ret0.val.i32.ptr = const_cast<i32*>(it->data());
        ret0.val.i32.len = it->size();
    } else if (auto it = std::get_if<std::span<i64>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_I64;
        ret0.val.i64.ptr = const_cast<i64*>(it->data());
        ret0.val.i64.len = it->size();
    } else if (auto it = std::get_if<std::span<f32>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_F32;
        ret0.val.f32.ptr = const_cast<f32*>(it->data());
        ret0.val.f32.len = it->size();
    } else if (auto it = std::get_if<std::span<f64>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_F64;
        ret0.val.f64.ptr = const_cast<f64*>(it->data());
        ret0.val.f64.len = it->size();
    } else if (auto it = std::get_if<std::span<std::string_view>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_TXT;

        auto& txt = ret0.val.txt;
        ptr = malloc(sizeof(imports_string_t) * it->size());

        txt.ptr = reinterpret_cast<imports_string_t*>(ptr);
        txt.len = it->size();

        for (size_t i = 0; i < txt.len; i++) {
            __set_string_param(txt.ptr[i], it->data()[i]);
        }

    } else if (auto it = std::get_if<std::span<std::string>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_TXT;

        auto& txt = ret0.val.txt;
        ptr = malloc(sizeof(imports_string_t) * it->size());

        txt.ptr = reinterpret_cast<imports_string_t*>(ptr);
        txt.len = it->size();
        
        for (size_t i = 0; i < txt.len; i++) {
            __set_string_param(txt.ptr[i], it->data()[i]);
        }
    } else {
        LOG_ABORT("invalid vector_param")
    }
    return {ptr, free};
}

void __set_item_param(imports_item_t& ret0, const item_param param) {
    __set_string_param(ret0.key, param.key);
    __set_value_param(ret0.val, param.val);
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_item_param(imports_row_t& ret0, std::span<item_param> param) {
    void* ptr = malloc(sizeof(imports_item_t) * param.size());

    ret0.ptr = reinterpret_cast<imports_item_t*>(ptr);
    ret0.len = param.size();

    for (size_t i = 0; i < ret0.len; i++) {
        __set_item_param(ret0.ptr[i], param[i]);
    }

    return {ptr, free};
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_item_param(imports_row_t& ret0, std::initializer_list<item_param> param) {
    void* ptr = malloc(sizeof(imports_item_t) * param.size());

    ret0.ptr = reinterpret_cast<imports_item_t*>(ptr);
    ret0.len = param.size();

    imports_item_t* p = ret0.ptr;
    for (auto it = param.begin(); it != param.end(); ++it) {
        __set_item_param(*p, *it); p ++;
    }
    return {ptr, free};
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_series_param(imports_series_t& ret0, const series_param param) {
    __set_string_param(ret0.key, param.key);
    return __set_vector_param(ret0.val, param.val);
}

inline bool __lt_imports_item(const imports_item_t& x, const imports_item_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

inline bool __eq_imports_item(const imports_item_t& x, const imports_item_t& y) {
    return std::string_view(x.key.ptr, x.key.len) == std::string_view(y.key.ptr, y.key.len);
}

inline bool __lt_imports_series(const imports_series_t& x, const imports_series_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

inline bool __eq_imports_series(const imports_series_t& x, const imports_series_t& y) {
    return std::string_view(x.key.ptr, x.key.len) == std::string_view(y.key.ptr, y.key.len);
}

}

#endif