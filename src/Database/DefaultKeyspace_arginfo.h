/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 29504c7f6b4cad0e7171ab51d6aed5a83d19419b */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultKeyspace_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultKeyspace_replicationClassName arginfo_class_Cassandra_DefaultKeyspace_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultKeyspace_replicationOptions, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultKeyspace_hasDurableWrites, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_DefaultKeyspace_table, 0, 1, Cassandra\\Table, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultKeyspace_tables arginfo_class_Cassandra_DefaultKeyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultKeyspace_userType, 0, 1, Cassandra\\Type\\\125serType, 1)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultKeyspace_userTypes arginfo_class_Cassandra_DefaultKeyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_DefaultKeyspace_materializedView, 0, 1, Cassandra\\MaterializedView, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultKeyspace_materializedViews arginfo_class_Cassandra_DefaultKeyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_DefaultKeyspace_function, 0, 1, Cassandra\\Function_, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, types, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultKeyspace_functions arginfo_class_Cassandra_DefaultKeyspace_replicationOptions

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_DefaultKeyspace_aggregate, 0, 1, Cassandra\\Aggregate, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, types, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultKeyspace_aggregates arginfo_class_Cassandra_DefaultKeyspace_replicationOptions

ZEND_METHOD(Cassandra_DefaultKeyspace, name);
ZEND_METHOD(Cassandra_DefaultKeyspace, replicationClassName);
ZEND_METHOD(Cassandra_DefaultKeyspace, replicationOptions);
ZEND_METHOD(Cassandra_DefaultKeyspace, hasDurableWrites);
ZEND_METHOD(Cassandra_DefaultKeyspace, table);
ZEND_METHOD(Cassandra_DefaultKeyspace, tables);
ZEND_METHOD(Cassandra_DefaultKeyspace, userType);
ZEND_METHOD(Cassandra_DefaultKeyspace, userTypes);
ZEND_METHOD(Cassandra_DefaultKeyspace, materializedView);
ZEND_METHOD(Cassandra_DefaultKeyspace, materializedViews);
ZEND_METHOD(Cassandra_DefaultKeyspace, function);
ZEND_METHOD(Cassandra_DefaultKeyspace, functions);
ZEND_METHOD(Cassandra_DefaultKeyspace, aggregate);
ZEND_METHOD(Cassandra_DefaultKeyspace, aggregates);

static const zend_function_entry class_Cassandra_DefaultKeyspace_methods[] = {
	ZEND_ME(Cassandra_DefaultKeyspace, name, arginfo_class_Cassandra_DefaultKeyspace_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, replicationClassName, arginfo_class_Cassandra_DefaultKeyspace_replicationClassName, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, replicationOptions, arginfo_class_Cassandra_DefaultKeyspace_replicationOptions, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, hasDurableWrites, arginfo_class_Cassandra_DefaultKeyspace_hasDurableWrites, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, table, arginfo_class_Cassandra_DefaultKeyspace_table, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, tables, arginfo_class_Cassandra_DefaultKeyspace_tables, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, userType, arginfo_class_Cassandra_DefaultKeyspace_userType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, userTypes, arginfo_class_Cassandra_DefaultKeyspace_userTypes, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, materializedView, arginfo_class_Cassandra_DefaultKeyspace_materializedView, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, materializedViews, arginfo_class_Cassandra_DefaultKeyspace_materializedViews, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, function, arginfo_class_Cassandra_DefaultKeyspace_function, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, functions, arginfo_class_Cassandra_DefaultKeyspace_functions, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, aggregate, arginfo_class_Cassandra_DefaultKeyspace_aggregate, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultKeyspace, aggregates, arginfo_class_Cassandra_DefaultKeyspace_aggregates, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultKeyspace(zend_class_entry *class_entry_Cassandra_Keyspace)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultKeyspace", class_Cassandra_DefaultKeyspace_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Keyspace);

	return class_entry;
}
