#pragma once

/**
 * ZendCPP Examples and Patterns
 *
 * This file contains practical examples of using ZendCPP in real-world scenarios.
 * These patterns can be copy-pasted and adapted for your extension.
 */

#include <ZendCPP/ZendCPP.hpp>

namespace ZendCPP::Examples {

// ============================================================================
// Example 1: Simple Value Object
// ============================================================================

struct Point {
    double x;
    double y;
    zend_object zendObject;
};

// Free handler using ZendCPP
inline void point_free(zend_object* obj) {
    auto* point = ObjectFetch<Point>(obj);
    zend_object_std_dtor(&point->zendObject);
}

// Create handler using ZendCPP
inline zend_object* point_create(zend_class_entry* ce, zend_object_handlers* handlers) {
    auto* point = Allocate<Point>(ce, handlers);
    point->x = 0.0;
    point->y = 0.0;
    return &point->zendObject;
}

// Constructor using modern helpers
inline void point_construct(INTERNAL_FUNCTION_PARAMETERS) {
    double x, y;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("dd", &x, &y);

    auto* point = ObjectFetch<Point>(Z_OBJ_P(ZEND_THIS));
    point->x = x;
    point->y = y;
}

// Method with return value helper
inline void point_distance(INTERNAL_FUNCTION_PARAMETERS) {
    auto* point1 = ObjectFetch<Point>(Z_OBJ_P(ZEND_THIS));

    zval* other;
    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("O", &other);

    auto* point2 = ObjectFetch<Point>(Z_OBJ_P(other));

    double dx = point1->x - point2->x;
    double dy = point1->y - point2->y;
    double distance = sqrt(dx * dx + dy * dy);

    ReturnValue(return_value).SetDouble(distance);
}

// ============================================================================
// Example 2: Container Object with Dynamic Data
// ============================================================================

struct Container {
    HashTable* items;
    zend_long capacity;
    zend_object zendObject;
};

inline void container_free(zend_object* obj) {
    auto* container = ObjectFetch<Container>(obj);

    if (container->items) {
        zend_hash_destroy(container->items);
        FREE_HASHTABLE(container->items);
    }

    zend_object_std_dtor(&container->zendObject);
}

inline zend_object* container_create(zend_class_entry* ce, zend_object_handlers* handlers) {
    auto* container = Allocate<Container>(ce, handlers);
    container->capacity = 100;

    ALLOC_HASHTABLE(container->items);
    zend_hash_init(container->items, 0, nullptr, ZVAL_PTR_DTOR, 0);

    return &container->zendObject;
}

inline void container_add(INTERNAL_FUNCTION_PARAMETERS) {
    auto* container = ObjectFetch<Container>(Z_OBJ_P(ZEND_THIS));

    char* key;
    size_t key_len;
    zval* value;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("sz", &key, &key_len, &value);

    if (zend_hash_num_elements(container->items) >= container->capacity) {
        Exception::ThrowRuntimeError("Container is full");
        return;
    }

    ZendCPP::HashTable ht(container->items);
    Z_TRY_ADDREF_P(value);
    ht.Set(key, value);

    ReturnValue(return_value).SetBool(true);
}

inline void container_get(INTERNAL_FUNCTION_PARAMETERS) {
    auto* container = ObjectFetch<Container>(Z_OBJ_P(ZEND_THIS));

    char* key;
    size_t key_len;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("s", &key, &key_len);

    ZendCPP::HashTable ht(container->items);
    zval* value = ht.Get(key);

    if (!value) {
        ReturnValue(return_value).SetNull();
        return;
    }

    ReturnValue(return_value).SetCopy(value);
}

// ============================================================================
// Example 3: Builder Pattern
// ============================================================================

struct QueryBuilder {
    ZendCPP::StringBuilder* query;
    bool has_where;
    zend_object zendObject;
};

inline void query_builder_free(zend_object* obj) {
    auto* qb = ObjectFetch<QueryBuilder>(obj);

    if (qb->query) {
        delete qb->query;
    }

    zend_object_std_dtor(&qb->zendObject);
}

inline zend_object* query_builder_create(zend_class_entry* ce, zend_object_handlers* handlers) {
    auto* qb = Allocate<QueryBuilder>(ce, handlers);
    qb->query = new StringBuilder(256);
    qb->has_where = false;
    return &qb->zendObject;
}

inline void query_builder_select(INTERNAL_FUNCTION_PARAMETERS) {
    auto* qb = ObjectFetch<QueryBuilder>(Z_OBJ_P(ZEND_THIS));

    char* table;
    size_t table_len;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("s", &table, &table_len);

    qb->query->Append("SELECT * FROM ").Append(table, table_len);

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

inline void query_builder_where(INTERNAL_FUNCTION_PARAMETERS) {
    auto* qb = ObjectFetch<QueryBuilder>(Z_OBJ_P(ZEND_THIS));

    char* condition;
    size_t condition_len;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("s", &condition, &condition_len);

    if (!qb->has_where) {
        qb->query->Append(" WHERE ");
        qb->has_where = true;
    } else {
        qb->query->Append(" AND ");
    }

    qb->query->Append(condition, condition_len);

    RETURN_ZVAL(ZEND_THIS, 1, 0);
}

inline void query_builder_build(INTERNAL_FUNCTION_PARAMETERS) {
    auto* qb = ObjectFetch<QueryBuilder>(Z_OBJ_P(ZEND_THIS));

    // Create a copy of the query
    StringBuilder copy(qb->query->Length());
    copy << *qb->query;

    ReturnValue(return_value).SetString(copy.Build());
}

// ============================================================================
// Example 4: Exception-Safe Resource Management
// ============================================================================

struct FileHandle {
    FILE* fp;
    zend_string* path;
    zend_object zendObject;
};

inline void file_handle_free(zend_object* obj) {
    auto* fh = ObjectFetch<FileHandle>(obj);

    if (fh->fp) {
        fclose(fh->fp);
        fh->fp = nullptr;
    }

    if (fh->path) {
        zend_string_release(fh->path);
    }

    zend_object_std_dtor(&fh->zendObject);
}

inline void file_handle_open(INTERNAL_FUNCTION_PARAMETERS) {
    auto* fh = ObjectFetch<FileHandle>(Z_OBJ_P(ZEND_THIS));

    char* path;
    size_t path_len;
    char* mode;
    size_t mode_len;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("ss", &path, &path_len, &mode, &mode_len);

    // Use exception guard for error handling
    ExceptionGuard::Call([&]() {
        if (fh->fp) {
            throw std::runtime_error("File already open");
        }

        fh->fp = fopen(path, mode);
        if (!fh->fp) {
            throw std::runtime_error("Failed to open file");
        }

        fh->path = zend_string_init(path, path_len, 0);
    }, "FileHandle::open");
}

inline void file_handle_read_line(INTERNAL_FUNCTION_PARAMETERS) {
    auto* fh = ObjectFetch<FileHandle>(Z_OBJ_P(ZEND_THIS));

    if (!fh->fp) {
        Exception::ThrowRuntimeError("File not open");
        return;
    }

    char buffer[8192];
    if (fgets(buffer, sizeof(buffer), fh->fp)) {
        ReturnValue(return_value).SetString(buffer);
    } else {
        ReturnValue(return_value).SetFalse();
    }
}

// ============================================================================
// Example 5: Converting Between PHP and C++ Types
// ============================================================================

inline void array_sum_example(INTERNAL_FUNCTION_PARAMETERS) {
    zval* arr;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("a", &arr);

    double sum = 0.0;

    // Iterate over array
    zend_ulong idx;
    zend_string* key;
    zval* val;

    ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(arr), idx, key, val) {
        if (Z_TYPE_P(val) == IS_LONG) {
            sum += Z_LVAL_P(val);
        } else if (Z_TYPE_P(val) == IS_DOUBLE) {
            sum += Z_DVAL_P(val);
        }
    } ZEND_HASH_FOREACH_END();

    ReturnValue(return_value).SetDouble(sum);
}

// ============================================================================
// Example 6: Working with Callbacks
// ============================================================================

inline void array_map_example(INTERNAL_FUNCTION_PARAMETERS) {
    zval* callback;
    zval* arr;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("za", &callback, &arr);

    ZendCPP::Array result(zend_hash_num_elements(Z_ARRVAL_P(arr)));

    zval* val;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(arr), val) {
        zval retval;
        zval params[1];
        ZVAL_COPY(&params[0], val);

        if (call_user_function(CG(function_table), nullptr, callback, &retval, 1, params) == SUCCESS) {
            result.Append(&retval);
        }

        zval_ptr_dtor(&params[0]);
    } ZEND_HASH_FOREACH_END();

    ZVAL_ARR(return_value, result.Release());
}

// ============================================================================
// Example 7: Property Access Pattern
// ============================================================================

inline void property_demo(INTERNAL_FUNCTION_PARAMETERS) {
    zval* obj;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("o", &obj);

    PropertyReader props(obj);

    // Read properties
    zval* name = props.Read("name");
    zval* age = props.Read("age");

    // Write properties
    props.WriteString("status", "active");
    props.WriteLong("lastAccess", (zend_long)time(nullptr));
    props.WriteBool("verified", true);

    // Build result
    StringBuilder sb;
    sb << "Name: " << Z_STRVAL_P(name)
       << ", Age: " << Z_LVAL_P(age)
       << ", Status: active";

    ReturnValue(return_value).SetString(sb.Build());
}

// ============================================================================
// Example 8: Multi-type Return Values
// ============================================================================

inline void multi_return_example(INTERNAL_FUNCTION_PARAMETERS) {
    char* type;
    size_t type_len;

    ZENDCPP_PARSE_PARAMETERS_OR_RETURN("s", &type, &type_len);

    ReturnValue rv(return_value);

    if (strcmp(type, "array") == 0) {
        ZendCPP::Array arr;
        arr.AppendString("item1");
        arr.AppendLong(42);
        rv.SetArray(arr.Release());
    } else if (strcmp(type, "object") == 0) {
        object_init(return_value);
        PropertyReader props(return_value);
        props.WriteString("type", "demo");
    } else if (strcmp(type, "string") == 0) {
        rv.SetString("Demo string");
    } else {
        rv.SetNull();
    }
}

} // namespace ZendCPP::Examples
