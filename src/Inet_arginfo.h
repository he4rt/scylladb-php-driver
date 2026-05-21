/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 58241ec6588ce2ab59a69acb728bb752d02f5735 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Inet___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Inet_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Inet_address, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Inet___toString arginfo_class_Cassandra_Inet_address


ZEND_METHOD(Cassandra_Inet, __construct);
ZEND_METHOD(Cassandra_Inet, type);
ZEND_METHOD(Cassandra_Inet, address);
ZEND_METHOD(Cassandra_Inet, __toString);


static const zend_function_entry class_Cassandra_Inet_methods[] = {
	ZEND_ME(Cassandra_Inet, __construct, arginfo_class_Cassandra_Inet___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Inet, type, arginfo_class_Cassandra_Inet_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Inet, address, arginfo_class_Cassandra_Inet_address, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Inet, __toString, arginfo_class_Cassandra_Inet___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Inet(zend_class_entry *class_entry_Cassandra_Value)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Inet", class_Cassandra_Inet_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Value);

	return class_entry;
}
