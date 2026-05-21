/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 681b39244d863b54321c19d3dcfd27ad9e84c321 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UuidInterface_uuid, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_UuidInterface_version, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_UuidInterface_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_UuidInterface, uuid, arginfo_class_Cassandra_UuidInterface_uuid, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_UuidInterface, version, arginfo_class_Cassandra_UuidInterface_version, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_UuidInterface(zend_class_entry *class_entry_Cassandra_Value)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "UuidInterface", class_Cassandra_UuidInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Value);

	return class_entry;
}
