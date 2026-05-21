/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 2c56bc2402f355fab7158dbaad16b17312a4decb */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_ExecutionOptions___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_ExecutionOptions___get, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_ExecutionOptions, __construct);
ZEND_METHOD(Cassandra_ExecutionOptions, __get);


static const zend_function_entry class_Cassandra_ExecutionOptions_methods[] = {
	ZEND_ME(Cassandra_ExecutionOptions, __construct, arginfo_class_Cassandra_ExecutionOptions___construct, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	ZEND_ME(Cassandra_ExecutionOptions, __get, arginfo_class_Cassandra_ExecutionOptions___get, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_ExecutionOptions(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "ExecutionOptions", class_Cassandra_ExecutionOptions_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
