/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 0dd3b09bd4e7156a607c383b8b84711d03f92ea5 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Collection___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, type, Cassandra\\Type, MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_values, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_add, 0, 0, IS_LONG, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_remove, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, idx, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_get, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, idx, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_find, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Collection_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_next, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Collection_key arginfo_class_Cassandra_Collection_current

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Collection_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Collection_rewind arginfo_class_Cassandra_Collection_next


ZEND_METHOD(Cassandra_Collection, __construct);
ZEND_METHOD(Cassandra_Collection, values);
ZEND_METHOD(Cassandra_Collection, add);
ZEND_METHOD(Cassandra_Collection, remove);
ZEND_METHOD(Cassandra_Collection, get);
ZEND_METHOD(Cassandra_Collection, find);
ZEND_METHOD(Cassandra_Collection, type);
ZEND_METHOD(Cassandra_Collection, count);
ZEND_METHOD(Cassandra_Collection, current);
ZEND_METHOD(Cassandra_Collection, next);
ZEND_METHOD(Cassandra_Collection, key);
ZEND_METHOD(Cassandra_Collection, valid);
ZEND_METHOD(Cassandra_Collection, rewind);


static const zend_function_entry class_Cassandra_Collection_methods[] = {
	ZEND_ME(Cassandra_Collection, __construct, arginfo_class_Cassandra_Collection___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, values, arginfo_class_Cassandra_Collection_values, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, add, arginfo_class_Cassandra_Collection_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, remove, arginfo_class_Cassandra_Collection_remove, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, get, arginfo_class_Cassandra_Collection_get, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, find, arginfo_class_Cassandra_Collection_find, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, type, arginfo_class_Cassandra_Collection_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, count, arginfo_class_Cassandra_Collection_count, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, current, arginfo_class_Cassandra_Collection_current, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, next, arginfo_class_Cassandra_Collection_next, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, key, arginfo_class_Cassandra_Collection_key, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, valid, arginfo_class_Cassandra_Collection_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Collection, rewind, arginfo_class_Cassandra_Collection_rewind, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Collection(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_Iterator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Collection", class_Cassandra_Collection_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 3, class_entry_Cassandra_Value, class_entry_Countable, class_entry_Iterator);

	return class_entry;
}
