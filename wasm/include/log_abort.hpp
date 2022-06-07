#ifndef __LOG_ABORT_HPP__
#define __LOG_ABORT_HPP__

#include <string>
#include <imports.h>

namespace imports {

[[noreturn]] void log_abort(std::string_view msg) {
    imports_string_t s = {
        .ptr = const_cast<char*>(msg.data()),
        .len = msg.length(),
    };
    imports_log(IMPORTS_LOG_LEVEL_ERROR, &s);
    abort();
}

}

#endif