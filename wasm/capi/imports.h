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
  } imports_future_row_t;
  void imports_future_row_free(imports_future_row_t *ptr);
  imports_future_row_t imports_future_row_clone(imports_future_row_t *ptr);
  
  typedef struct {
    uint32_t idx;
  } imports_future_table_t;
  void imports_future_table_free(imports_future_table_t *ptr);
  imports_future_table_t imports_future_table_clone(imports_future_table_t *ptr);
  
  typedef struct {
    char *ptr;
    size_t len;
  } imports_string_t;
  
  void imports_string_set(imports_string_t *ret, const char *s);
  void imports_string_dup(imports_string_t *ret, const char *s);
  void imports_string_free(imports_string_t *ret);
  typedef struct {
    uint8_t tag;
    union {
      int64_t i64;
      imports_string_t txt;
    } val;
  } imports_node_id_t;
  #define IMPORTS_NODE_ID_I64 0
  #define IMPORTS_NODE_ID_TXT 1
  void imports_node_id_free(imports_node_id_t *ptr);
  typedef struct {
    int64_t *ptr;
    size_t len;
  } imports_list_s64_t;
  void imports_list_s64_free(imports_list_s64_t *ptr);
  typedef struct {
    imports_string_t *ptr;
    size_t len;
  } imports_list_string_t;
  void imports_list_string_free(imports_list_string_t *ptr);
  typedef struct {
    uint8_t tag;
    union {
      imports_list_s64_t i64;
      imports_list_string_t txt;
    } val;
  } imports_series_id_t;
  #define IMPORTS_SERIES_ID_I64 0
  #define IMPORTS_SERIES_ID_TXT 1
  void imports_series_id_free(imports_series_id_t *ptr);
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
    uint8_t tag;
    union {
      imports_list_bool_t bol;
      imports_list_s32_t i32;
      imports_list_s64_t i64;
      imports_list_float32_t f32;
      imports_list_float64_t f64;
      imports_list_string_t txt;
    } val;
  } imports_series_t;
  #define IMPORTS_SERIES_NIL 0
  #define IMPORTS_SERIES_BOL 1
  #define IMPORTS_SERIES_I32 2
  #define IMPORTS_SERIES_I64 3
  #define IMPORTS_SERIES_F32 4
  #define IMPORTS_SERIES_F64 5
  #define IMPORTS_SERIES_TXT 6
  void imports_series_free(imports_series_t *ptr);
  typedef struct {
    imports_value_t *ptr;
    size_t len;
  } imports_row_t;
  void imports_row_free(imports_row_t *ptr);
  typedef struct {
    imports_series_t *ptr;
    size_t len;
  } imports_table_t;
  void imports_table_free(imports_table_t *ptr);
  typedef uint8_t imports_future_error_t;
  #define IMPORTS_FUTURE_ERROR_EMPTY 0
  #define IMPORTS_FUTURE_ERROR_CHANNEL_DROPPED 1
  #define IMPORTS_FUTURE_ERROR_CHANNEL_CLOSED 2
  imports_future_error_t imports_get_future_row(imports_future_row_t x, imports_row_t *ret0);
  imports_future_error_t imports_get_future_table(imports_future_table_t x, imports_table_t *ret0);
  imports_future_error_t imports_async_query_kv(imports_list_string_t *keys, imports_future_row_t *ret0);
  imports_future_error_t imports_async_query_node(imports_node_id_t *id, imports_string_t *tag, imports_list_string_t *keys, imports_future_row_t *ret0);
  imports_future_error_t imports_async_choice_nodes(imports_string_t *tag, int32_t number, imports_future_table_t *ret0);
  imports_future_error_t imports_async_query_neighbors(imports_node_id_t *id, imports_string_t *tag, imports_list_string_t *keys, bool reversely, imports_future_table_t *ret0);
  int64_t imports_select_nodes(imports_series_id_t *id);
  int64_t imports_select_edges(imports_series_id_t *src, imports_series_id_t *dst);
  #ifdef __cplusplus
}
#endif
#endif
