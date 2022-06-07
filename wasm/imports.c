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

__attribute__((import_module("canonical_abi"), import_name("resource_drop_data-frame")))
void __resource_data_frame_drop(uint32_t idx);

void imports_data_frame_free(imports_data_frame_t *ptr) {
  __resource_data_frame_drop(ptr->idx);
}

__attribute__((import_module("canonical_abi"), import_name("resource_clone_data-frame")))
uint32_t __resource_data_frame_clone(uint32_t idx);

imports_data_frame_t imports_data_frame_clone(imports_data_frame_t *ptr) {
  return (imports_data_frame_t){__resource_data_frame_clone(ptr->idx)};
}

__attribute__((import_module("canonical_abi"), import_name("resource_drop_storage")))
void __resource_storage_drop(uint32_t idx);

void imports_storage_free(imports_storage_t *ptr) {
  __resource_storage_drop(ptr->idx);
}

__attribute__((import_module("canonical_abi"), import_name("resource_clone_storage")))
uint32_t __resource_storage_clone(uint32_t idx);

imports_storage_t imports_storage_clone(imports_storage_t *ptr) {
  return (imports_storage_t){__resource_storage_clone(ptr->idx)};
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
void imports_list_s64_free(imports_list_s64_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 8, 8);
}
void imports_list_float32_free(imports_list_float32_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 4, 4);
}
void imports_list_float64_free(imports_list_float64_t *ptr) {
  canonical_abi_free(ptr->ptr, ptr->len * 8, 8);
}
void imports_list_string_free(imports_list_string_t *ptr) {
  for (size_t i = 0; i < ptr->len; i++) {
    imports_string_free(&ptr->ptr[i]);
  }
  canonical_abi_free(ptr->ptr, ptr->len * 8, 4);
}
void imports_vector_free(imports_vector_t *ptr) {
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
void imports_item_free(imports_item_t *ptr) {
  imports_string_free(&ptr->key);
  imports_value_free(&ptr->val);
}
void imports_series_free(imports_series_t *ptr) {
  imports_string_free(&ptr->key);
  imports_vector_free(&ptr->val);
}
void imports_row_free(imports_row_t *ptr) {
  for (size_t i = 0; i < ptr->len; i++) {
    imports_item_free(&ptr->ptr[i]);
  }
  canonical_abi_free(ptr->ptr, ptr->len * 24, 8);
}
void imports_table_free(imports_table_t *ptr) {
  for (size_t i = 0; i < ptr->len; i++) {
    imports_series_free(&ptr->ptr[i]);
  }
  canonical_abi_free(ptr->ptr, ptr->len * 20, 4);
}
typedef struct {
  bool is_some;
  imports_data_frame_t val;
} imports_option_data_frame_t;
typedef struct {
  bool is_some;
} imports_option_empty_t;
typedef struct {
  bool is_some;
  uint64_t val;
} imports_option_u64_t;
typedef struct {
  bool is_some;
  imports_storage_t val;
} imports_option_storage_t;
typedef struct {
  bool is_some;
  imports_series_t val;
} imports_option_series_t;
typedef struct {
  bool is_some;
  imports_row_t val;
} imports_option_row_t;
typedef struct {
  bool is_some;
  imports_table_t val;
} imports_option_table_t;

__attribute__((aligned(8)))
static uint8_t RET_AREA[24];
__attribute__((import_module("imports"), import_name("data-frame::new")))
void __wasm_import_imports_data_frame_new(int32_t, int32_t, int32_t, int32_t, int32_t);
bool imports_data_frame_new(imports_string_t *name, imports_row_t *default, imports_data_frame_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_data_frame_new((int32_t) (*name).ptr, (int32_t) (*name).len, (int32_t) (*default).ptr, (int32_t) (*default).len, ptr);
  imports_option_data_frame_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      option.val = (imports_data_frame_t){ *((int32_t*) (ptr + 4)) };
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("data-frame::push")))
int32_t __wasm_import_imports_data_frame_push(int32_t, int32_t, int32_t);
bool imports_data_frame_push(imports_data_frame_t self, imports_row_t *data, imports_empty_t *ret0) {
  int32_t ret = __wasm_import_imports_data_frame_push((self).idx, (int32_t) (*data).ptr, (int32_t) (*data).len);
  imports_option_empty_t option;
  switch (ret) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("data-frame::size")))
void __wasm_import_imports_data_frame_size(int32_t, int32_t);
bool imports_data_frame_size(imports_data_frame_t self, uint64_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_data_frame_size((self).idx, ptr);
  imports_option_u64_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      option.val = (uint64_t) (*((int64_t*) (ptr + 8)));
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("storage::new")))
void __wasm_import_imports_storage_new(int32_t);
bool imports_storage_new(imports_storage_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_storage_new(ptr);
  imports_option_storage_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      option.val = (imports_storage_t){ *((int32_t*) (ptr + 4)) };
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("storage::choice-nodes")))
void __wasm_import_imports_storage_choice_nodes(int32_t, int32_t, int32_t, int32_t, int32_t);
bool imports_storage_choice_nodes(imports_storage_t self, imports_string_t *tag, int32_t number, imports_series_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_storage_choice_nodes((self).idx, (int32_t) (*tag).ptr, (int32_t) (*tag).len, number, ptr);
  imports_option_series_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      imports_vector_t variant;
      variant.tag = (int32_t) (*((uint8_t*) (ptr + 12)));
      switch ((int32_t) variant.tag) {
        case 0: {
          break;
        }
        case 1: {
          variant.val.bol = (imports_list_bool_t) { (bool*)(*((int32_t*) (ptr + 16))), (size_t)(*((int32_t*) (ptr + 20))) };
          break;
        }
        case 2: {
          variant.val.i32 = (imports_list_s32_t) { (int32_t*)(*((int32_t*) (ptr + 16))), (size_t)(*((int32_t*) (ptr + 20))) };
          break;
        }
        case 3: {
          variant.val.i64 = (imports_list_s64_t) { (int64_t*)(*((int32_t*) (ptr + 16))), (size_t)(*((int32_t*) (ptr + 20))) };
          break;
        }
        case 4: {
          variant.val.f32 = (imports_list_float32_t) { (float*)(*((int32_t*) (ptr + 16))), (size_t)(*((int32_t*) (ptr + 20))) };
          break;
        }
        case 5: {
          variant.val.f64 = (imports_list_float64_t) { (double*)(*((int32_t*) (ptr + 16))), (size_t)(*((int32_t*) (ptr + 20))) };
          break;
        }
        case 6: {
          variant.val.txt = (imports_list_string_t) { (imports_string_t*)(*((int32_t*) (ptr + 16))), (size_t)(*((int32_t*) (ptr + 20))) };
          break;
        }
      }
      
      option.val = (imports_series_t) {
        (imports_string_t) { (char*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) },
        variant,
      };
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("storage::query-node")))
void __wasm_import_imports_storage_query_node(int32_t, int32_t, int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
bool imports_storage_query_node(imports_storage_t self, imports_value_t *id, imports_string_t *tag, imports_list_string_t *keys, imports_row_t *ret0) {
  int32_t variant;
  int64_t variant6;
  int32_t variant7;
  switch ((int32_t) (*id).tag) {
    case 0: {
      variant = 0;
      variant6 = 0;
      variant7 = 0;
      break;
    }
    case 1: {
      const bool *payload0 = &(*id).val.bol;
      variant = 1;
      variant6 = (int64_t) *payload0;
      variant7 = 0;
      break;
    }
    case 2: {
      const int32_t *payload1 = &(*id).val.i32;
      variant = 2;
      variant6 = (int64_t) *payload1;
      variant7 = 0;
      break;
    }
    case 3: {
      const int64_t *payload2 = &(*id).val.i64;
      variant = 3;
      variant6 = *payload2;
      variant7 = 0;
      break;
    }
    case 4: {
      const float *payload3 = &(*id).val.f32;
      variant = 4;
      variant6 = ((union { float a; int32_t b; }){ *payload3 }).b;
      variant7 = 0;
      break;
    }
    case 5: {
      const double *payload4 = &(*id).val.f64;
      variant = 5;
      variant6 = ((union { double a; int64_t b; }){ *payload4 }).b;
      variant7 = 0;
      break;
    }
    case 6: {
      const imports_string_t *payload5 = &(*id).val.txt;
      variant = 6;
      variant6 = (int64_t) (int32_t) (*payload5).ptr;
      variant7 = (int32_t) (*payload5).len;
      break;
    }
  }
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_storage_query_node((self).idx, variant, variant6, variant7, (int32_t) (*tag).ptr, (int32_t) (*tag).len, (int32_t) (*keys).ptr, (int32_t) (*keys).len, ptr);
  imports_option_row_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      option.val = (imports_row_t) { (imports_item_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("storage::query-neighbors")))
void __wasm_import_imports_storage_query_neighbors(int32_t, int32_t, int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
bool imports_storage_query_neighbors(imports_storage_t self, imports_value_t *id, imports_string_t *tag, imports_list_string_t *keys, bool reversely, imports_table_t *ret0) {
  int32_t variant;
  int64_t variant6;
  int32_t variant7;
  switch ((int32_t) (*id).tag) {
    case 0: {
      variant = 0;
      variant6 = 0;
      variant7 = 0;
      break;
    }
    case 1: {
      const bool *payload0 = &(*id).val.bol;
      variant = 1;
      variant6 = (int64_t) *payload0;
      variant7 = 0;
      break;
    }
    case 2: {
      const int32_t *payload1 = &(*id).val.i32;
      variant = 2;
      variant6 = (int64_t) *payload1;
      variant7 = 0;
      break;
    }
    case 3: {
      const int64_t *payload2 = &(*id).val.i64;
      variant = 3;
      variant6 = *payload2;
      variant7 = 0;
      break;
    }
    case 4: {
      const float *payload3 = &(*id).val.f32;
      variant = 4;
      variant6 = ((union { float a; int32_t b; }){ *payload3 }).b;
      variant7 = 0;
      break;
    }
    case 5: {
      const double *payload4 = &(*id).val.f64;
      variant = 5;
      variant6 = ((union { double a; int64_t b; }){ *payload4 }).b;
      variant7 = 0;
      break;
    }
    case 6: {
      const imports_string_t *payload5 = &(*id).val.txt;
      variant = 6;
      variant6 = (int64_t) (int32_t) (*payload5).ptr;
      variant7 = (int32_t) (*payload5).len;
      break;
    }
  }
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_storage_query_neighbors((self).idx, variant, variant6, variant7, (int32_t) (*tag).ptr, (int32_t) (*tag).len, (int32_t) (*keys).ptr, (int32_t) (*keys).len, reversely, ptr);
  imports_option_table_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      option.val = (imports_table_t) { (imports_series_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("storage::query-kv")))
void __wasm_import_imports_storage_query_kv(int32_t, int32_t, int32_t, int32_t);
bool imports_storage_query_kv(imports_storage_t self, imports_row_t *data, imports_row_t *ret0) {
  int32_t ptr = (int32_t) &RET_AREA;
  __wasm_import_imports_storage_query_kv((self).idx, (int32_t) (*data).ptr, (int32_t) (*data).len, ptr);
  imports_option_row_t option;
  switch ((int32_t) (*((uint8_t*) (ptr + 0)))) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      option.val = (imports_row_t) { (imports_item_t*)(*((int32_t*) (ptr + 4))), (size_t)(*((int32_t*) (ptr + 8))) };
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("storage::update-kv")))
int32_t __wasm_import_imports_storage_update_kv(int32_t, int32_t, int32_t, int32_t);
bool imports_storage_update_kv(imports_storage_t self, imports_row_t *data, imports_merge_type_t ops, imports_empty_t *ret0) {
  int32_t ret = __wasm_import_imports_storage_update_kv((self).idx, (int32_t) (*data).ptr, (int32_t) (*data).len, (int32_t) ops);
  imports_option_empty_t option;
  switch (ret) {
    case 0: {
      option.is_some = false;
      
      break;
    }
    case 1: {
      option.is_some = true;
      
      
      break;
    }
  }*ret0 = option.val;
  return option.is_some;
}
__attribute__((import_module("imports"), import_name("log")))
void __wasm_import_imports_log(int32_t, int32_t, int32_t);
void imports_log(imports_log_level_t lv, imports_string_t *msg) {
  __wasm_import_imports_log((int32_t) lv, (int32_t) (*msg).ptr, (int32_t) (*msg).len);
}
__attribute__((import_module("imports"), import_name("log-enabled")))
void __wasm_import_imports_log_enabled(int32_t, int32_t, int32_t);
void imports_log_enabled(imports_log_level_t lv, imports_string_t *msg) {
  __wasm_import_imports_log_enabled((int32_t) lv, (int32_t) (*msg).ptr, (int32_t) (*msg).len);
}
