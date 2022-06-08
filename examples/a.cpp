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
        {"node_id", -1LL},
        {"dummy", false},
    }).unwrap();

    // 随机选择3个节点
    auto r = store.choice_nodes("author", 3).expect("choice_nodes() returned 'None'");

    // 获取节点内容
    auto vs = r.view().as_i64(); // 这里做了类型转换
    for (auto it = vs.begin(); it != vs.end(); ++it) {
        table.push({
            {"node_id", *it},
        });
    }
}

void test_query_nodes(imports::storage& store) {
    // age 是int64_t类型
    auto table = imports::data_frame::open("query_nodes(1005)", {
        { "author_id", -1},
        { "label", -1},
    }).unwrap();

    auto r = store.query_nodes(1005, "author", {"author_id", "label"}).unwrap();

    assert(r.view()["author.author_id"].unwrap().is_i64()); // 实际类型都为i64
    assert(r.view()["author.label"].expect("no author.label").is_i64()); // 使用expect解包以获得更明确的错误信息

    auto author_id = r.view()["author.author_id"].unwrap().to_i32(); // 强制转成i32
    auto label = r.view()["author.label"].unwrap().to_i32();
    table.push({
        {"author_id", author_id},
        {"label", label},
    });
}

void test_query_neighbors(imports::storage& store) {
    auto table = imports::data_frame::open("test_query_neighbors(1005)", {
        {"target", -1LL},
        {"time_stamp", -1LL},
    }).unwrap();

    auto [dst, attr] = store.query_neighbors(1005, "coauthor", {"time_stamp"}).unwrap();

    assert(attr.view()["coauthor.time_stamp"].unwrap().is_i64()); // 实际类型为i64

    auto n = dst.view().size();
    for (size_t i = 0; i < n; i++) {
        imports::item_param data[] = {
            {"target", dst.view().as_i64()[i]},
            {"time_stamp", attr.view()["coauthor.time_stamp"].unwrap().as_i64()[i]}, // 这里不发生强制类型转换
        };
        table.push(data);
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

int main(int argc, char* argv[]) {

    if (argc != 0) {
        // 获得整数类型的参数
        auto a = imports::txt2i32(argv[0]);
        imports::log_info<"args: {}">(a);

        // 如果需要在模板函数里使用，可以这样写
        auto b = imports::__from_txt<imports::i32>(argv[0]);
        imports::log_info<"args: {}">(b);
    }

    auto store = imports::storage::open().unwrap();
    // test_choice_nodes(store);
    test_query_nodes(store);
    test_query_neighbors(store);
    test_log();
    return 0;
}