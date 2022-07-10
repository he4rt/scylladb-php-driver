/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3404a2a39030d5324b1e45ad3cc528f0fd9ad013 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Cluster_ClusterInterface_connect, 0, 0, Cassandra\\Cluster\\Session, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Cluster_ClusterInterface_connectAsync, 0, 0, Cassandra\\Cluster\\Future, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Cluster_ClusterInterface_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Cluster_ClusterInterface, connect, arginfo_class_Cassandra_Cluster_ClusterInterface_connect, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Cluster_ClusterInterface, connectAsync, arginfo_class_Cassandra_Cluster_ClusterInterface_connectAsync, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Cluster_ClusterInterface(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Cluster", "ClusterInterface", class_Cassandra_Cluster_ClusterInterface_methods);
	class_entry = zend_register_internal_interface(&ce);
	class_entry->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
