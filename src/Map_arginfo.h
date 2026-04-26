/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9030dc774613d347072bbe680a8aadfd0100e72b */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Map___construct, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, keyType, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, valueType, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Map_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_keys, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Map_values arginfo_class_Cassandra_Map_keys

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_set, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_get, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_remove, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, key, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Map_has arginfo_class_Cassandra_Map_remove

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Map_key arginfo_class_Cassandra_Map_current

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_next, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Map_rewind arginfo_class_Cassandra_Map_next

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_offsetSet, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_offsetGet, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_offsetUnset, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Map_offsetExists, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_Map, __construct);
ZEND_METHOD(Cassandra_Map, type);
ZEND_METHOD(Cassandra_Map, keys);
ZEND_METHOD(Cassandra_Map, values);
ZEND_METHOD(Cassandra_Map, set);
ZEND_METHOD(Cassandra_Map, get);
ZEND_METHOD(Cassandra_Map, remove);
ZEND_METHOD(Cassandra_Map, has);
ZEND_METHOD(Cassandra_Map, count);
ZEND_METHOD(Cassandra_Map, current);
ZEND_METHOD(Cassandra_Map, key);
ZEND_METHOD(Cassandra_Map, next);
ZEND_METHOD(Cassandra_Map, valid);
ZEND_METHOD(Cassandra_Map, rewind);
ZEND_METHOD(Cassandra_Map, offsetSet);
ZEND_METHOD(Cassandra_Map, offsetGet);
ZEND_METHOD(Cassandra_Map, offsetUnset);
ZEND_METHOD(Cassandra_Map, offsetExists);

static const zend_function_entry class_Cassandra_Map_methods[] = {
	ZEND_ME(Cassandra_Map, __construct, arginfo_class_Cassandra_Map___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, type, arginfo_class_Cassandra_Map_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, keys, arginfo_class_Cassandra_Map_keys, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, values, arginfo_class_Cassandra_Map_values, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, set, arginfo_class_Cassandra_Map_set, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, get, arginfo_class_Cassandra_Map_get, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, remove, arginfo_class_Cassandra_Map_remove, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, has, arginfo_class_Cassandra_Map_has, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, count, arginfo_class_Cassandra_Map_count, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, current, arginfo_class_Cassandra_Map_current, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, key, arginfo_class_Cassandra_Map_key, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, next, arginfo_class_Cassandra_Map_next, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, valid, arginfo_class_Cassandra_Map_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, rewind, arginfo_class_Cassandra_Map_rewind, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, offsetSet, arginfo_class_Cassandra_Map_offsetSet, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, offsetGet, arginfo_class_Cassandra_Map_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, offsetUnset, arginfo_class_Cassandra_Map_offsetUnset, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Map, offsetExists, arginfo_class_Cassandra_Map_offsetExists, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Map(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_Iterator, zend_class_entry *class_entry_ArrayAccess)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Map", class_Cassandra_Map_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 4, class_entry_Cassandra_Value, class_entry_Countable, class_entry_Iterator, class_entry_ArrayAccess);

	return class_entry;
}
