/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 2b05d886b5793e1c189c6a145c8522b491e0ed58 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Bigint___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\Bigint, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Bigint___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Bigint_value arginfo_class_Cassandra_Bigint___toString

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Bigint_min, 0, 0, Cassandra\\Bigint, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Bigint_max arginfo_class_Cassandra_Bigint_min


ZEND_METHOD(Cassandra_Bigint, __construct);
ZEND_METHOD(Cassandra_Bigint, __toString);
ZEND_METHOD(Cassandra_Bigint, value);
ZEND_METHOD(Cassandra_Bigint, min);
ZEND_METHOD(Cassandra_Bigint, max);


static const zend_function_entry class_Cassandra_Bigint_methods[] = {
	ZEND_ME(Cassandra_Bigint, __construct, arginfo_class_Cassandra_Bigint___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, __toString, arginfo_class_Cassandra_Bigint___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, value, arginfo_class_Cassandra_Bigint_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, min, arginfo_class_Cassandra_Bigint_min, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Bigint, max, arginfo_class_Cassandra_Bigint_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Bigint(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Bigint", class_Cassandra_Bigint_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
