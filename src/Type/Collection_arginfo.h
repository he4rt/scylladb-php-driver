/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 98a7c33aaf6ef36f0ac568513cbf8fe3e1c5f88d */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_Collection___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Collection_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Collection_valueType, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_Collection___toString arginfo_class_Cassandra_Type_Collection_name

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Collection_create, 0, 0, Cassandra\\Collection, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Type_Collection, __construct);
ZEND_METHOD(Cassandra_Type_Collection, name);
ZEND_METHOD(Cassandra_Type_Collection, valueType);
ZEND_METHOD(Cassandra_Type_Collection, __toString);
ZEND_METHOD(Cassandra_Type_Collection, create);


static const zend_function_entry class_Cassandra_Type_Collection_methods[] = {
	ZEND_ME(Cassandra_Type_Collection, __construct, arginfo_class_Cassandra_Type_Collection___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_Collection, name, arginfo_class_Cassandra_Type_Collection_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Collection, valueType, arginfo_class_Cassandra_Type_Collection_valueType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Collection, __toString, arginfo_class_Cassandra_Type_Collection___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Collection, create, arginfo_class_Cassandra_Type_Collection_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_Collection(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "Collection", class_Cassandra_Type_Collection_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Type);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
