/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8d43c9be9c981b870a2f52519124e3a932a10610 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Set___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, type, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Set_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Set_values, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Set_add, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Set_has arginfo_class_Cassandra_Set_add

#define arginfo_class_Cassandra_Set_remove arginfo_class_Cassandra_Set_add

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Set_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Set_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Set_key arginfo_class_Cassandra_Set_current

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Set_next, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Set_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Set_rewind arginfo_class_Cassandra_Set_next

ZEND_METHOD(Cassandra_Set, __construct);
ZEND_METHOD(Cassandra_Set, type);
ZEND_METHOD(Cassandra_Set, values);
ZEND_METHOD(Cassandra_Set, add);
ZEND_METHOD(Cassandra_Set, has);
ZEND_METHOD(Cassandra_Set, remove);
ZEND_METHOD(Cassandra_Set, count);
ZEND_METHOD(Cassandra_Set, current);
ZEND_METHOD(Cassandra_Set, key);
ZEND_METHOD(Cassandra_Set, next);
ZEND_METHOD(Cassandra_Set, valid);
ZEND_METHOD(Cassandra_Set, rewind);

static const zend_function_entry class_Cassandra_Set_methods[] = {
	ZEND_ME(Cassandra_Set, __construct, arginfo_class_Cassandra_Set___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, type, arginfo_class_Cassandra_Set_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, values, arginfo_class_Cassandra_Set_values, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, add, arginfo_class_Cassandra_Set_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, has, arginfo_class_Cassandra_Set_has, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, remove, arginfo_class_Cassandra_Set_remove, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, count, arginfo_class_Cassandra_Set_count, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, current, arginfo_class_Cassandra_Set_current, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, key, arginfo_class_Cassandra_Set_key, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, next, arginfo_class_Cassandra_Set_next, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, valid, arginfo_class_Cassandra_Set_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Set, rewind, arginfo_class_Cassandra_Set_rewind, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Set(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_Iterator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Set", class_Cassandra_Set_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 3, class_entry_Cassandra_Value, class_entry_Countable, class_entry_Iterator);

	return class_entry;
}
