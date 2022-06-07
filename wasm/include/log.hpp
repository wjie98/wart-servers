#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <imports.h>
#include <format.hpp>
#include <log_abort.hpp>

namespace imports {

using fmt::format;

enum log_level {
    trace = IMPORTS_LOG_LEVEL_TRACE,
    debug = IMPORTS_LOG_LEVEL_DEBUG,
    info  = IMPORTS_LOG_LEVEL_INFO,
    warn  = IMPORTS_LOG_LEVEL_WARN,
    error = IMPORTS_LOG_LEVEL_ERROR,
};

inline void log(log_level lv, const std::string_view msg) {
    imports_string_t s = {
        .ptr = const_cast<char*>(msg.data()),
        .len = msg.length(),
    };
    return imports_log(lv, &s);
}

inline void log_trace(const std::string_view msg) { return log(log_level::trace, msg); }
inline void log_debug(const std::string_view msg) { return log(log_level::debug, msg); }
inline void log_info (const std::string_view msg) { return log(log_level::info,  msg); }
inline void log_warn (const std::string_view msg) { return log(log_level::warn,  msg); }
inline void log_error(const std::string_view msg) { return log(log_level::error, msg); }

template<fmt::details::FixedString pattern, typename ... Args>
inline void log(log_level lv, Args&& ... args) { return log(lv, fmt::format<pattern>(std::forward<Args>(args) ...)); }
template<fmt::details::FixedString pattern, typename ... Args>
inline void log_trace(Args&& ... args) { return log_trace(fmt::format<pattern>(std::forward<Args>(args) ...)); }
template<fmt::details::FixedString pattern, typename ... Args>
inline void log_debug(Args&& ... args) { return log_debug(fmt::format<pattern>(std::forward<Args>(args) ...)); }
template<fmt::details::FixedString pattern, typename ... Args>
inline void log_info (Args&& ... args) { return log_info (fmt::format<pattern>(std::forward<Args>(args) ...)); }
template<fmt::details::FixedString pattern, typename ... Args>
inline void log_warn (Args&& ... args) { return log_warn (fmt::format<pattern>(std::forward<Args>(args) ...)); }
template<fmt::details::FixedString pattern, typename ... Args>
inline void log_error(Args&& ... args) { return log_error(fmt::format<pattern>(std::forward<Args>(args) ...)); }

bool log_enabled(log_level lv) {
    return imports_log_enabled(lv);
}

}

#endif