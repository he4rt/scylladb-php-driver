/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: e4556dbeef4df3c54e6850645fff80cf54c28118 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Timestamp___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, seconds, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, microseconds, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Timestamp_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Timestamp_time, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_Cassandra_Timestamp_microtime, 0, 0, MAY_BE_DOUBLE|MAY_BE_STRING)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, get_as_float, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Timestamp_toDateTime, 0, 0, DateTime, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Timestamp_fromDateTime, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, datetime, DateTimeInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Timestamp___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Timestamp, __construct);
ZEND_METHOD(Cassandra_Timestamp, type);
ZEND_METHOD(Cassandra_Timestamp, time);
ZEND_METHOD(Cassandra_Timestamp, microtime);
ZEND_METHOD(Cassandra_Timestamp, toDateTime);
ZEND_METHOD(Cassandra_Timestamp, fromDateTime);
ZEND_METHOD(Cassandra_Timestamp, __toString);


static const zend_function_entry class_Cassandra_Timestamp_methods[] = {
	ZEND_ME(Cassandra_Timestamp, __construct, arginfo_class_Cassandra_Timestamp___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Timestamp, type, arginfo_class_Cassandra_Timestamp_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Timestamp, time, arginfo_class_Cassandra_Timestamp_time, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Timestamp, microtime, arginfo_class_Cassandra_Timestamp_microtime, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Timestamp, toDateTime, arginfo_class_Cassandra_Timestamp_toDateTime, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Timestamp, fromDateTime, arginfo_class_Cassandra_Timestamp_fromDateTime, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Timestamp, __toString, arginfo_class_Cassandra_Timestamp___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Timestamp(zend_class_entry *class_entry_Cassandra_Value)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Timestamp", class_Cassandra_Timestamp_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Value);

	return class_entry;
}
