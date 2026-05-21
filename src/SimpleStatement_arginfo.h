/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a9627ff9655740bcda0ba15690e1a0d53daeaac3 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_SimpleStatement___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, cql, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_SimpleStatement, __construct);


static const zend_function_entry class_Cassandra_SimpleStatement_methods[] = {
	ZEND_ME(Cassandra_SimpleStatement, __construct, arginfo_class_Cassandra_SimpleStatement___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_SimpleStatement(zend_class_entry *class_entry_Cassandra_Statement)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "SimpleStatement", class_Cassandra_SimpleStatement_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Statement);

	return class_entry;
}
