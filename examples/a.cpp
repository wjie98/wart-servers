#include <iostream>
#include <ostream>
#include <sstream>
#include <format>
#include <imports.hpp>

using imports::log_info;
using imports::log_error;

template<typename T>
std::string to_string(const T& t) {
    std::stringstream ss; ss << t;
    return ss.str();
}

imports::item_param table_defaults[] = {
    imports::item_param { "bool", false },
    imports::item_param { "int32", 1 },
    imports::item_param { "int64", 2LL },
    imports::item_param { "float32", 3.0f },
    imports::item_param { "float64", 4.0 },
    imports::item_param { "string", "5.0" },
};

auto store = imports::storage::open();
auto table = imports::data_frame::open("node", table_defaults);

int main() {
    auto r = store->choice_nodes("player", 1);
    switch (r->view().type())
    {
    case imports::vector_view::i64:
        log_info(to_string(r->view().as_i64().front()));
        break;
    case imports::vector_view::txt:
        break;
    default:
        log_error("invalid type on node_id");
        abort();
    }

    imports::item_param data[] = {
        imports::item_param { "bool", true },
        imports::item_param { "string", "hello" },
    };
    table->push(data);
    table->push(data);
    table->push(data);

    return 0;
}