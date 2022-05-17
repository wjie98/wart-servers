#include <imports.h>

#define TCB_SPAN_NAMESPACE_NAME std
#include <tcb/span.hpp>

#include <string>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

class value_view {
    imports_value_t _data;
public:
    value_view(const imports_value_t* data) { this->_data = *data; }
    bool is_nil() const { return this->_data.tag == IMPORTS_VALUE_NIL; }
    bool is_bol() const { return this->_data.tag == IMPORTS_VALUE_BOL; }
    bool is_i32() const { return this->_data.tag == IMPORTS_VALUE_I32; }
    bool is_i64() const { return this->_data.tag == IMPORTS_VALUE_I64; }
    bool is_f32() const { return this->_data.tag == IMPORTS_VALUE_F32; }
    bool is_f64() const { return this->_data.tag == IMPORTS_VALUE_F64; }
    bool is_txt() const { return this->_data.tag == IMPORTS_VALUE_TXT; }

    bool as_bol() const { return this->_data.val.bol; }
    int32_t as_i32() const { return this->_data.val.i32; }
    int64_t as_i64() const { return this->_data.val.i64; }
    float as_f32() const { return this->_data.val.f32; }
    double as_f64() const { return this->_data.val.f64; }
    std::string_view as_txt() const {
        auto& txt = this->_data.val.txt;
        return {txt.ptr, txt.len};
    }
};

class bol_series_view {
    imports_list_bool_t _data;
public:
    bol_series_view(const imports_list_bool_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    bool operator[] (size_t i) const { return this->_data.ptr[i]; }
};

class i32_series_view {
    imports_list_s32_t _data;
public:
    i32_series_view(const imports_list_s32_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    int32_t operator[] (size_t i) const { return this->_data.ptr[i]; }
};

class i64_series_view {
    imports_list_s64_t _data;
public:
    i64_series_view(const imports_list_s64_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    int64_t operator[] (size_t i) const { return this->_data.ptr[i]; }
};

class f32_series_view {
    imports_list_float32_t _data;
public:
    f32_series_view(const imports_list_float32_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    float operator[] (size_t i) const { return this->_data.ptr[i]; }
};

class f64_series_view {
    imports_list_float64_t _data;
public:
    f64_series_view(const imports_list_float64_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    double operator[] (size_t i) const { return this->_data.ptr[i]; }
};

class txt_series_view {
    imports_list_string_t _data;
public:
    txt_series_view(const imports_list_string_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    std::string_view operator[] (size_t i) const {
        auto& txt = this->_data.ptr[i];
        return {txt.ptr, txt.len};
    }
};

class series_view {
    imports_series_t _data;
public:
    series_view(const imports_series_t* data) { this->_data = *data; }
    bool is_nil() const { return this->_data.tag == IMPORTS_VALUE_NIL; }
    bool is_bol() const { return this->_data.tag == IMPORTS_VALUE_BOL; }
    bool is_i32() const { return this->_data.tag == IMPORTS_VALUE_I32; }
    bool is_i64() const { return this->_data.tag == IMPORTS_VALUE_I64; }
    bool is_f32() const { return this->_data.tag == IMPORTS_VALUE_F32; }
    bool is_f64() const { return this->_data.tag == IMPORTS_VALUE_F64; }
    bool is_txt() const { return this->_data.tag == IMPORTS_VALUE_TXT; }

    bol_series_view as_bol() const { return bol_series_view(&this->_data.val.bol); }
    i32_series_view as_i32() const { return i32_series_view(&this->_data.val.i32); }
    i64_series_view as_i64() const { return i64_series_view(&this->_data.val.i64); }
    f32_series_view as_f32() const { return f32_series_view(&this->_data.val.f32); }
    f64_series_view as_f64() const { return f64_series_view(&this->_data.val.f64); }
    txt_series_view as_txt() const { return txt_series_view(&this->_data.val.txt); }
};

class row {
    imports_row_t _data;
public:
    row(imports_row_t* data) { this->_data = *data; *data = {NULL, 0}; }
    row(row&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; }
    ~row() { imports_row_free(&this->_data); }
    row& operator= (row&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; return *this; }
    size_t size() const { return this->_data.len; }
    value_view operator[] (size_t i) const { return value_view(&this->_data.ptr[i]); }
};

class table {
    imports_table_t _data;
public:
    table(imports_table_t* data) { this->_data = *data; *data = {NULL, 0}; }
    table(table&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; }
    ~table() { imports_table_free(&this->_data); }
    table& operator= (table&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; return *this; }
    size_t size() const { return this->_data.len; }
    series_view operator[] (size_t i) const { return series_view(&this->_data.ptr[i]); }
};

class future_row {
    imports_future_row_t _fut;
    bool _owner;
public:
    future_row(imports_future_row_t* fut) { this->_fut = *fut; this->_owner = true; }
    future_row(future_row&& rhs) { this->_fut = rhs._fut; this->_owner = rhs._owner; rhs._owner = false; }
    ~future_row() { if (this->_owner) imports_future_row_free(&this->_fut); }
    future_row& operator= (future_row&& rhs) { this->_fut = rhs._fut; this->_owner = rhs._owner; rhs._owner = false; return *this; }
    std::variant<imports_future_error_t, row> get() const {
        imports_row_t row0;
        imports_future_error_t err = imports_get_future_row(this->_fut, &row0);
        if (err != (uint8_t)(-1)) return err;
        else return row(&row0);
    }
};

class future_table {
    imports_future_table_t _fut;
    bool _owner;
public:
    future_table(imports_future_table_t* fut) { this->_fut = *fut; this->_owner = true; }
    future_table(future_table&& rhs) { this->_fut = rhs._fut; this->_owner = rhs._owner; rhs._owner = false; }
    ~future_table() { if (this->_owner) imports_future_table_free(&this->_fut); }
    future_table& operator= (future_table&& rhs) { this->_fut = rhs._fut; this->_owner = rhs._owner; rhs._owner = false; return *this; }
    std::variant<imports_future_error_t, table> get() const {
        imports_table_t table0;
        imports_future_error_t err = imports_get_future_table(this->_fut, &table0);
        if (err != (uint8_t)(-1)) return err;
        else return table(&table0);
    }
};

std::vector<imports_string_t> __dump_imports_list_string(std::span<std::string_view> keys) {
    std::vector<imports_string_t> ks(keys.size());
    for (size_t i = 0; i < ks.size(); i++) {
        ks[i].ptr = const_cast<char*>(keys[i].data());
        ks[i].len = keys[i].size();
    }
    return ks;
}

std::variant<imports_future_error_t, future_row> async_query_kv(std::span<std::string_view> keys) {
    auto ks = __dump_imports_list_string(keys);
    imports_list_string_t keys0 = {.ptr = ks.data(), .len = ks.size()};

    imports_future_row_t ret0;
    imports_future_error_t err = imports_async_query_kv(&keys0, &ret0);
    if (err != (uint8_t)(-1)) return err;
    else return future_row(&ret0);
}

std::variant<imports_future_error_t, future_row> __async_query_node_impl(imports_node_id_t& id0, std::string_view tag, std::span<std::string_view> keys) {
    imports_string_t tag0 = {.ptr = const_cast<char*>(tag.data()), .len = tag.size()};
    
    auto ks = __dump_imports_list_string(keys);
    imports_list_string_t keys0 = {.ptr = ks.data(), .len = ks.size()};

    imports_future_row_t ret0;
    imports_future_error_t err = imports_async_query_node(&id0, &tag0, &keys0, &ret0);
    if (err != (uint8_t)(-1)) return err;
    else return future_row(&ret0);
}

std::variant<imports_future_error_t, future_row> async_query_node(std::string_view id, std::string_view tag, std::span<std::string_view> keys) {
    imports_node_id_t id0;
    id0.tag = IMPORTS_NODE_ID_TXT;
    id0.val.txt.ptr = const_cast<char*>(id.data());
    id0.val.txt.len = id.size();
    return __async_query_node_impl(id0, tag, keys);
}

std::variant<imports_future_error_t, future_row> async_query_node(int64_t id, std::string_view tag, std::span<std::string_view> keys) {
    imports_node_id_t id0;
    id0.tag = IMPORTS_NODE_ID_I64;
    id0.val.i64 = id;
    return __async_query_node_impl(id0, tag, keys);
}

std::variant<imports_future_error_t, future_table> async_choice_nodes(std::string_view tag, int32_t n) {
    imports_string_t tag0 = {.ptr = const_cast<char*>(tag.data()), .len = tag.size()};
    imports_future_table_t ret0;
    imports_future_error_t err = imports_async_choice_nodes(&tag0, n, &ret0);
    if (err != (uint8_t)(-1)) return err;
    else return future_table(&ret0);
}

std::variant<imports_future_error_t, future_table> __async_query_neighbors_impl(imports_node_id_t& id0, std::string_view tag, std::span<std::string_view> keys, bool reversely) {
    imports_string_t tag0 = {.ptr = const_cast<char*>(tag.data()), .len = tag.size()};

    auto ks = __dump_imports_list_string(keys);
    imports_list_string_t keys0 = {.ptr = ks.data(), .len = ks.size()};

    imports_future_table_t ret0;
    imports_future_error_t err = imports_async_query_neighbors(&id0, &tag0, &keys0, reversely, &ret0);
    if (err != (uint8_t)(-1)) return err;
    else return future_table(&ret0);
}

std::variant<imports_future_error_t, future_table> async_query_neighbors(std::string_view id, std::string_view tag, std::vector<std::string_view>& keys, bool reversely) {
    imports_node_id_t id0;
    id0.tag = IMPORTS_NODE_ID_TXT;
    id0.val.txt.ptr = const_cast<char*>(id.data());
    id0.val.txt.len = id.size();
    return __async_query_neighbors_impl(id0, tag, keys, reversely);
}

std::variant<imports_future_error_t, future_table> async_query_neighbors(int64_t id, std::string_view tag, std::vector<std::string_view>& keys, bool reversely) {
    imports_node_id_t id0;
    id0.tag = IMPORTS_NODE_ID_I64;
    id0.val.i64 = id;
    return __async_query_neighbors_impl(id0, tag, keys, reversely);
}

int64_t select_nodes(std::span<int64_t> id) {
    imports_series_id_t id0;
    id0.tag = IMPORTS_SERIES_ID_I64;
    id0.val.i64 = {.ptr = const_cast<int64_t*>(id.data()), .len = id.size()};
    return imports_select_nodes(&id0);
}

int64_t select_nodes(std::span<std::string_view> id) {
    auto ss = __dump_imports_list_string(id);

    imports_series_id_t id0;
    id0.tag = IMPORTS_SERIES_ID_TXT;
    id0.val.txt = {.ptr = ss.data(), .len = ss.size()};

    return imports_select_nodes(&id0);
}

int64_t select_edges(std::span<int64_t> src, std::span<int64_t> dst) {
    imports_series_id_t src0, dst0;
    src0.tag = IMPORTS_SERIES_ID_I64;
    dst0.tag = IMPORTS_SERIES_ID_I64;
    src0.val.i64 = {.ptr = const_cast<int64_t*>(src.data()), .len = src.size()};
    dst0.val.i64 = {.ptr = const_cast<int64_t*>(dst.data()), .len = dst.size()};

    return imports_select_edges(&src0, &dst0);
}

int64_t select_edges(std::span<std::string_view> src, std::span<std::string_view> dst) {
    auto ss = __dump_imports_list_string(src);
    auto ds = __dump_imports_list_string(dst);

    imports_series_id_t src0, dst0;
    src0.tag = IMPORTS_SERIES_ID_TXT;
    dst0.tag = IMPORTS_SERIES_ID_TXT;
    src0.val.txt = {.ptr = ss.data(), .len = ss.size()};
    dst0.val.txt = {.ptr = ds.data(), .len = ds.size()};

    return imports_select_edges(&src0, &dst0);
}
