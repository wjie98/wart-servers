#ifndef __OPTION_HPP__
#define __OPTION_HPP__

#include <string>
#include <optional>

#include <log_abort.hpp>

namespace imports {

template<typename T>
class option {
    std::optional<T> _data;

    template<typename ... Args>
    constexpr explicit option<T>(Args&&... args): _data(std::forward<Args>(args)...) {}

    std::optional<T> take() {
        std::optional<T> tmp = std::nullopt;
        this->_data.swap(tmp);
        return tmp;
    }

public:
    static constexpr option<T> some(T&& o) {
        return option<T>(std::in_place, std::forward<T>(o));
    }

    static constexpr option<T> none() {
        return option<T>(std::nullopt);
    }

    bool is_some() { return this->_data.has_value(); }
    bool is_none() { return !this->_data.has_value(); }

    T expect(std::string_view msg) {
        if (this->is_none()) {
            LOG_ABORT(msg)
        }
        return *this->take();
    }
    
    T unwrap() { return this->expect("unwrapping none"); }
};

}

#endif