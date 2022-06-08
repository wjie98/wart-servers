#ifndef __FUNCTIONAL_HPP__
#define __FUNCTIONAL_HPP__

#include <view.hpp>
#include <utils.hpp>
#include <log.hpp>
#include <option.hpp>

#include <span>
#include <tuple>
#include <memory>
// #include <optional>

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
    static option<data_frame> open(std::string_view name, std::span<item_param> defa) {
        return data_frame::open_impl(name, defa);
    }

    static option<data_frame> open(std::string_view name, std::initializer_list<item_param> defa) {
        return data_frame::open_impl(name, defa);
    }
    
    void push(std::span<item_param> data) { return this->push_impl(data); }
    void push(std::initializer_list<item_param> data) { return this->push_impl(data); }

    size_t size() const {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }

        uint64_t ret0;
        if (!imports_data_frame_size(this->_handle, &ret0)) {
            LOG_ABORT("data_frame::size")
        }
        return ret0;
    }

    bool is_owner() const { return this->_owner; }

    data_frame(data_frame&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; }
    ~data_frame() { if (this->_owner) imports_data_frame_free(&this->_handle); }
    data_frame& operator= (data_frame&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; return *this; }

private:
    template<typename K>
    static inline option<data_frame> open_impl(std::string_view name, K&& defa) {
        imports_string_t name0;
        __set_string_param(name0, name);

        imports_row_t defa0;
        auto owner = __set_span_item_param(defa0, defa);

        imports_data_frame_t ret0;
        if (imports_data_frame_new(&name0, &defa0, &ret0)) {
            return option<data_frame>::some(&ret0);
        }
        return option<data_frame>::none();
    }

    template<typename K>
    inline void push_impl(K&& data) {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }

        imports_row_t data0;
        auto owner = __set_span_item_param(data0, data);

        uint64_t ret0;
        if (!imports_data_frame_push(this->_handle, &data0, &ret0)) {
            LOG_ABORT("data_frame::push_impl")
        }
    }
};

class storage {
    imports_storage_t _handle;
    bool _owner;
    storage(imports_storage_t* handle) { this->_handle = *handle; this->_owner = true; }
public:
    static option<storage> open() {
        imports_storage_t ret0;
        if (imports_storage_new(&ret0)) {
            return option<storage>::some(&ret0);
        }
        return option<storage>::none();
    }

    bool is_owner() const { return this->_owner; }

    // choice-nodes

    [[nodiscard]] option<vector> choice_nodes(std::string_view tag, int32_t n) {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }

        imports_string_t tag0;
        __set_string_param(tag0, tag);

        imports_vector_t ret0;
        if (imports_storage_choice_nodes(this->_handle, &tag0, n, &ret0)) {
            return option<vector>::some(&ret0);
        }
        return option<vector>::none();
    }

    // query-nodes

    [[nodiscard]] option<row> query_nodes(int64_t id, std::string_view tag, std::span<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] option<row> query_nodes(int64_t id, std::string_view tag, std::initializer_list<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] option<row> query_nodes(std::string_view id, std::string_view tag, std::span<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    [[nodiscard]] option<row> query_nodes(std::string_view id, std::string_view tag, std::initializer_list<std::string_view> keys) {
        return this->query_nodes_impl(id, tag, keys);
    }

    // query-neighbors

    [[nodiscard]] option<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag,
        std::span<std::string_view> keys, bool reversely = false)
    {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] option<std::tuple<vector, table>> query_neighbors(int64_t id, std::string_view tag,
        std::initializer_list<std::string_view> keys, bool reversely = false)
    {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] option<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag,
        std::span<std::string_view> keys, bool reversely = false)
    {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    [[nodiscard]] option<std::tuple<vector, table>> query_neighbors(std::string_view id, std::string_view tag,
        std::initializer_list<std::string_view> keys, bool reversely = false)
    {
        return this->query_neighbors_impl(id, tag, keys, reversely);
    }

    // query-kv

    [[nodiscard]] option<vector> query_kv(std::span<std::string_view> keys, value_param defa) {
        return this->query_kv_impl(keys, defa);
    }

    [[nodiscard]] option<vector> query_kv(std::initializer_list<std::string_view> keys, value_param defa) {
        return this->query_kv_impl(keys, defa);
    }

    void update_kv(std::span<std::string_view> keys, vector_param vals, merge_type ops) {
        return this->update_kv_impl(keys, vals, ops);
    }

    void update_kv(std::initializer_list<std::string_view> keys, vector_param vals, merge_type ops) {
        return this->update_kv_impl(keys, vals, ops);
    }

    storage(storage&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; }
    ~storage() { if (this->_owner) imports_storage_free(&this->_handle); }
    storage& operator= (storage&& rhs) { this->_handle = rhs._handle; this->_owner = rhs._owner; rhs._owner = false; return *this; }

private:
    template<typename K>
    inline option<row> query_nodes_impl(value_param id, std::string_view tag, K keys) {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }

        imports_value_t id0;
        __set_value_param(id0, id);
        
        imports_string_t tag0;
        __set_string_param(tag0, tag);

        imports_list_string_t keys0;
        auto owner = __set_span_string_param(keys0, keys);

        imports_row_t ret0;
        if (imports_storage_query_nodes(this->_handle, &id0, &tag0, &keys0, &ret0)) {
            return option<row>::some(&ret0);
        }
        return option<row>::none();
    }

    template<typename K>
    inline option<std::tuple<vector, table>> query_neighbors_impl(value_param id, std::string_view tag, K&& keys, bool reversely) {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }

        imports_value_t id0;
        __set_value_param(id0, id);
        
        imports_string_t tag0;
        __set_string_param(tag0, tag);

        imports_list_string_t keys0;
        auto owner = __set_span_string_param(keys0, keys);

        imports_tuple2_vector_table_t ret0;
        if (imports_storage_query_neighbors(this->_handle, &id0, &tag0, &keys0, reversely, &ret0)) {
            return option<std::tuple<vector, table>>::some(std::make_tuple(&ret0.f0, &ret0.f1));
        }
        return option<std::tuple<vector, table>>::none();
    }

    template<typename K>
    inline option<vector> query_kv_impl(K&& keys, value_param defa) {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }

        imports_list_string_t keys0;
        auto owner = __set_span_string_param(keys0, keys);

        imports_value_t defa0;
        __set_value_param(defa0, defa);

        imports_vector_t ret0;
        if (imports_storage_query_kv(this->_handle, &keys0, &defa0, &ret0)) {
            return option<vector>::some(&ret0);
        }
        return option<vector>::none();
    }

    template<typename K>
    inline void update_kv_impl(K&& keys, vector_param vals, merge_type ops) {
        if (!this->is_owner()) {
            LOG_ABORT("object moved")
        }
        
        imports_list_string_t keys0;
        auto owner0 = __set_span_string_param(keys0, keys);

        imports_vector_t vals0;
        auto owner1 = __set_vector_param(vals0, vals);

        uint64_t ret0;
        if (!imports_storage_update_kv(this->_handle, &keys0, &vals0, ops, &ret0)) {
            LOG_ABORT("storage::update_kv_impl")
        }
    }
};

}

#endif