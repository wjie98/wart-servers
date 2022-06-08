#ifndef __CONV_HPP__
#define __CONV_HPP__

#include <vector>
#include <string>
#include <charconv>

#include <imports.h>
#include <types.hpp>
#include <fast_float.h>
#include <log_abort.hpp>

namespace imports {

bol txt2bol(std::string_view s) {
    i64 val = 0;
    auto [_, ec] = std::from_chars(s.begin(), s.end(), val);
    
    switch (ec)
    {
    case std::errc::invalid_argument:
        break;
    case std::errc::result_out_of_range:
        break;
    default:
        return val != 0;
    }

    if (s.starts_with("true") || s.starts_with("True")) {
        return true;
    }

    if (s.starts_with("false") || s.starts_with("False"))
    {
        return false;
    }

    LOG_ABORT("invalid argument")
}

i32 txt2i32(std::string_view s) {
    i32 val = 0;
    auto [_, ec] = std::from_chars(s.begin(), s.end(), val);
    
    switch (ec)
    {
    case std::errc::invalid_argument:
        LOG_ABORT("invalid argument")
    case std::errc::result_out_of_range:
        LOG_ABORT("result out of range")
    default:
        return val;
    }
}

i64 txt2i64(std::string_view s) {
    i64 val = 0;
    auto [_, ec] = std::from_chars(s.begin(), s.end(), val);
    
    switch (ec)
    {
    case std::errc::invalid_argument:
        LOG_ABORT("invalid argument")
    case std::errc::result_out_of_range:
        LOG_ABORT("result out of range")
    default:
        return val;
    }
}

f32 txt2f32(std::string_view s) {
    f32 val = 0;
    auto fmt = fast_float::chars_format::general;
    auto [_, ec] = fast_float::from_chars(s.begin(), s.end(), val, fmt);
    
    switch (ec)
    {
    case std::errc::invalid_argument:
        LOG_ABORT("invalid argument")
    case std::errc::result_out_of_range:
        LOG_ABORT("result out of range")
    default:
        return val;
    }
}

f64 txt2f64(std::string_view s) {
    f64 val = 0;
    auto fmt = fast_float::chars_format::general;
    auto [_, ec] = fast_float::from_chars(s.begin(), s.end(), val, fmt);
    
    switch (ec)
    {
    case std::errc::invalid_argument:
        LOG_ABORT("invalid argument")
    case std::errc::result_out_of_range:
        LOG_ABORT("result out of range")
    default:
        return val;
    }
}

std::string txt2txt(std::string_view s) {
    return {s.data(), s.length()};
}

template<typename T> inline T __from_txt(std::string_view s);
template<> inline bol __from_txt<bol>(std::string_view s) { return txt2bol(s); }
template<> inline i32 __from_txt<i32>(std::string_view s) { return txt2i32(s); }
template<> inline i64 __from_txt<i64>(std::string_view s) { return txt2i64(s); }
template<> inline f32 __from_txt<f32>(std::string_view s) { return txt2f32(s); }
template<> inline f64 __from_txt<f64>(std::string_view s) { return txt2f64(s); }
// template<> inline std::string __from_txt<std::string>(std::string_view s) { return txt2txt(s); }

template<typename T>
inline T __into_number(const imports_value_t& val) {
    switch (val.tag)
    {
    case IMPORTS_VALUE_BOL:
        return static_cast<T>(val.val.bol ? 1 : 0);
    case IMPORTS_VALUE_I32:
        return static_cast<T>(val.val.i32);
    case IMPORTS_VALUE_I64:
        return static_cast<T>(val.val.i64);
    case IMPORTS_VALUE_F32:
        return static_cast<T>(val.val.f32);
    case IMPORTS_VALUE_F64:
        return static_cast<T>(val.val.f64);
    case IMPORTS_VALUE_TXT:
        return __from_txt<T>({val.val.txt.ptr, val.val.txt.len});
    default:
        LOG_ABORT("invalid argument")
    }
}

inline bol __into_boolean(const imports_value_t& val) {
    switch (val.tag) {
        case IMPORTS_VALUE_BOL:
            return val.val.bol;
        case IMPORTS_VALUE_I32:
            return val.val.i32 != 0;
        case IMPORTS_VALUE_I64:
            return val.val.i64 != 0;
        case IMPORTS_VALUE_F32:
            return val.val.f32 != 0.0f && val.val.f32 != -0.0f;
        case IMPORTS_VALUE_F64:
            return val.val.f64 != 0.0 && val.val.f64 != -0.0;
        case IMPORTS_VALUE_TXT:
            return __from_txt<bol>({val.val.txt.ptr, val.val.txt.len});
        default:
            LOG_ABORT("invalid argument")
    }
}

inline std::string __into_string(const imports_value_t& val) {
    switch (val.tag) {
        case IMPORTS_VALUE_BOL:
            return val.val.bol ? "true" : "else";
        case IMPORTS_VALUE_I32:
            return std::to_string(val.val.i32);
        case IMPORTS_VALUE_I64:
            return std::to_string(val.val.i64);
        case IMPORTS_VALUE_F32:
            return std::to_string(val.val.f32);
        case IMPORTS_VALUE_F64:
            return std::to_string(val.val.f64);
        case IMPORTS_VALUE_TXT:
            return {val.val.txt.ptr, val.val.txt.len};
        default:
            LOG_ABORT("invalid argument")
    }
}

template<typename T> inline T to_type(const imports_value_t& val) { return __into_number<T>(val); }
template<> inline bol to_type(const imports_value_t& val) { return __into_boolean(val); }
template<> inline std::string to_type(const imports_value_t& val) { return __into_string(val); }


template<typename T>
inline std::vector<T> __into_number(const imports_vector_t& vec) {
    switch (vec.tag)
    {
    case IMPORTS_VECTOR_BOL: {
        auto& vs = vec.val.bol;
        std::vector<T> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = static_cast<T>(vs.ptr[i] ? 1 : 0);
        }
        return rs;
    }
    case IMPORTS_VECTOR_I32: {
        auto& vs = vec.val.i32;
        std::vector<T> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = static_cast<T>(vs.ptr[i]);
        }
        return rs;
    }
    case IMPORTS_VECTOR_I64: {
        auto& vs = vec.val.i64;
        std::vector<T> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = static_cast<T>(vs.ptr[i]);
        }
        return rs;
    }
    case IMPORTS_VECTOR_F32: {
        auto& vs = vec.val.f32;
        std::vector<T> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = static_cast<T>(vs.ptr[i]);
        }
        return rs;
    }
    case IMPORTS_VECTOR_F64: {
        auto& vs = vec.val.f64;
        std::vector<T> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = static_cast<T>(vs.ptr[i]);
        }
        return rs;
    }
    case IMPORTS_VECTOR_TXT: {
        auto& vs = vec.val.txt;
        std::vector<T> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = __from_txt<T>({vs.ptr[i].ptr, vs.ptr[i].len});
        }
        return rs;
    }
    default:
        LOG_ABORT("invalid argument")
    }
}

inline std::vector<bol> __into_boolean(const imports_vector_t& vec) {
    switch (vec.tag)
    {
    case IMPORTS_VECTOR_BOL: {
        auto& vs = vec.val.bol;
        std::vector<bol> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = vs.ptr[i];
        }
        return rs;
    }
    case IMPORTS_VECTOR_I32: {
        auto& vs = vec.val.i32;
        std::vector<bol> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = vs.ptr[i] != 0;
        }
        return rs;
    }
    case IMPORTS_VECTOR_I64: {
        auto& vs = vec.val.i64;
        std::vector<bol> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = vs.ptr[i] != 0;
        }
        return rs;
    }
    case IMPORTS_VECTOR_F32: {
        auto& vs = vec.val.f32;
        std::vector<bol> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = vs.ptr[i] != 0.0f && vs.ptr[i] != -0.0f;
        }
        return rs;
    }
    case IMPORTS_VECTOR_F64: {
        auto& vs = vec.val.f64;
        std::vector<bol> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = vs.ptr[i] != 0.0 && vs.ptr[i] != -0.0;
        }
        return rs;
    }
    case IMPORTS_VECTOR_TXT: {
        auto& vs = vec.val.txt;
        std::vector<bol> rs(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs[i] = __from_txt<bol>({vs.ptr[i].ptr, vs.ptr[i].len});
        }
        return rs;
    }
    default:
        LOG_ABORT("invalid argument")
    }
}

inline std::vector<std::string> __into_string(const imports_vector_t& vec) {
    switch (vec.tag)
    {
    case IMPORTS_VECTOR_BOL: {
        auto& vs = vec.val.bol;
        std::vector<std::string> rs; rs.reserve(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs.emplace_back(vs.ptr[i] ? "true" : "else");
        }
        return rs;
    }
    case IMPORTS_VECTOR_I32: {
        auto& vs = vec.val.i32;
        std::vector<std::string> rs; rs.reserve(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs.emplace_back(std::to_string(vs.ptr[i]));
        }
        return rs;
    }
    case IMPORTS_VECTOR_I64: {
        auto& vs = vec.val.i64;
        std::vector<std::string> rs; rs.reserve(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs.emplace_back(std::to_string(vs.ptr[i]));
        }
        return rs;
    }
    case IMPORTS_VECTOR_F32: {
        auto& vs = vec.val.f32;
        std::vector<std::string> rs; rs.reserve(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs.emplace_back(std::to_string(vs.ptr[i]));
        }
        return rs;
    }
    case IMPORTS_VECTOR_F64: {
        auto& vs = vec.val.f64;
        std::vector<std::string> rs; rs.reserve(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs.emplace_back(std::to_string(vs.ptr[i]));
        }
        return rs;
    }
    case IMPORTS_VECTOR_TXT: {
        auto& vs = vec.val.txt;
        std::vector<std::string> rs; rs.reserve(vs.len);
        for (size_t i = 0; i < vs.len; i++) {
            rs.emplace_back(vs.ptr[i].ptr, vs.ptr[i].len);
        }
        return rs;
    }
    default:
        LOG_ABORT("invalid argument")
    }
}

template<typename T> inline std::vector<T> to_type(const imports_vector_t& vec) { return __into_number<T>(vec); }
template<> inline std::vector<bol> to_type(const imports_vector_t& vec) { return __into_boolean(vec); }
template<> inline std::vector<std::string> to_type(const imports_vector_t& vec) { return __into_string(vec); }


}

#endif