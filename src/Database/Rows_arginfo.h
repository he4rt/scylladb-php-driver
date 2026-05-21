/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 72600cbe83a3c182b26b763b58937ee7be0ac712 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Rows___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_rewind, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Rows_key arginfo_class_Cassandra_Rows_current

#define arginfo_class_Cassandra_Rows_next arginfo_class_Cassandra_Rows_rewind

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_offsetExists, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_offsetGet, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_offsetSet, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_offsetUnset, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Rows_isLastPage arginfo_class_Cassandra_Rows_valid

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_Rows_nextPage, 0, 0, Cassandra\\Rows, MAY_BE_FALSE)
	ZEND_ARG_TYPE_MASK(0, timeout, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Rows_nextPageAsync, 0, 0, Cassandra\\Future, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Rows_pagingStateToken, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Rows_first arginfo_class_Cassandra_Rows_current


ZEND_METHOD(Cassandra_Rows, __construct);
ZEND_METHOD(Cassandra_Rows, count);
ZEND_METHOD(Cassandra_Rows, rewind);
ZEND_METHOD(Cassandra_Rows, current);
ZEND_METHOD(Cassandra_Rows, key);
ZEND_METHOD(Cassandra_Rows, next);
ZEND_METHOD(Cassandra_Rows, valid);
ZEND_METHOD(Cassandra_Rows, offsetExists);
ZEND_METHOD(Cassandra_Rows, offsetGet);
ZEND_METHOD(Cassandra_Rows, offsetSet);
ZEND_METHOD(Cassandra_Rows, offsetUnset);
ZEND_METHOD(Cassandra_Rows, isLastPage);
ZEND_METHOD(Cassandra_Rows, nextPage);
ZEND_METHOD(Cassandra_Rows, nextPageAsync);
ZEND_METHOD(Cassandra_Rows, pagingStateToken);
ZEND_METHOD(Cassandra_Rows, first);


static const zend_function_entry class_Cassandra_Rows_methods[] = {
	ZEND_ME(Cassandra_Rows, __construct, arginfo_class_Cassandra_Rows___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, count, arginfo_class_Cassandra_Rows_count, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, rewind, arginfo_class_Cassandra_Rows_rewind, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, current, arginfo_class_Cassandra_Rows_current, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, key, arginfo_class_Cassandra_Rows_key, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, next, arginfo_class_Cassandra_Rows_next, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, valid, arginfo_class_Cassandra_Rows_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, offsetExists, arginfo_class_Cassandra_Rows_offsetExists, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, offsetGet, arginfo_class_Cassandra_Rows_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, offsetSet, arginfo_class_Cassandra_Rows_offsetSet, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, offsetUnset, arginfo_class_Cassandra_Rows_offsetUnset, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, isLastPage, arginfo_class_Cassandra_Rows_isLastPage, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, nextPage, arginfo_class_Cassandra_Rows_nextPage, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, nextPageAsync, arginfo_class_Cassandra_Rows_nextPageAsync, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, pagingStateToken, arginfo_class_Cassandra_Rows_pagingStateToken, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Rows, first, arginfo_class_Cassandra_Rows_first, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Rows(zend_class_entry *class_entry_Iterator, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_ArrayAccess)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Rows", class_Cassandra_Rows_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 3, class_entry_Iterator, class_entry_Countable, class_entry_ArrayAccess);

	return class_entry;
}
