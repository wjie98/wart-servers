#include <stdlib.h>
#include <imports.h>

__attribute__((weak, export_name("canonical_abi_realloc")))
void *canonical_abi_realloc(
void *ptr,
size_t orig_size,
size_t org_align,
size_t new_size
) {
  void *ret = realloc(ptr, new_size);
  if (!ret)
  abort();
  return ret;
}

__attribute__((weak, export_name("canonical_abi_free")))
void canonical_abi_free(
void *ptr,
size_t size,
size_t align
) {
  free(ptr);
}

__attribute__((import_module("canonical_abi"), import_name("resource_drop_future-row")))
void __resource_future_row_drop(uint32_t idx);

void imports_future_row_free(imports_future_row_t *ptr) {
  __resource_future_row_drop(ptr->idx);
}

__attribute__((import_module("canonical_abi"), import_name("resource_clone_future-row")))
uint32_t __resource_future_row_clone(uint32_t idx);

imports_future_row_t imports_future_row_clone(imports_future_row_t *ptr) {
  return (imports_future_row_t){__resource_future_row_clone(ptr->idx)};
}

__attribute__((import_module("canonical_abi"), import_name("resource_drop_future-table")))
void __resource_future_table_drop(uint32_t idx);

void imports_future_table_free(imports_future_table_t *ptr) {
  __resource_future_table_drop(ptr->idx);
}

__attribute__((import_module("canonical_abi"), import_name("resource_clone_future-table")))
uint32_t __resource_future_table_clone(uint32_t idx);

imports_future_table_t imports_future_table_clone(imports_future_table_t *ptr) {
  return (imports_future_table_t){__resource_future_table_clone(ptr->idx)};
}
#include <string.h>

void imports_string_set(imports_string_t *ret, const char *s) {
  ret->ptr = (char*) s;
  ret->len = strlen(s);
}

void imports_string_dup(imports_string_t *ret, const char *s) {
  ret->len = strlen(s);
  ret->ptr = canonical_abi_realloc(NULL, 0, 1, ret->len);
  memcpy(ret->ptr, s, ret->len);
}

void imports_string_free(imports_string_t *ret) {
  canonical_abi_free(ret->ptr, ret->len, 1);
  ret->ptr = NULL;
  ret->len = 0;
}
void imports_node_id_free(imports_node_id_t *ptr) {
  switch ((int32_t) ptr->tag) {
    case 1: {
      imports_string_free(&ptr->val.txt);
      break;
    }
  }
}
void imports_list_s64_free(imports_list_s64_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 8, 8);
}
void imports_list_string_free(imports_list_string_t *ptr) {
  for (size_t i = 0; i < ptr->len; i++) {
    imports_string_free(&ptr->ptr[i]);
  }
  canonical_abi_free(ptr->ptr, ptr->len * 8, 4);
}
void imports_series_id_free(imports_series_id_t *ptr) {
  switch ((int32_t) ptr->tag) {
    case 0: {
      imports_list_s64_free(&ptr->val.i64);
      break;
    }
    case 1: {
      imports_list_string_free(&ptr->val.txt);
      break;
    }
  }
}
void imports_value_free(imports_value_t *ptr) {
  switch ((int32_t) ptr->tag) {
    case 6: {
      imports_string_free(&ptr->val.txt);
      break;
    }
  }
}
void imports_list_bool_free(imports_list_bool_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 1, 1);
}
void imports_list_s32_free(imports_list_s32_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 4, 4);
}
void imports_list_float32_free(imports_list_float32_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 4, 4);
}
void imports_list_float64_free(imports_list_float64_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 8, 8);
}
void imports_series_free(imports_series_t *ptr) {
  switch ((int32_t) ptr->tag) {
    case 1: {
      imports_list_bool_free(&ptr->val.bol);
      break;
    }
    case 2: {
      imports_list_s32_free(&ptr->val.i32);
      break;
    }
    case 3: {
      imports_list_s64_free(&ptr->val.i64);
      break;
    }
    case 4: {
      imports_list_float32_free(&ptr->val.f32);
      break;
    }
    case 5: {
      imports_list_float64_free(&ptr->val.f64);
      break;
    }
    case 6: {
      imports_list_string_free(&ptr->val.txt);
      break;
    }
  }
}
void imports_row_free(imports_row_t *ptr) {
  for (size_t i = 0; i < ptr->len; i++) {
    imports_value_free(&ptr->ptr[i]);
  }
  canonical_abi_free(ptr->ptr, ptr->len * 16, 8);
}
void imports_table_free(imports_table_t *ptr) {
  for (size_t i = 0; i < ptr->len; i++) {
    imports_series_free(&ptr->ptr[i]);
  }
  canonical_abi_free(ptr->ptr, ptr->len * 12, 4);
}
typedef struct {
  bool is_err;
  union {
    imports_row_t ok;
    imports_future_error_t err;
  } val;
} imports_expected_row_future_error_t;
typedef struct {
  bool is_err;
  union {
    imports_table_t ok;
    imports_future_error_t err;
  } val;
} imports_expected_table_future_error_t;
typedef struct {
  bool is_err;
  union {
    imports_future_row_t ok;
    imports_future_error_t err;
  } val;
} imports_expected_future_row_future_error_t;
typedef struct {
  bool is_err;
  union {
    imports_future_table_t ok;
    imports_future_error_t err;
  } val;
} imports_expected_future_table_future_error_t;

__attribute__((aligned(4)))
static uint8_t RET_AREA[12];
__attribute__((import_module("imports"), import_name("get-future-row")))
void __wasm_import_imports_get_future_row(int32_t, int32_t);
imports_future_error_t imports_get_future_row(imports_future_row_t x, imports_row_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_get_future_row((x).idx, ptr);
  imports_expected_row_future_error_t expected;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      expected.is_err = false;
      
      expected.val.ok = (imports_row_t) { (imports_value_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      expected.is_err = true;
      
      expected.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }*ret0 = expected.val.ok;
  return expected.is_err ? expected.val.err : -1;
}
__attribute__((import_module("imports"), import_name("get-future-table")))
void __wasm_import_imports_get_future_table(int32_t, int32_t);
imports_future_error_t imports_get_future_table(imports_future_table_t x, imports_table_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_get_future_table((x).idx, ptr);
  imports_expected_table_future_error_t expected;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      expected.is_err = false;
      
      expected.val.ok = (imports_table_t) { (imports_series_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
    case 1: {
      expected.is_err = true;
      
      expected.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }*ret0 = expected.val.ok;
  return expected.is_err ? expected.val.err : -1;
}
__attribute__((import_module("imports"), import_name("async-query-kv")))
void __wasm_import_imports_async_query_kv(int32_t, int32_t, int32_t);
imports_future_error_t imports_async_query_kv(imports_list_string_t *keys, imports_future_row_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_async_query_kv((int32_t) (*keys).ptr, (int32_t) (*keys).len, ptr);
  imports_expected_future_row_future_error_t expected;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      expected.is_err = false;
      
      expected.val.ok = (imports_future_row_t){ *((int32_t*) (ptr + 4)) };
      break;
    }
    case 1: {
      expected.is_err = true;
      
      expected.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }*ret0 = expected.val.ok;
  return expected.is_err ? expected.val.err : -1;
}
__attribute__((import_module("imports"), import_name("async-query-node")))
void __wasm_import_imports_async_query_node(int32_t, int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
imports_future_error_t imports_async_query_node(imports_node_id_t *id, imports_string_t *tag, imports_list_string_t *keys, imports_future_row_t *ret0) {
  int32_t variant;
  int64_t variant1;
  int32_t variant2;
  switch ((int32_t) (*id).tag) {
    case 0: {
      const int64_t *payload = &(*id).val.i64;
      variant = 0;
      variant1 = *payload;
      variant2 = 0;
      break;
    }
    case 1: {
      const imports_string_t *payload0 = &(*id).val.txt;
      variant = 1;
      variant1 = (int64_t) (int32_t) (*payload0).ptr;
      variant2 = (int32_t) (*payload0).len;
      break;
    }
  }
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_async_query_node(variant, variant1, variant2, (int32_t) (*tag).ptr, (int32_t) (*tag).len, (int32_t) (*keys).ptr, (int32_t) (*keys).len, ptr);
  imports_expected_future_row_future_error_t expected;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      expected.is_err = false;
      
      expected.val.ok = (imports_future_row_t){ *((int32_t*) (ptr + 4)) };
      break;
    }
    case 1: {
      expected.is_err = true;
      
      expected.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }*ret0 = expected.val.ok;
  return expected.is_err ? expected.val.err : -1;
}
__attribute__((import_module("imports"), import_name("async-choice-nodes")))
void __wasm_import_imports_async_choice_nodes(int32_t, int32_t, int32_t, int32_t);
imports_future_error_t imports_async_choice_nodes(imports_string_t *tag, int32_t number, imports_future_table_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_async_choice_nodes((int32_t) (*tag).ptr, (int32_t) (*tag).len, number, ptr);
  imports_expected_future_table_future_error_t expected;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      expected.is_err = false;
      
      expected.val.ok = (imports_future_table_t){ *((int32_t*) (ptr + 4)) };
      break;
    }
    case 1: {
      expected.is_err = true;
      
      expected.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }*ret0 = expected.val.ok;
  return expected.is_err ? expected.val.err : -1;
}
__attribute__((import_module("imports"), import_name("async-query-neighbors")))
void __wasm_import_imports_async_query_neighbors(int32_t, int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
imports_future_error_t imports_async_query_neighbors(imports_node_id_t *id, imports_string_t *tag, imports_list_string_t *keys, bool reversely, imports_future_table_t *ret0) {
  int32_t variant;
  int64_t variant1;
  int32_t variant2;
  switch ((int32_t) (*id).tag) {
    case 0: {
      const int64_t *payload = &(*id).val.i64;
      variant = 0;
      variant1 = *payload;
      variant2 = 0;
      break;
    }
    case 1: {
      const imports_string_t *payload0 = &(*id).val.txt;
      variant = 1;
      variant1 = (int64_t) (int32_t) (*payload0).ptr;
      variant2 = (int32_t) (*payload0).len;
      break;
    }
  }
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_async_query_neighbors(variant, variant1, variant2, (int32_t) (*tag).ptr, (int32_t) (*tag).len, (int32_t) (*keys).ptr, (int32_t) (*keys).len, reversely, ptr);
  imports_expected_future_table_future_error_t expected;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      expected.is_err = false;
      
      expected.val.ok = (imports_future_table_t){ *((int32_t*) (ptr + 4)) };
      break;
    }
    case 1: {
      expected.is_err = true;
      
      expected.val.err = (int32_t) (*((uint8_t*) (ptr + 4)));
      break;
    }
  }*ret0 = expected.val.ok;
  return expected.is_err ? expected.val.err : -1;
}
__attribute__((import_module("imports"), import_name("select-nodes")))
int64_t __wasm_import_imports_select_nodes(int32_t, int32_t, int32_t);
int64_t imports_select_nodes(imports_series_id_t *id) {
  int32_t variant;
  int32_t variant1;
  int32_t variant2;
  switch ((int32_t) (*id).tag) {
    case 0: {
      const imports_list_s64_t *payload = &(*id).val.i64;
      variant = 0;
      variant1 = (int32_t) (*payload).ptr;
      variant2 = (int32_t) (*payload).len;
      break;
    }
    case 1: {
      const imports_list_string_t *payload0 = &(*id).val.txt;
      variant = 1;
      variant1 = (int32_t) (*payload0).ptr;
      variant2 = (int32_t) (*payload0).len;
      break;
    }
  }
  int64_t ret = __wasm_import_imports_select_nodes(variant, variant1, variant2);
  return ret;
}
__attribute__((import_module("imports"), import_name("select-edges")))
int64_t __wasm_import_imports_select_edges(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
int64_t imports_select_edges(imports_series_id_t *src, imports_series_id_t *dst) {
  int32_t variant;
  int32_t variant1;
  int32_t variant2;
  switch ((int32_t) (*src).tag) {
    case 0: {
      const imports_list_s64_t *payload = &(*src).val.i64;
      variant = 0;
      variant1 = (int32_t) (*payload).ptr;
      variant2 = (int32_t) (*payload).len;
      break;
    }
    case 1: {
      const imports_list_string_t *payload0 = &(*src).val.txt;
      variant = 1;
      variant1 = (int32_t) (*payload0).ptr;
      variant2 = (int32_t) (*payload0).len;
      break;
    }
  }
  int32_t variant5;
  int32_t variant6;
  int32_t variant7;
  switch ((int32_t) (*dst).tag) {
    case 0: {
      const imports_list_s64_t *payload3 = &(*dst).val.i64;
      variant5 = 0;
      variant6 = (int32_t) (*payload3).ptr;
      variant7 = (int32_t) (*payload3).len;
      break;
    }
    case 1: {
      const imports_list_string_t *payload4 = &(*dst).val.txt;
      variant5 = 1;
      variant6 = (int32_t) (*payload4).ptr;
      variant7 = (int32_t) (*payload4).len;
      break;
    }
  }
  int64_t ret = __wasm_import_imports_select_edges(variant, variant1, variant2, variant5, variant6, variant7);
  return ret;
}
