/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: ce70dd0967a30f28e1dac29d11f7d87ce8fde4db */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Decimal___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\Decimal, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Decimal_value arginfo_class_Cassandra_Decimal___toString

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal_scale, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Decimal, __construct);
ZEND_METHOD(Cassandra_Decimal, __toString);
ZEND_METHOD(Cassandra_Decimal, value);
ZEND_METHOD(Cassandra_Decimal, scale);


static const zend_function_entry class_Cassandra_Decimal_methods[] = {
	ZEND_ME(Cassandra_Decimal, __construct, arginfo_class_Cassandra_Decimal___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, __toString, arginfo_class_Cassandra_Decimal___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, value, arginfo_class_Cassandra_Decimal_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, scale, arginfo_class_Cassandra_Decimal_scale, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Decimal(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Decimal", class_Cassandra_Decimal_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
