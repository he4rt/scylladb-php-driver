/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a415f09f225d364be8d781bb7e1cb7ff2d4a1521 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Cluster_DefaultCluster_connect, 0, 0, Cassandra\\Cluster\\Session, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Cluster_DefaultCluster_connectAsync, 0, 0, Cassandra\\Cluster\\Future, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Cluster_DefaultCluster, connect);
ZEND_METHOD(Cassandra_Cluster_DefaultCluster, connectAsync);


static const zend_function_entry class_Cassandra_Cluster_DefaultCluster_methods[] = {
	ZEND_ME(Cassandra_Cluster_DefaultCluster, connect, arginfo_class_Cassandra_Cluster_DefaultCluster_connect, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Cluster_DefaultCluster, connectAsync, arginfo_class_Cassandra_Cluster_DefaultCluster_connectAsync, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Cluster_DefaultCluster(zend_class_entry *class_entry_Cassandra_Cluster_Cluster)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Cluster", "DefaultCluster", class_Cassandra_Cluster_DefaultCluster_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Cluster_Cluster);

	return class_entry;
}
