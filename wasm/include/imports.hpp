#pragma once

#include <span>
#include <string>
#include <memory>
#include <tuple>
#include <variant>
#include <optional>
#include <vector>

namespace imports {

#include <imports.h>

enum log_level {
    trace = IMPORTS_LOG_LEVEL_TRACE,
    debug = IMPORTS_LOG_LEVEL_DEBUG,
    info  = IMPORTS_LOG_LEVEL_INFO,
    warn  = IMPORTS_LOG_LEVEL_WARN,
    error = IMPORTS_LOG_LEVEL_ERROR,
};

void log(log_level lv, const std::string_view msg) {
    imports_string_t s = {
        .ptr = const_cast<char*>(msg.data()),
        .len = msg.length(),
    };
    return imports_log(lv, &s);
}

bool log_enabled(log_level lv) {
    return imports_log_enabled(lv);
}

void log_trace(const std::string_view msg) { return log(log_level::trace, msg); }
void log_debug(const std::string_view msg) { return log(log_level::debug, msg); }
void log_info (const std::string_view msg) { return log(log_level::info,  msg); }
void log_warn (const std::string_view msg) { return log(log_level::warn,  msg); }
void log_error(const std::string_view msg) { return log(log_level::error, msg); }

enum merge_type {
    add = IMPORTS_MERGE_TYPE_ADD,
    mov = IMPORTS_MERGE_TYPE_MOV,
};

class value_view {
    imports_value_t _data;
public:
    enum value_type {
        nil = IMPORTS_VALUE_NIL,
        bol = IMPORTS_VALUE_BOL,
        i32 = IMPORTS_VALUE_I32,
        i64 = IMPORTS_VALUE_I64,
        f32 = IMPORTS_VALUE_F32,
        f64 = IMPORTS_VALUE_F64,
        txt = IMPORTS_VALUE_TXT,
    };

    value_view(const imports_value_t* data) { this->_data = *data; }
    value_type type() const { return static_cast<value_type>(this->_data.tag); }
    bool is_nil() const { return this->type() == value_type::nil; }
    bool is_bol() const { return this->type() == value_type::bol; }
    bool is_i32() const { return this->type() == value_type::i32; }
    bool is_i64() const { return this->type() == value_type::i64; }
    bool is_f32() const { return this->type() == value_type::f32; }
    bool is_f64() const { return this->type() == value_type::f64; }
    bool is_txt() const { return this->type() == value_type::txt; }

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

class item_view {
    imports_item_t _data;
public:
    item_view(const imports_item_t* data) { this->_data = *data; }
    std::string_view key() const {
        auto& key = this->_data.key;
        return {key.ptr, key.len};
    }
    value_view val() const { return &this->_data.val; }
};

class vector_view {
    imports_vector_t _data;
public:
    enum vector_type {
        nil = IMPORTS_VECTOR_NIL,
        bol = IMPORTS_VECTOR_BOL,
        i32 = IMPORTS_VECTOR_I32,
        i64 = IMPORTS_VECTOR_I64,
        f32 = IMPORTS_VECTOR_F32,
        f64 = IMPORTS_VECTOR_F64,
        txt = IMPORTS_VECTOR_TXT,
    };

    vector_view(const imports_vector_t* data) { this->_data = *data; }
    vector_type type() const { return static_cast<vector_type>(this->_data.tag); }
    bool is_nil() const { return this->type() == vector_type::nil; }
    bool is_bol() const { return this->type() == vector_type::bol; }
    bool is_i32() const { return this->type() == vector_type::i32; }
    bool is_i64() const { return this->type() == vector_type::i64; }
    bool is_f32() const { return this->type() == vector_type::f32; }
    bool is_f64() const { return this->type() == vector_type::f64; }
    bool is_txt() const { return this->type() == vector_type::txt; }

    size_t size() const {
        switch (this->type())
        {
        case vector_type::bol:
            return this->_data.val.bol.len;
        case vector_type::i32:
            return this->_data.val.i32.len;
        case vector_type::i64:
            return this->_data.val.i64.len;
        case vector_type::f32:
            return this->_data.val.f32.len;
        case vector_type::f64:
            return this->_data.val.f64.len;
        case vector_type::txt:
            return this->_data.val.txt.len;
        default:
            return 0;
        }
    }
    bool empty() const { return this->size() == 0; }
    
    std::span<bool> as_bol() const {
        auto& bol = this->_data.val.bol;
        return {bol.ptr, bol.len};
    }
    std::span<int32_t> as_i32() const {
        auto& i32 = this->_data.val.i32;
        return {i32.ptr, i32.len};
    }
    std::span<int64_t> as_i64() const {
        auto& i64 = this->_data.val.i64;
        return {i64.ptr, i64.len};
    }
    std::span<float> as_f32() const {
        auto& f32 = this->_data.val.f32;
        return {f32.ptr, f32.len};
    }
    std::span<double> as_f64() const {
        auto& f64 = this->_data.val.f64;
        return {f64.ptr, f64.len};
    }
    std::vector<std::string_view> as_txt() const {
        auto& txt = this->_data.val.txt;
        
        std::vector<std::string_view> view;
        view.reserve(txt.len);

        for (size_t i = 0; i < txt.len; i++) {
            view.emplace_back(txt.ptr[i].ptr, txt.ptr[i].len);
        }
        return view;
    }
    std::string_view view_string(size_t i) const {
        auto& txt = this->_data.val.txt;
        auto& str = std::span<imports_string_t>(txt.ptr, txt.len)[i];
        return {str.ptr, str.len};
    }
};

class series_view {
    imports_series_t _data;
public:
    series_view(const imports_series_t* data) { this->_data = *data; }
    std::string_view key() const {
        auto& key = this->_data.key;
        return {key.ptr, key.len};
    }
    vector_view val() const { return &this->_data.val; }
    size_t size() const { return this->val().size(); }
    bool empty() const { return this->val().empty(); }
};

inline bool __lt_imports_item(const imports_item_t& x, const imports_item_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

inline bool __eq_imports_item(const imports_item_t& x, const imports_item_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

inline bool __lt_imports_series(const imports_series_t& x, const imports_series_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

inline bool __eq_imports_series(const imports_series_t& x, const imports_series_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

class row_view {
    imports_row_t _data;
public:
    row_view(const imports_row_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    bool empty() const { return this->size() == 0; }
    item_view operator[] (size_t i) const {
        return &std::span<imports_item_t>(this->_data.ptr, this->_data.len)[i];
    }
    std::optional<value_view> operator[] (std::string_view key) const {
        std::span<imports_item_t> span0 = {this->_data.ptr, this->_data.len};
        imports_item_t item0 = {
            .key = {
                .ptr = const_cast<char*>(key.data()),
                .len = key.length()
            }
        };

        auto it = std::lower_bound(span0.begin(), span0.end(), item0, __lt_imports_item);

        if (it != span0.end() && __eq_imports_item(*it, item0)) {
            return &it->val;
        }
        return nullptr;
    }
};

class table_view {
    imports_table_t _data;
public:
    table_view(const imports_table_t* data) { this->_data = *data; }
    size_t size() const { return this->_data.len; }
    bool empty() const { return this->size() == 0; }
    series_view operator[] (size_t i) const {
        return &std::span<imports_series_t>(this->_data.ptr, this->_data.len)[i];
    }
    std::optional<vector_view> operator[] (std::string_view key) const {
        std::span<imports_series_t> span0 = {this->_data.ptr, this->_data.len};
        imports_series_t series0 = {
            .key = {
                .ptr = const_cast<char*>(key.data()),
                .len = key.length()
            }
        };

        auto it = std::lower_bound(span0.begin(), span0.end(), series0, __lt_imports_series);

        if (it != span0.end() && __eq_imports_series(*it, series0)) {
            return &it->val;
        }
        return nullptr;
    }
};

class row {
    imports_row_t _data;
public:
    row(imports_row_t* data) {
        this->_data = *data; *data = {NULL, 0};
        auto p = this->_data.ptr;
        auto n = this->_data.len;
        std::sort(p, p + n, __lt_imports_item);
    }
    row(row&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; }
    ~row() { imports_row_free(&this->_data); }
    row& operator= (row&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; return *this; }
    row_view view() const { return &this->_data; }
    row_view operator* () const { return this->view(); }
};

class vector {
    imports_vector_t _data;
public:
    vector(imports_vector_t* data) {
        this->_data = *data;
        data->tag = IMPORTS_VECTOR_NIL;
    }
    vector(vector&& rhs) {
        this->_data = rhs._data;
        rhs._data.tag = IMPORTS_VECTOR_NIL;
    }
    ~vector() { imports_vector_free(&this->_data); }
    vector& operator= (vector&& rhs) {
        this->_data = rhs._data;
        rhs._data.tag = IMPORTS_VECTOR_NIL;
        return *this;
    }
    vector_view view() const { return &this->_data; }
    vector_view operator* () const { return this->view(); }
};

class series {
    imports_series_t _data;
public:
    series(imports_series_t* data) {
        this->_data = *data;
        data->key = {NULL, 0};
        data->val = {.tag = IMPORTS_VECTOR_NIL};
    }
    series(series&& rhs) {
        this->_data = rhs._data;
        rhs._data.key = {NULL, 0};
        rhs._data.val = {.tag = IMPORTS_VECTOR_NIL};
    }
    ~series() { imports_series_free(&this->_data); }
    series& operator= (series&& rhs) {
        this->_data = rhs._data;
        rhs._data.key = {NULL, 0};
        rhs._data.val = {.tag = IMPORTS_VECTOR_NIL};
        return *this;
    }
    series_view view() const { return &this->_data; }
    series_view operator* () const { return this->view(); }
};

class table {
    imports_table_t _data;
public:
    table(imports_table_t* data) {
        this->_data = *data; *data = {NULL, 0};
        auto p = this->_data.ptr;
        auto n = this->_data.len;
        std::sort(p, p + n, __lt_imports_series);
    }
    table(table&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; }
    ~table() { imports_table_free(&this->_data); }
    table& operator= (table&& rhs) { this->_data = rhs._data; rhs._data = {NULL, 0}; return *this; }
    table_view view() const { return &this->_data; }
    table_view operator* () const { return this->view(); }
};

using value_param = std::variant<
    bool, int32_t, int64_t, float, double,
    std::string_view>;
using vector_param = std::variant<
    std::span<bool>, std::span<int32_t>, std::span<int64_t>, std::span<float>, std::span<double>,
    std::span<std::string_view>, std::span<std::string>>;
struct item_param {
    std::string_view key;
    value_param val;
};

struct series_param {
    std::string_view key;
    vector_param val;
};

void __set_string_param(imports_string_t& ret0, const std::string_view param) {
    ret0.ptr = const_cast<char*>(param.data());
    ret0.len = param.length();
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_string_param(imports_list_string_t& ret0, const std::span<std::string_view> param) {
    void* ptr = malloc(sizeof(imports_string_t) * param.size());
    
    ret0.ptr = reinterpret_cast<imports_string_t*>(ptr);
    ret0.len = param.size();

    for (size_t i = 0; i < ret0.len; i++) {
        __set_string_param(ret0.ptr[i], param[i]);
    }
    return {ptr, free};
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_string_param(imports_list_string_t& ret0, const std::span<std::string> param) {
    void* ptr = malloc(sizeof(imports_string_t) * param.size());
    
    ret0.ptr = reinterpret_cast<imports_string_t*>(ptr);
    ret0.len = param.size();

    for (size_t i = 0; i < ret0.len; i++) {
        __set_string_param(ret0.ptr[i], param[i]);
    }
    return {ptr, free};
}

void __set_value_param(imports_value_t& ret0, const value_param param) {
    if (auto it = std::get_if<bool>(&param); it) {
        ret0.tag = IMPORTS_VALUE_BOL;
        ret0.val.bol = *it;
    } else if (auto it = std::get_if<int32_t>(&param); it) {
        ret0.tag = IMPORTS_VALUE_I32;
        ret0.val.i32 = *it;
    } else if (auto it = std::get_if<int64_t>(&param); it) {
        ret0.tag = IMPORTS_VALUE_I64;
        ret0.val.i64 = *it;
    } else if (auto it = std::get_if<float>(&param); it) {
        ret0.tag = IMPORTS_VALUE_F32;
        ret0.val.f32 = *it;
    } else if (auto it = std::get_if<double>(&param); it) {
        ret0.tag = IMPORTS_VALUE_F64;
        ret0.val.f64 = *it;
    } else if (auto it = std::get_if<std::string_view>(&param); it) {
        ret0.tag = IMPORTS_VALUE_TXT;
        __set_string_param(ret0.val.txt, *it);
    } else {
        log_error("invalid value_param");
        abort();
    }
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_vector_param(imports_vector_t& ret0, const vector_param param) {
    void* ptr = NULL;
    if (auto it = std::get_if<std::span<bool>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_BOL;
        ret0.val.bol.ptr = const_cast<bool*>(it->data());
        ret0.val.bol.len = it->size();
    } else if (auto it = std::get_if<std::span<int32_t>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_I32;
        ret0.val.i32.ptr = const_cast<int32_t*>(it->data());
        ret0.val.i32.len = it->size();
    } else if (auto it = std::get_if<std::span<int64_t>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_I64;
        ret0.val.i64.ptr = const_cast<int64_t*>(it->data());
        ret0.val.i64.len = it->size();
    } else if (auto it = std::get_if<std::span<float>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_F32;
        ret0.val.f32.ptr = const_cast<float*>(it->data());
        ret0.val.f32.len = it->size();
    } else if (auto it = std::get_if<std::span<double>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_F64;
        ret0.val.f64.ptr = const_cast<double*>(it->data());
        ret0.val.f64.len = it->size();
    } else if (auto it = std::get_if<std::span<std::string_view>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_TXT;

        auto& txt = ret0.val.txt;
        ptr = malloc(sizeof(imports_string_t) * it->size());

        txt.ptr = reinterpret_cast<imports_string_t*>(ptr);
        txt.len = it->size();

        for (size_t i = 0; i < txt.len; i++) {
            __set_string_param(txt.ptr[i], it->data()[i]);
        }

    } else if (auto it = std::get_if<std::span<std::string>>(&param); it) {
        ret0.tag = IMPORTS_VECTOR_TXT;

        auto& txt = ret0.val.txt;
        ptr = malloc(sizeof(imports_string_t) * it->size());

        txt.ptr = reinterpret_cast<imports_string_t*>(ptr);
        txt.len = it->size();
        
        for (size_t i = 0; i < txt.len; i++) {
            __set_string_param(txt.ptr[i], it->data()[i]);
        }
    } else {
        log_error("invalid vector_param");
        abort();
    }
    return {ptr, free};
}

void __set_item_param(imports_item_t& ret0, const item_param param) {
    __set_string_param(ret0.key, param.key);
    __set_value_param(ret0.val, param.val);
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_span_item_param(imports_row_t& ret0, std::span<item_param> param) {
    void* ptr = malloc(sizeof(imports_item_t) * param.size());

    ret0.ptr = reinterpret_cast<imports_item_t*>(ptr);
    ret0.len = param.size();

    for (size_t i = 0; i < ret0.len; i++) {
        __set_item_param(ret0.ptr[i], param[i]);
    }

    return {ptr, free};
}

[[nodiscard]] std::unique_ptr<void, decltype(free)*> __set_series_param(imports_series_t& ret0, const series_param param) {
    __set_string_param(ret0.key, param.key);
    return __set_vector_param(ret0.val, param.val);
}

class data_frame {
    imports_data_frame_t _handle;
    bool _owner;
    data_frame(imports_data_frame_t* handle) { this->_handle = *handle; this->_owner = true; }
public:
    static std::optional<data_frame> open(std::string_view name, std::span<item_param> defa) {
        
        imports_string_t name0;
        __set_string_param(name0, name);

        imports_row_t defa0;
        auto owner = __set_span_item_param(defa0, defa);

        imports_data_frame_t ret0;
        if (imports_data_frame_new(&name0, &defa0, &ret0)) {
            return data_frame(&ret0);
        }
        return std::nullopt;
    }
    
    void push(std::span<item_param> data) {
        imports_row_t data0;
        auto owner = __set_span_item_param(data0, data);

        uint64_t ret0;
        if (!imports_data_frame_push(this->_handle, &data0, &ret0)) {
            abort();
        }
    }

    size_t size() const {
        uint64_t ret0;
        if (!imports_data_frame_size(this->_handle, &ret0)) {
            abort();
        }
        return ret0;
    }

    data_frame(data_frame&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; }
    ~data_frame() { if (this->_owner) imports_data_frame_free(&this->_handle); }
    data_frame& operator= (data_frame&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; return *this; }
};

class storage {
    imports_storage_t _handle;
    bool _owner;
    storage(imports_storage_t* handle) { this->_handle = *handle; this->_owner = true; }
public:
    static std::optional<storage> open() {
        imports_storage_t ret0;
        if (imports_storage_new(&ret0)) {
            return storage(&ret0);
        }
        return std::nullopt;
    }

    // choice-nodes

    [[nodiscard]] std::optional<vector> choice_nodes(std::string_view tag, int32_t n) {
        imports_string_t tag0;
        __set_string_param(tag0, tag);

        imports_vector_t ret0;
        if (imports_storage_choice_nodes(this->_handle, &tag0, n, &ret0)) {
            return &ret0;
        }
        return std::nullopt;
    }

    // query-nodes

    [[nodiscard]] std::optional<row> query_nodes(int64_t id, std::string_view tag, std::span<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] std::optional<row> query_nodes(int64_t id, std::string_view tag, std::span<std::string> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] std::optional<row> query_nodes(std::string_view id, std::string_view tag, std::span<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] std::optional<row> query_nodes(std::string_view id, std::string_view tag, std::span<std::string> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    // query-neighbors

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag, std::span<std::string_view> keys, bool reversely = false) {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag, std::span<std::string> keys, bool reversely = false) {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag, std::span<std::string_view> keys, bool reversely = false) {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag, std::span<std::string> keys, bool reversely = false) {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    // query-kv

    [[nodiscard]] std::optional<vector> query_kv(std::span<std::string_view> keys, value_param defa) {
        return this->query_kv_impl(keys, defa);
    }

    [[nodiscard]] std::optional<vector> query_kv(std::span<std::string> keys, value_param defa) {
        return this->query_kv_impl(keys, defa);
    }

    void update_kv(std::span<std::string_view> keys, vector_param vals, merge_type ops) {
        return this->update_kv_impl(keys, vals, ops);
    }

    void update_kv(std::span<std::string> keys, vector_param vals, merge_type ops) {
        return this->update_kv_impl(keys, vals, ops);
    }

    storage(storage&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; }
    ~storage() { if (this->_owner) imports_storage_free(&this->_handle); }
    storage& operator= (storage&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; return *this; }

private:
    template<typename K>
    std::optional<row> query_nodes_impl(value_param id, std::string_view tag, K keys) {
        imports_value_t id0;
        __set_value_param(id0, id);
        
        imports_string_t tag0;
        __set_string_param(tag0, tag);

        imports_list_string_t keys0;
        auto owner = __set_span_string_param(keys0, keys);

        imports_row_t ret0;
        if (imports_storage_query_nodes(this->_handle, &id0, &tag0, &keys0, &ret0)) {
            return &ret0;
        }
        return std::nullopt;
    }

    template<typename K>
    std::optional<std::tuple<vector, table>> query_neighbors_impl(value_param id, std::string_view tag, K keys, bool reversely) {
        imports_value_t id0;
        __set_value_param(id0, id);
        
        imports_string_t tag0;
        __set_string_param(tag0, tag);

        imports_list_string_t keys0;
        auto owner = __set_span_string_param(keys0, keys);

        imports_tuple2_vector_table_t ret0;
        if (imports_storage_query_neighbors(this->_handle, &id0, &tag0, &keys0, reversely, &ret0)) {
            return std::make_tuple(&ret0.f0, &ret0.f1);
        }
        return std::nullopt;
    }

    template<typename K>
    std::optional<vector> query_kv_impl(K keys, value_param defa) {
        imports_list_string_t keys0;
        auto owner = __set_span_string_param(keys0, keys);

        imports_value_t defa0;
        __set_value_param(defa0, defa);

        imports_vector_t ret0;
        if (imports_storage_query_kv(this->_handle, &keys0, &defa0, &ret0)) {
            return &ret0;
        }
        return std::nullopt;
    }

    template<typename K>
    void update_kv_impl(K keys, vector_param vals, merge_type ops) {
        imports_list_string_t keys0;
        auto owner0 = __set_span_string_param(keys0, keys);

        imports_vector_t vals0;
        auto owner1 = __set_vector_param(vals0, vals);

        uint64_t ret0;
        if (!imports_storage_update_kv(this->_handle, &keys0, &vals0, ops, &ret0)) {
            abort();
        }
    }
};

}
