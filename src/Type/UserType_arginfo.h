/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f1bdf4e9044cde6a2445e077c3ab35ddda68e1e7 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_UserType___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_UserType_withName, 0, 1, Cassandra\\Type\\\125serType, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_UserType_name, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_UserType_withKeyspace, 0, 1, Cassandra\\Type\\\125serType, 0)
	ZEND_ARG_TYPE_INFO(0, keyspace, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_UserType_keyspace arginfo_class_Cassandra_Type_UserType_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_UserType___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_UserType_types, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_UserType_create, 0, 0, Cassandra\\\125serTypeValue, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_Type_UserType, __construct);
ZEND_METHOD(Cassandra_Type_UserType, withName);
ZEND_METHOD(Cassandra_Type_UserType, name);
ZEND_METHOD(Cassandra_Type_UserType, withKeyspace);
ZEND_METHOD(Cassandra_Type_UserType, keyspace);
ZEND_METHOD(Cassandra_Type_UserType, __toString);
ZEND_METHOD(Cassandra_Type_UserType, types);
ZEND_METHOD(Cassandra_Type_UserType, create);

static const zend_function_entry class_Cassandra_Type_UserType_methods[] = {
	ZEND_ME(Cassandra_Type_UserType, __construct, arginfo_class_Cassandra_Type_UserType___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_UserType, withName, arginfo_class_Cassandra_Type_UserType_withName, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_UserType, name, arginfo_class_Cassandra_Type_UserType_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_UserType, withKeyspace, arginfo_class_Cassandra_Type_UserType_withKeyspace, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_UserType, keyspace, arginfo_class_Cassandra_Type_UserType_keyspace, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_UserType, __toString, arginfo_class_Cassandra_Type_UserType___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_UserType, types, arginfo_class_Cassandra_Type_UserType_types, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_UserType, create, arginfo_class_Cassandra_Type_UserType_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_UserType(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "UserType", class_Cassandra_Type_UserType_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Type, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);

	return class_entry;
}
