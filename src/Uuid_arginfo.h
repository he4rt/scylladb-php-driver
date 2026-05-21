/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a1df7911e8d20261a28b79b021b7f608f56e36f8 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Uuid___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, uuid, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Uuid_uuid, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Uuid_version, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Uuid_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Uuid___toString arginfo_class_Cassandra_Uuid_uuid


ZEND_METHOD(Cassandra_Uuid, __construct);
ZEND_METHOD(Cassandra_Uuid, uuid);
ZEND_METHOD(Cassandra_Uuid, version);
ZEND_METHOD(Cassandra_Uuid, type);
ZEND_METHOD(Cassandra_Uuid, __toString);


static const zend_function_entry class_Cassandra_Uuid_methods[] = {
	ZEND_ME(Cassandra_Uuid, __construct, arginfo_class_Cassandra_Uuid___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Uuid, uuid, arginfo_class_Cassandra_Uuid_uuid, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Uuid, version, arginfo_class_Cassandra_Uuid_version, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Uuid, type, arginfo_class_Cassandra_Uuid_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Uuid, __toString, arginfo_class_Cassandra_Uuid___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Uuid(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_UuidInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Uuid", class_Cassandra_Uuid_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_UuidInterface);

	return class_entry;
}
