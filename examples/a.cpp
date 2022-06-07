#include <sstream>
#include <imports.hpp>

// 支持的数据类型
imports::item_param __support_item_types[] = {
    imports::item_param { "bool", false },
    imports::item_param { "int32", 1 },
    imports::item_param { "int64", 2LL },
    imports::item_param { "float32", 3.0f },
    imports::item_param { "float64", 4.0 },
    imports::item_param { "string", "5.0" },
};

void test_choice_nodes(imports::storage& store) {
    // 定义一张表，采样结束后返回到客户端
    // default_values用于设定表里包含的列，列的类型和默认值
    auto table = imports::data_frame::open("choice_nodes", {
        {"node_id", "none"},
        {"dummy", false},
    });

    // 随机选择3个节点
    auto r = *store.choice_nodes("player", 3);  // 返回的是optional值，需要解引用

    // 获取节点内容
    auto n = r.view().size();   // r.view() 获得数据查看器，数据所有权依然和变量r绑定，支持移动语义
    for (size_t i = 0; i < n; i++) {
        table->push({
            {"node_id", r.view().view_string(i)},
        });
    }
}

void test_query_nodes(imports::storage& store) {
    // age 是int64_t类型
    auto table = imports::data_frame::open("query_nodes(player102)", {
        { "name", ""},
        { "age", -1LL },
    });

    std::string_view keys[] = {"name", "age"};  // 目前返回的table的headers会附带tag作为前缀
    auto r = *store.query_nodes("player102", "player", keys);

    auto name = r.view()["player.name"]->as_txt();
    auto age = r.view()["player.age"]->as_i64();
    table->push({
        {"name", name},
        {"age", age},
    });
}

void test_query_neighbors(imports::storage& store) {
    // 可以用一个span或者vector存储列表参数
    imports::item_param default_values[] = {
        {"target", "none"},
        {"start_year", -1LL},
    };
    auto table = imports::data_frame::open("test_query_neighbors(player101)", default_values);

    auto [dst, attr] = *store.query_neighbors("player101", "serve", {"start_year"});  // 这里用了c++17的结构化绑定

    auto n = dst.view().size();
    for (size_t i = 0; i < n; i++) {
        imports::item_param data[] = {
            {"target", dst.view().as_txt()[i]},
            {"start_year", attr.view()["serve.start_year"]->as_i64()[i]},
        };
        table->push(data);
    }
}

void test_log() {
    using imports::log;
    using imports::log_info;
    using imports::log_error;
    using imports::log_enabled;
    using imports::log_level;

    // std::cout和std::cerr不再可用，使用imports里的日志接口。
    // pattern的使用方法参考：https://github.com/MU001999/format，基于C++20的constexpr实现编译期检查
    log_error<"{},{}">("hello", "world");

    if (log_enabled(log_level::info)) {
        // 复杂的计算可以放在这个控制流里
        log<"{},{}!">(log_level::info, "log", "yes");  // 等价于log_info()
    }

    // 格式化函数可以单独使用
    imports::format<"{}">("ABC"); // fmt::format()
}

int main() {

    auto store = *imports::storage::open();
    test_choice_nodes(store);
    test_query_nodes(store);
    test_query_neighbors(store);
    test_log();
    return 0;
}