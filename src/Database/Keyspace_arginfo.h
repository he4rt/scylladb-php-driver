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
	ZEND_RAW_FENTRY("name", NULL, arginfo_class_Cassandra_Keyspace_name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("replicationClassName", NULL, arginfo_class_Cassandra_Keyspace_replicationClassName, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("replicationOptions", NULL, arginfo_class_Cassandra_Keyspace_replicationOptions, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("hasDurableWrites", NULL, arginfo_class_Cassandra_Keyspace_hasDurableWrites, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("table", NULL, arginfo_class_Cassandra_Keyspace_table, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("tables", NULL, arginfo_class_Cassandra_Keyspace_tables, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("userType", NULL, arginfo_class_Cassandra_Keyspace_userType, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("userTypes", NULL, arginfo_class_Cassandra_Keyspace_userTypes, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("materializedView", NULL, arginfo_class_Cassandra_Keyspace_materializedView, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("materializedViews", NULL, arginfo_class_Cassandra_Keyspace_materializedViews, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("function", NULL, arginfo_class_Cassandra_Keyspace_function, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("functions", NULL, arginfo_class_Cassandra_Keyspace_functions, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("aggregate", NULL, arginfo_class_Cassandra_Keyspace_aggregate, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("aggregates", NULL, arginfo_class_Cassandra_Keyspace_aggregates, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Keyspace(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Keyspace", class_Cassandra_Keyspace_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
