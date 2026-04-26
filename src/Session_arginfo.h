/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 33f5bee0711115cace072ecc520d470d3cb4f0f4 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Session_execute, 0, 1, Cassandra\\Rows, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, statement, Cassandra\\Statement, MAY_BE_STRING, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Session_executeAsync, 0, 1, Cassandra\\FutureRows, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, statement, Cassandra\\Statement, MAY_BE_STRING, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Session_prepare, 0, 1, Cassandra\\PreparedStatement, 0)
	ZEND_ARG_TYPE_INFO(0, cql, IS_STRING, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Session_prepareAsync, 0, 1, Cassandra\\FuturePreparedStatement, 0)
	ZEND_ARG_TYPE_INFO(0, cql, IS_STRING, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Session_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_MASK(0, timeout, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Session_closeAsync, 0, 0, Cassandra\\FutureClose, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Session_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Session_schema, 0, 0, Cassandra\\Schema, 0)
ZEND_END_ARG_INFO()


static const zend_function_entry class_Cassandra_Session_methods[] = {
	ZEND_RAW_FENTRY("execute", NULL, arginfo_class_Cassandra_Session_execute, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("executeAsync", NULL, arginfo_class_Cassandra_Session_executeAsync, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("prepare", NULL, arginfo_class_Cassandra_Session_prepare, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("prepareAsync", NULL, arginfo_class_Cassandra_Session_prepareAsync, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("close", NULL, arginfo_class_Cassandra_Session_close, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("closeAsync", NULL, arginfo_class_Cassandra_Session_closeAsync, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("metrics", NULL, arginfo_class_Cassandra_Session_metrics, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("schema", NULL, arginfo_class_Cassandra_Session_schema, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Session(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Session", class_Cassandra_Session_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
