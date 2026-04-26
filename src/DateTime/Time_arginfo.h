/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 74d241cbd4bca211d1360e838945b5c435a09f2b */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Time___construct, 0, 0, 0)
	ZEND_ARG_TYPE_MASK(0, nanoseconds, MAY_BE_LONG|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Time_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Time_seconds, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Time_fromDateTime, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, datetime, DateTimeInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Time___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_Time, __construct);
ZEND_METHOD(Cassandra_Time, type);
ZEND_METHOD(Cassandra_Time, seconds);
ZEND_METHOD(Cassandra_Time, fromDateTime);
ZEND_METHOD(Cassandra_Time, __toString);

static const zend_function_entry class_Cassandra_Time_methods[] = {
	ZEND_ME(Cassandra_Time, __construct, arginfo_class_Cassandra_Time___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Time, type, arginfo_class_Cassandra_Time_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Time, seconds, arginfo_class_Cassandra_Time_seconds, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Time, fromDateTime, arginfo_class_Cassandra_Time_fromDateTime, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Time, __toString, arginfo_class_Cassandra_Time___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Time(zend_class_entry *class_entry_Cassandra_Value)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Time", class_Cassandra_Time_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Value);

	return class_entry;
}
