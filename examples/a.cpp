#include <sstream>
#include <cassert>
#include <imports.hpp>

template<typename T>
void ugly_concat_impl(std::stringstream& ss, const T& t) { ss << t; }

template<typename T, typename ... Args>
void ugly_concat_impl(std::stringstream& ss, const T& t, Args ... args) { ss << t; ugly_concat_impl(ss, args...); }

template<typename ... Args>
std::string ugly_concat(Args ... args) {
    std::stringstream ss;
    ugly_concat_impl(ss, args...);
    return ss.str();
}

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
    imports::item_param default_values[] = {{"node_id", "none"}, {"dummy", false}}; // 不提供dummy值就用默认的false
    auto table = imports::data_frame::open("choice_nodes", default_values);

    // 随机选择3个节点
    auto r = *store.choice_nodes("player", 3);  // 返回的是optional值，需要解引用

    // 获取节点内容
    auto n = r.view().size();   // r.view() 获得数据查看器，数据所有权依然和变量r绑定，支持移动语义
    for (size_t i = 0; i < n; i++) {
        imports::item_param values[] = {{"node_id", r.view().view_string(i)}};
        table->push(values);
    }
}

void test_query_nodes(imports::storage& store) {
    imports::item_param default_values[] = {{ "name", ""}, { "age", -1LL }}; // age 是int64_t类型
    auto table = imports::data_frame::open("query_nodes(player102)", default_values);

    std::string_view keys[] = {"name", "age"};  // 目前返回的table的headers会附带tag作为前缀
    auto r = *store.query_nodes("player102", "player", keys);

    imports::item_param data[] = {
        {"name", r.view()["player.name"]->as_txt()},   // 下标可以是整数或者字符串key
        {"age",  r.view()["player.age"]->as_i64()},
    };
    table->push(data);
}

void test_query_neighbors(imports::storage& store) {
    imports::item_param default_values[] = {{"target", "none"}, {"start_year", -1LL}};
    auto table = imports::data_frame::open("test_query_neighbors(player101)", default_values);

    std::string_view keys[] = {"start_year"};   // 实际用的时候这些统统可以定义成全局变量
    auto [dst, attr] = *store.query_neighbors("player101", "serve", keys);  // 这里用了c++17的结构化绑定

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

    // std::cout和std::cerr不再可用，使用imports里的日志接口。易用性有待优化
    // 临时可以用这个库代替格式化：https://github.com/MU001999/format
    log_error(ugly_concat("hello", ",", "world"));

    if (log_enabled(log_level::info)) {
        // 复杂的计算可以放在这个控制流里
        log(log_level::info, "log, yes!");  // 等价于log_info()
    }
}

int main() {

    auto store = *imports::storage::open();
    test_choice_nodes(store);
    test_query_nodes(store);
    test_query_neighbors(store);
    test_log();
    return 0;
}