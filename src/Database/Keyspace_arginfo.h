/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: fadd4b2ee2a051e56871262423b2a5de1f88c92f */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Keyspace_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Keyspace_replicationClassName arginfo_class_Cassandra_Keyspace_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Keyspace_replicationOptions, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Keyspace_hasDurableWrites, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_Keyspace_table, 0, 1, Cassandra\\Table, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Keyspace_tables arginfo_class_Cassandra_Keyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Keyspace_userType, 0, 1, Cassandra\\Type\\\125serType, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Keyspace_userTypes arginfo_class_Cassandra_Keyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_Keyspace_materializedView, 0, 1, Cassandra\\MaterializedView, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Keyspace_materializedViews arginfo_class_Cassandra_Keyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_Keyspace_function, 0, 1, Cassandra\\Function_, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, types, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Keyspace_functions arginfo_class_Cassandra_Keyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_Keyspace_aggregate, 0, 1, Cassandra\\Aggregate, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, types, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Keyspace_aggregates arginfo_class_Cassandra_Keyspace_replicationOptions




static const zend_function_entry class_Cassandra_Keyspace_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, name, arginfo_class_Cassandra_Keyspace_name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, replicationClassName, arginfo_class_Cassandra_Keyspace_replicationClassName, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, replicationOptions, arginfo_class_Cassandra_Keyspace_replicationOptions, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, hasDurableWrites, arginfo_class_Cassandra_Keyspace_hasDurableWrites, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, table, arginfo_class_Cassandra_Keyspace_table, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, tables, arginfo_class_Cassandra_Keyspace_tables, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, userType, arginfo_class_Cassandra_Keyspace_userType, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, userTypes, arginfo_class_Cassandra_Keyspace_userTypes, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, materializedView, arginfo_class_Cassandra_Keyspace_materializedView, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, materializedViews, arginfo_class_Cassandra_Keyspace_materializedViews, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, function, arginfo_class_Cassandra_Keyspace_function, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, functions, arginfo_class_Cassandra_Keyspace_functions, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, aggregate, arginfo_class_Cassandra_Keyspace_aggregate, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Keyspace, aggregates, arginfo_class_Cassandra_Keyspace_aggregates, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Keyspace(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Keyspace", class_Cassandra_Keyspace_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
