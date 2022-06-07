#ifndef __FUNCTIONAL_HPP__
#define __FUNCTIONAL_HPP__

#include <view.hpp>
#include <utils.hpp>
#include <log.hpp>

#include <span>
#include <tuple>
#include <memory>
#include <optional>

namespace imports {

enum merge_type {
    add = IMPORTS_MERGE_TYPE_ADD,
    mov = IMPORTS_MERGE_TYPE_MOV,
};

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

    static std::optional<data_frame> open(std::string_view name, std::initializer_list<item_param> defa) {
        std::vector<item_param> defa0 = defa;
        return data_frame::open(name, defa0);
    }
    
    void push(std::span<item_param> data) {
        imports_row_t data0;
        auto owner = __set_span_item_param(data0, data);

        uint64_t ret0;
        if (!imports_data_frame_push(this->_handle, &data0, &ret0)) {
            abort();
        }
    }

    void push(std::initializer_list<item_param> args) {
        std::vector<item_param> args0 = args;
        return push(args0);
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

    [[nodiscard]] std::optional<row> query_nodes(int64_t id, std::string_view tag, std::initializer_list<std::string_view> keys) {
        std::vector<std::string_view> keys0 = keys;
        return this->query_nodes_impl(id, tag, keys0);
    }

    // [[nodiscard]] std::optional<row> query_nodes(int64_t id, std::string_view tag, std::span<std::string> keys) {
    //     return this->query_nodes_impl(id, tag, keys);
    // }

    [[nodiscard]] std::optional<row> query_nodes(std::string_view id, std::string_view tag, std::span<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] std::optional<row> query_nodes(std::string_view id, std::string_view tag, std::initializer_list<std::string_view> keys) {
        std::vector<std::string_view> keys0 = keys;
        return this->query_nodes_impl(id, tag, keys0);
    }

    // [[nodiscard]] std::optional<row> query_nodes(std::string_view id, std::string_view tag, std::span<std::string> keys) {
    //     return this->query_nodes_impl(id, tag, keys);
    // }

    // query-neighbors

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag,
        std::span<std::string_view> keys, bool reversely = false)
    {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag,
        std::initializer_list<std::string_view> keys, bool reversely = false)
    {
        std::vector<std::string_view> keys0 = keys;
        return this->query_neighbors_impl(id, tag, keys0, reversely);
    }

    // [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag,
    //     std::span<std::string> keys, bool reversely = false)
    // {
    //     return this->query_neighbors_impl(id, tag, keys, reversely);
    // }

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag,
        std::span<std::string_view> keys, bool reversely = false)
    {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag,
        std::initializer_list<std::string_view> keys, bool reversely = false)
    {
        std::vector<std::string_view> keys0 = keys;
        return this->query_neighbors_impl(id, tag, keys0, reversely);
    }

    // [[nodiscard]] std::optional<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag,
    //     std::span<std::string> keys, bool reversely = false)
    // {
    //     return this->query_neighbors_impl(id, tag, keys, reversely);
    // }

    // query-kv

    [[nodiscard]] std::optional<vector> query_kv(std::span<std::string_view> keys, value_param defa) {
        return this->query_kv_impl(keys, defa);
    }

    [[nodiscard]] std::optional<vector> query_kv(std::initializer_list<std::string_view> keys, value_param defa) {
        std::vector<std::string_view> keys0 = keys;
        return this->query_kv_impl(keys0, defa);
    }

    // [[nodiscard]] std::optional<vector> query_kv(std::span<std::string> keys, value_param defa) {
    //     return this->query_kv_impl(keys, defa);
    // }

    void update_kv(std::span<std::string_view> keys, vector_param vals, merge_type ops) {
        return this->update_kv_impl(keys, vals, ops);
    }

    void update_kv(std::initializer_list<std::string_view> keys, vector_param vals, merge_type ops) {
        std::vector<std::string_view> keys0 = keys;
        return this->update_kv_impl(keys0, vals, ops);
    }

    // void update_kv(std::span<std::string> keys, vector_param vals, merge_type ops) {
    //     return this->update_kv_impl(keys, vals, ops);
    // }

    storage(storage&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; }
    ~storage() { if (this->_owner) imports_storage_free(&this->_handle); }
    storage& operator= (storage&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; return *this; }

private:
    template<typename K>
    inline std::optional<row> query_nodes_impl(value_param id, std::string_view tag, K keys) {
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
    inline std::optional<std::tuple<vector, table>> query_neighbors_impl(value_param id, std::string_view tag, K&& keys, bool reversely) {
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
    inline std::optional<vector> query_kv_impl(K&& keys, value_param defa) {
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
    inline void update_kv_impl(K&& keys, vector_param vals, merge_type ops) {
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

#endif