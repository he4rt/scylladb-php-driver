/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: efd9d17bf2a2030f788c8b18943e6067136df9fb */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Value_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Value_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Value, type, arginfo_class_Cassandra_Value_type, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Value(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Value", class_Cassandra_Value_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
