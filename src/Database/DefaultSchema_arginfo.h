/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b5630ecf25d1c3a3be3235695a99b686b1b8d701 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_class_Cassandra_DefaultSchema_keyspace, 0, 1, Cassandra\\Keyspace, MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultSchema_keyspaces, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultSchema_version, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_DefaultSchema, keyspace);
ZEND_METHOD(Cassandra_DefaultSchema, keyspaces);
ZEND_METHOD(Cassandra_DefaultSchema, version);


static const zend_function_entry class_Cassandra_DefaultSchema_methods[] = {
	ZEND_ME(Cassandra_DefaultSchema, keyspace, arginfo_class_Cassandra_DefaultSchema_keyspace, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSchema, keyspaces, arginfo_class_Cassandra_DefaultSchema_keyspaces, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultSchema, version, arginfo_class_Cassandra_DefaultSchema_version, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultSchema(zend_class_entry *class_entry_Cassandra_Schema)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultSchema", class_Cassandra_DefaultSchema_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Schema);

	return class_entry;
}
