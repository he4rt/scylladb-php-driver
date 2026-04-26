/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: bfee8fdb3f95c0fcc18bdb5985b1973a29d98d22 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_BatchStatement___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, type, IS_LONG, 0, "Cassandra::BATCH_LOGGED")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_BatchStatement_add, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_TYPE_MASK(0, statement, Cassandra\\Statement, MAY_BE_STRING, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, arguments, IS_ARRAY, 1, "null")
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_BatchStatement, __construct);
ZEND_METHOD(Cassandra_BatchStatement, add);

static const zend_function_entry class_Cassandra_BatchStatement_methods[] = {
	ZEND_ME(Cassandra_BatchStatement, __construct, arginfo_class_Cassandra_BatchStatement___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_BatchStatement, add, arginfo_class_Cassandra_BatchStatement_add, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_BatchStatement(zend_class_entry *class_entry_Cassandra_Statement)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "BatchStatement", class_Cassandra_BatchStatement_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Statement);

	return class_entry;
}
