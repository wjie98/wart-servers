#ifndef __VIEW_HPP__
#define __VIEW_HPP__

#include <span>
#include <string>
#include <vector>
#include <optional>

#include <imports.h>

namespace imports {

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
    return std::string_view(x.key.ptr, x.key.len) == std::string_view(y.key.ptr, y.key.len);
}

inline bool __lt_imports_series(const imports_series_t& x, const imports_series_t& y) {
    return std::string_view(x.key.ptr, x.key.len) < std::string_view(y.key.ptr, y.key.len);
}

inline bool __eq_imports_series(const imports_series_t& x, const imports_series_t& y) {
    return std::string_view(x.key.ptr, x.key.len) == std::string_view(y.key.ptr, y.key.len);
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

}

#endif