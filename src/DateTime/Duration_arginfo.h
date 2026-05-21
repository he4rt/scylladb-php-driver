/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: e9c455c4c9188ff6fb5c2e8fad0041141c6fccfc */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Duration___construct, 0, 0, 3)
	ZEND_ARG_OBJ_TYPE_MASK(0, months, Cassandra\\Bigint, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, days, Cassandra\\Bigint, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
	ZEND_ARG_OBJ_TYPE_MASK(0, nanos, Cassandra\\Bigint, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Duration_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Duration_months, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Duration_days arginfo_class_Cassandra_Duration_months

#define arginfo_class_Cassandra_Duration_nanos arginfo_class_Cassandra_Duration_months

#define arginfo_class_Cassandra_Duration___toString arginfo_class_Cassandra_Duration_months


ZEND_METHOD(Cassandra_Duration, __construct);
ZEND_METHOD(Cassandra_Duration, type);
ZEND_METHOD(Cassandra_Duration, months);
ZEND_METHOD(Cassandra_Duration, days);
ZEND_METHOD(Cassandra_Duration, nanos);
ZEND_METHOD(Cassandra_Duration, __toString);


static const zend_function_entry class_Cassandra_Duration_methods[] = {
	ZEND_ME(Cassandra_Duration, __construct, arginfo_class_Cassandra_Duration___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Duration, type, arginfo_class_Cassandra_Duration_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Duration, months, arginfo_class_Cassandra_Duration_months, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Duration, days, arginfo_class_Cassandra_Duration_days, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Duration, nanos, arginfo_class_Cassandra_Duration_nanos, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Duration, __toString, arginfo_class_Cassandra_Duration___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Duration(zend_class_entry *class_entry_Cassandra_Value)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Duration", class_Cassandra_Duration_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Value);

	return class_entry;
}
