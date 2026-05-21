/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 4d0b235dd32686f192626e4ef598a8b347835afe */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_UserTypeValue___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, types, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_UserTypeValue_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_values, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_set, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_get, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_current, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_key, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_next, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UserTypeValue_valid, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_UserTypeValue_rewind arginfo_class_Cassandra_UserTypeValue_next


ZEND_METHOD(Cassandra_UserTypeValue, __construct);
ZEND_METHOD(Cassandra_UserTypeValue, type);
ZEND_METHOD(Cassandra_UserTypeValue, values);
ZEND_METHOD(Cassandra_UserTypeValue, set);
ZEND_METHOD(Cassandra_UserTypeValue, get);
ZEND_METHOD(Cassandra_UserTypeValue, count);
ZEND_METHOD(Cassandra_UserTypeValue, current);
ZEND_METHOD(Cassandra_UserTypeValue, key);
ZEND_METHOD(Cassandra_UserTypeValue, next);
ZEND_METHOD(Cassandra_UserTypeValue, valid);
ZEND_METHOD(Cassandra_UserTypeValue, rewind);


static const zend_function_entry class_Cassandra_UserTypeValue_methods[] = {
	ZEND_ME(Cassandra_UserTypeValue, __construct, arginfo_class_Cassandra_UserTypeValue___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, type, arginfo_class_Cassandra_UserTypeValue_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, values, arginfo_class_Cassandra_UserTypeValue_values, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, set, arginfo_class_Cassandra_UserTypeValue_set, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, get, arginfo_class_Cassandra_UserTypeValue_get, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, count, arginfo_class_Cassandra_UserTypeValue_count, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, current, arginfo_class_Cassandra_UserTypeValue_current, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, key, arginfo_class_Cassandra_UserTypeValue_key, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, next, arginfo_class_Cassandra_UserTypeValue_next, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, valid, arginfo_class_Cassandra_UserTypeValue_valid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_UserTypeValue, rewind, arginfo_class_Cassandra_UserTypeValue_rewind, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_UserTypeValue(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_Iterator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "UserTypeValue", class_Cassandra_UserTypeValue_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 3, class_entry_Cassandra_Value, class_entry_Countable, class_entry_Iterator);

	return class_entry;
}
