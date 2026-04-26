/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a37cae88c0e44f66c5a9a6264f953d7bde0dc290 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_Schema_keyspace, 0, 1, Cassandra\\Keyspace, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Schema_keyspaces, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()


static const zend_function_entry class_Cassandra_Schema_methods[] = {
	ZEND_RAW_FENTRY("keyspace", NULL, arginfo_class_Cassandra_Schema_keyspace, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("keyspaces", NULL, arginfo_class_Cassandra_Schema_keyspaces, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Schema(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Schema", class_Cassandra_Schema_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
