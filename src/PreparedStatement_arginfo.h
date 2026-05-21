/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9e5e65b865ffd0b5e8d1f6cdd763065e1fa21cc8 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_PreparedStatement___construct, 0, 0, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_PreparedStatement, __construct);


static const zend_function_entry class_Cassandra_PreparedStatement_methods[] = {
	ZEND_ME(Cassandra_PreparedStatement, __construct, arginfo_class_Cassandra_PreparedStatement___construct, ZEND_ACC_PRIVATE)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_PreparedStatement(zend_class_entry *class_entry_Cassandra_Statement)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "PreparedStatement", class_Cassandra_PreparedStatement_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Statement);

	return class_entry;
}
