/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 0e04740908c7a0d8f6ff18fb8ff745d5b1685f4e */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_Tuple___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Tuple_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_Tuple___toString arginfo_class_Cassandra_Type_Tuple_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Tuple_types, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Tuple_create, 0, 0, Cassandra\\Tuple, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, values, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Type_Tuple, __construct);
ZEND_METHOD(Cassandra_Type_Tuple, name);
ZEND_METHOD(Cassandra_Type_Tuple, __toString);
ZEND_METHOD(Cassandra_Type_Tuple, types);
ZEND_METHOD(Cassandra_Type_Tuple, create);


static const zend_function_entry class_Cassandra_Type_Tuple_methods[] = {
	ZEND_ME(Cassandra_Type_Tuple, __construct, arginfo_class_Cassandra_Type_Tuple___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_Tuple, name, arginfo_class_Cassandra_Type_Tuple_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Tuple, __toString, arginfo_class_Cassandra_Type_Tuple___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Tuple, types, arginfo_class_Cassandra_Type_Tuple_types, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Tuple, create, arginfo_class_Cassandra_Type_Tuple_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_Tuple(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "Tuple", class_Cassandra_Type_Tuple_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Type);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
