/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 39d2c284db8451691a9874ab96955416fd59efbf */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Future_get, 0, 0, IS_MIXED, 0)
	ZEND_ARG_TYPE_MASK(0, timeout, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Future_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Future, get, arginfo_class_Cassandra_Future_get, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Future(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Future", class_Cassandra_Future_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
