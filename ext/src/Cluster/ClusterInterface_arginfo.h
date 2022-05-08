/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3715e33527dca7c7056d4e37fa96f15a4d5c53fe */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Cluster_connect, 0, 0, Cassandra\\Session, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Cluster_connectAsync, 0, 0, Cassandra\\Future, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Cluster_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Cluster, connect, arginfo_class_Cassandra_Cluster_connect, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Cluster, connectAsync, arginfo_class_Cassandra_Cluster_connectAsync, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Cluster(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Cluster", class_Cassandra_Cluster_methods);
	class_entry = zend_register_internal_interface(&ce);
	class_entry->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
