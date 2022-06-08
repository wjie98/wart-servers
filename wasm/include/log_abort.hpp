#ifndef __LOG_ABORT_HPP__
#define __LOG_ABORT_HPP__

#include <string>
#include <sstream>
#include <imports.h>

namespace imports {

#define LOG_ABORT(x) imports::__detail_abort(__FILE__, __func__, __LINE__, (x));

[[noreturn]] void __log_abort(std::string_view msg) {
    imports_string_t s = {
        .ptr = const_cast<char*>(msg.data()),
        .len = msg.length(),
    };
    imports_log(IMPORTS_LOG_LEVEL_ERROR, &s);
    abort();
}

[[noreturn]] void __detail_abort(const char * file, const char* func, int line, std::string_view msg) {
    std::stringstream ss;
    ss << file << ": " << func << ": " << line << ": " << msg;
    __log_abort(ss.str());
}

}

#endif