/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 49a3eca68a113e9b92d1b620df183205e8bdf5b0 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_Scalar___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Scalar_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_Scalar___toString arginfo_class_Cassandra_Type_Scalar_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Scalar_create, 0, 0, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, value, IS_MIXED, 0, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_Type_Scalar, __construct);
ZEND_METHOD(Cassandra_Type_Scalar, name);
ZEND_METHOD(Cassandra_Type_Scalar, __toString);
ZEND_METHOD(Cassandra_Type_Scalar, create);

static const zend_function_entry class_Cassandra_Type_Scalar_methods[] = {
	ZEND_ME(Cassandra_Type_Scalar, __construct, arginfo_class_Cassandra_Type_Scalar___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_Scalar, name, arginfo_class_Cassandra_Type_Scalar_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Scalar, __toString, arginfo_class_Cassandra_Type_Scalar___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Scalar, create, arginfo_class_Cassandra_Type_Scalar_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_Scalar(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "Scalar", class_Cassandra_Type_Scalar_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Type, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);

	return class_entry;
}
