/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 52316df3ddc64b19fc102a36c39a1d6b85843014 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Tuple___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, types, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Tuple_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_values, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_set, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_get, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Tuple_key arginfo_class_Cassandra_Tuple_count

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_next, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tuple_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Tuple_rewind arginfo_class_Cassandra_Tuple_next


ZEND_METHOD(Cassandra_Tuple, __construct);
ZEND_METHOD(Cassandra_Tuple, type);
ZEND_METHOD(Cassandra_Tuple, values);
ZEND_METHOD(Cassandra_Tuple, set);
ZEND_METHOD(Cassandra_Tuple, get);
ZEND_METHOD(Cassandra_Tuple, count);
ZEND_METHOD(Cassandra_Tuple, current);
ZEND_METHOD(Cassandra_Tuple, key);
ZEND_METHOD(Cassandra_Tuple, next);
ZEND_METHOD(Cassandra_Tuple, valid);
ZEND_METHOD(Cassandra_Tuple, rewind);


static const zend_function_entry class_Cassandra_Tuple_methods[] = {
	ZEND_ME(Cassandra_Tuple, __construct, arginfo_class_Cassandra_Tuple___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, type, arginfo_class_Cassandra_Tuple_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, values, arginfo_class_Cassandra_Tuple_values, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, set, arginfo_class_Cassandra_Tuple_set, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, get, arginfo_class_Cassandra_Tuple_get, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, count, arginfo_class_Cassandra_Tuple_count, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, current, arginfo_class_Cassandra_Tuple_current, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, key, arginfo_class_Cassandra_Tuple_key, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, next, arginfo_class_Cassandra_Tuple_next, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, valid, arginfo_class_Cassandra_Tuple_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tuple, rewind, arginfo_class_Cassandra_Tuple_rewind, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Tuple(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_Iterator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Tuple", class_Cassandra_Tuple_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 3, class_entry_Cassandra_Value, class_entry_Countable, class_entry_Iterator);

	return class_entry;
}
