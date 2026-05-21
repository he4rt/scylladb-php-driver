/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: e5558517dcb5d9cb8d662497e1d45557d179e59f */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_Custom___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Custom_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_Custom___toString arginfo_class_Cassandra_Type_Custom_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Custom_create, 0, 0, IS_NEVER, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Type_Custom, __construct);
ZEND_METHOD(Cassandra_Type_Custom, name);
ZEND_METHOD(Cassandra_Type_Custom, __toString);
ZEND_METHOD(Cassandra_Type_Custom, create);


static const zend_function_entry class_Cassandra_Type_Custom_methods[] = {
	ZEND_ME(Cassandra_Type_Custom, __construct, arginfo_class_Cassandra_Type_Custom___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_Custom, name, arginfo_class_Cassandra_Type_Custom_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Custom, __toString, arginfo_class_Cassandra_Type_Custom___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Custom, create, arginfo_class_Cassandra_Type_Custom_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_Custom(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "Custom", class_Cassandra_Type_Custom_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Type);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
