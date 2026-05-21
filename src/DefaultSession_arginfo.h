/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9dd449e05cfe53fdc3657df1389fdad0dfae5157 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultSession_execute, 0, 1, Cassandra\\Rows, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, statement, Cassandra\\Statement, MAY_BE_STRING, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultSession_executeAsync, 0, 1, Cassandra\\FutureRows, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, statement, Cassandra\\Statement, MAY_BE_STRING, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultSession_prepare, 0, 1, Cassandra\\PreparedStatement, 0)
	ZEND_ARG_TYPE_INFO(0, cql, IS_STRING, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultSession_prepareAsync, 0, 1, Cassandra\\FuturePreparedStatement, 0)
	ZEND_ARG_TYPE_INFO(0, cql, IS_STRING, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, options, Cassandra\\ExecutionOptions, MAY_BE_ARRAY|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultSession_close, 0, 0, IS_VOID, 0)
	ZEND_ARG_TYPE_MASK(0, timeout, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_NULL, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultSession_closeAsync, 0, 0, Cassandra\\FutureClose, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultSession_metrics, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultSession_schema, 0, 0, Cassandra\\Schema, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_DefaultSession, execute);
ZEND_METHOD(Cassandra_DefaultSession, executeAsync);
ZEND_METHOD(Cassandra_DefaultSession, prepare);
ZEND_METHOD(Cassandra_DefaultSession, prepareAsync);
ZEND_METHOD(Cassandra_DefaultSession, close);
ZEND_METHOD(Cassandra_DefaultSession, closeAsync);
ZEND_METHOD(Cassandra_DefaultSession, metrics);
ZEND_METHOD(Cassandra_DefaultSession, schema);


static const zend_function_entry class_Cassandra_DefaultSession_methods[] = {
	ZEND_ME(Cassandra_DefaultSession, execute, arginfo_class_Cassandra_DefaultSession_execute, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, executeAsync, arginfo_class_Cassandra_DefaultSession_executeAsync, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, prepare, arginfo_class_Cassandra_DefaultSession_prepare, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, prepareAsync, arginfo_class_Cassandra_DefaultSession_prepareAsync, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, close, arginfo_class_Cassandra_DefaultSession_close, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, closeAsync, arginfo_class_Cassandra_DefaultSession_closeAsync, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, metrics, arginfo_class_Cassandra_DefaultSession_metrics, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSession, schema, arginfo_class_Cassandra_DefaultSession_schema, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultSession(zend_class_entry *class_entry_Cassandra_Session)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultSession", class_Cassandra_DefaultSession_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Session);

	return class_entry;
}
