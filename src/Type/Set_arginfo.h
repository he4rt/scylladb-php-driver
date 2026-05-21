/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: ec346d75a7d5f8790f3a7f2ad0d35c28cf04990a */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_Set___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Set_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Set_valueType, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_Set___toString arginfo_class_Cassandra_Type_Set_name

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Set_create, 0, 0, Cassandra\\Set, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Type_Set, __construct);
ZEND_METHOD(Cassandra_Type_Set, name);
ZEND_METHOD(Cassandra_Type_Set, valueType);
ZEND_METHOD(Cassandra_Type_Set, __toString);
ZEND_METHOD(Cassandra_Type_Set, create);


static const zend_function_entry class_Cassandra_Type_Set_methods[] = {
	ZEND_ME(Cassandra_Type_Set, __construct, arginfo_class_Cassandra_Type_Set___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_Set, name, arginfo_class_Cassandra_Type_Set_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Set, valueType, arginfo_class_Cassandra_Type_Set_valueType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Set, __toString, arginfo_class_Cassandra_Type_Set___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Set, create, arginfo_class_Cassandra_Type_Set_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_Set(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "Set", class_Cassandra_Type_Set_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Type);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
