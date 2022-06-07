#ifndef __BINDINGS_IMPORTS_H
#define __BINDINGS_IMPORTS_H
#ifdef __cplusplus
extern "C"
{
  #endif
  
  #include <stdint.h>
  #include <stdbool.h>
  
  typedef struct {
    uint32_t idx;
  } imports_data_frame_t;
  void imports_data_frame_free(imports_data_frame_t *ptr);
  imports_data_frame_t imports_data_frame_clone(imports_data_frame_t *ptr);
  
  typedef struct {
    uint32_t idx;
  } imports_storage_t;
  void imports_storage_free(imports_storage_t *ptr);
  imports_storage_t imports_storage_clone(imports_storage_t *ptr);
  
  typedef struct {
    char *ptr;
    size_t len;
  } imports_string_t;
  
  void imports_string_set(imports_string_t *ret, const char *s);
  void imports_string_dup(imports_string_t *ret, const char *s);
  void imports_string_free(imports_string_t *ret);
  typedef struct {
  } imports_empty_t;
  typedef struct {
    uint8_t tag;
    union {
      bool bol;
      int32_t i32;
      int64_t i64;
      float f32;
      double f64;
      imports_string_t txt;
    } val;
  } imports_value_t;
  #define IMPORTS_VALUE_NIL 0
  #define IMPORTS_VALUE_BOL 1
  #define IMPORTS_VALUE_I32 2
  #define IMPORTS_VALUE_I64 3
  #define IMPORTS_VALUE_F32 4
  #define IMPORTS_VALUE_F64 5
  #define IMPORTS_VALUE_TXT 6
  void imports_value_free(imports_value_t *ptr);
  typedef struct {
    bool *ptr;
    size_t len;
  } imports_list_bool_t;
  void imports_list_bool_free(imports_list_bool_t *ptr);
  typedef struct {
    int32_t *ptr;
    size_t len;
  } imports_list_s32_t;
  void imports_list_s32_free(imports_list_s32_t *ptr);
  typedef struct {
    int64_t *ptr;
    size_t len;
  } imports_list_s64_t;
  void imports_list_s64_free(imports_list_s64_t *ptr);
  typedef struct {
    float *ptr;
    size_t len;
  } imports_list_float32_t;
  void imports_list_float32_free(imports_list_float32_t *ptr);
  typedef struct {
    double *ptr;
    size_t len;
  } imports_list_float64_t;
  void imports_list_float64_free(imports_list_float64_t *ptr);
  typedef struct {
    imports_string_t *ptr;
    size_t len;
  } imports_list_string_t;
  void imports_list_string_free(imports_list_string_t *ptr);
  typedef struct {
    uint8_t tag;
    union {
      imports_list_bool_t bol;
      imports_list_s32_t i32;
      imports_list_s64_t i64;
      imports_list_float32_t f32;
      imports_list_float64_t f64;
      imports_list_string_t txt;
    } val;
  } imports_vector_t;
  #define IMPORTS_VECTOR_NIL 0
  #define IMPORTS_VECTOR_BOL 1
  #define IMPORTS_VECTOR_I32 2
  #define IMPORTS_VECTOR_I64 3
  #define IMPORTS_VECTOR_F32 4
  #define IMPORTS_VECTOR_F64 5
  #define IMPORTS_VECTOR_TXT 6
  void imports_vector_free(imports_vector_t *ptr);
  typedef struct {
    imports_string_t key;
    imports_value_t val;
  } imports_item_t;
  void imports_item_free(imports_item_t *ptr);
  typedef struct {
    imports_string_t key;
    imports_vector_t val;
  } imports_series_t;
  void imports_series_free(imports_series_t *ptr);
  typedef struct {
    imports_item_t *ptr;
    size_t len;
  } imports_row_t;
  void imports_row_free(imports_row_t *ptr);
  typedef struct {
    imports_series_t *ptr;
    size_t len;
  } imports_table_t;
  void imports_table_free(imports_table_t *ptr);
  typedef uint8_t imports_merge_type_t;
  #define IMPORTS_MERGE_TYPE_DEL 0
  #define IMPORTS_MERGE_TYPE_ADD 1
  #define IMPORTS_MERGE_TYPE_MOV 2
  typedef uint8_t imports_log_level_t;
  #define IMPORTS_LOG_LEVEL_TRACE 0
  #define IMPORTS_LOG_LEVEL_DEBUG 1
  #define IMPORTS_LOG_LEVEL_INFO 2
  #define IMPORTS_LOG_LEVEL_WARN 3
  #define IMPORTS_LOG_LEVEL_ERROR 4
  bool imports_data_frame_new(imports_string_t *name, imports_row_t *default, imports_data_frame_t *ret0);
  bool imports_data_frame_push(imports_data_frame_t self, imports_row_t *data, imports_empty_t *ret0);
  bool imports_data_frame_size(imports_data_frame_t self, uint64_t *ret0);
  bool imports_storage_new(imports_storage_t *ret0);
  bool imports_storage_choice_nodes(imports_storage_t self, imports_string_t *tag, int32_t number, imports_series_t *ret0);
  bool imports_storage_query_node(imports_storage_t self, imports_value_t *id, imports_string_t *tag, imports_list_string_t *keys, imports_row_t *ret0);
  bool imports_storage_query_neighbors(imports_storage_t self, imports_value_t *id, imports_string_t *tag, imports_list_string_t *keys, bool reversely, imports_table_t *ret0);
  bool imports_storage_query_kv(imports_storage_t self, imports_row_t *data, imports_row_t *ret0);
  bool imports_storage_update_kv(imports_storage_t self, imports_row_t *data, imports_merge_type_t ops, imports_empty_t *ret0);
  void imports_log(imports_log_level_t lv, imports_string_t *msg);
  void imports_log_enabled(imports_log_level_t lv, imports_string_t *msg);
  #ifdef __cplusplus
}
#endif
#endif
