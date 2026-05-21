/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1e5a1f7c1ce4aac266f6d7a970dff87fe58ed196 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_name, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_ascii, 0, 0, Cassandra\\Type\\Scalar, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_bigint arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_smallint arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_tinyint arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_blob arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_boolean arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_counter arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_decimal arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_double arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_duration arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_float arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_int arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_text arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_timestamp arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_date arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_time arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_uuid arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_varchar arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_varint arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_timeuuid arginfo_class_Cassandra_Type_ascii

#define arginfo_class_Cassandra_Type_inet arginfo_class_Cassandra_Type_ascii

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_collection, 0, 1, Cassandra\\Type\\Collection, 0)
	ZEND_ARG_OBJ_INFO(0, type, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_set, 0, 1, Cassandra\\Type\\Set, 0)
	ZEND_ARG_OBJ_INFO(0, type, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_map, 0, 2, Cassandra\\Type\\Map, 0)
	ZEND_ARG_OBJ_INFO(0, keyType, Cassandra\\Type, 0)
	ZEND_ARG_OBJ_INFO(0, valueType, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_tuple, 0, 0, Cassandra\\Type\\Tuple, 0)
	ZEND_ARG_VARIADIC_OBJ_INFO(0, types, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_userType, 0, 0, Cassandra\\Type\\\125serType, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, types, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Type, ascii);
ZEND_METHOD(Cassandra_Type, bigint);
ZEND_METHOD(Cassandra_Type, smallint);
ZEND_METHOD(Cassandra_Type, tinyint);
ZEND_METHOD(Cassandra_Type, blob);
ZEND_METHOD(Cassandra_Type, boolean);
ZEND_METHOD(Cassandra_Type, counter);
ZEND_METHOD(Cassandra_Type, decimal);
ZEND_METHOD(Cassandra_Type, double);
ZEND_METHOD(Cassandra_Type, duration);
ZEND_METHOD(Cassandra_Type, float);
ZEND_METHOD(Cassandra_Type, int);
ZEND_METHOD(Cassandra_Type, text);
ZEND_METHOD(Cassandra_Type, timestamp);
ZEND_METHOD(Cassandra_Type, date);
ZEND_METHOD(Cassandra_Type, time);
ZEND_METHOD(Cassandra_Type, uuid);
ZEND_METHOD(Cassandra_Type, varchar);
ZEND_METHOD(Cassandra_Type, varint);
ZEND_METHOD(Cassandra_Type, timeuuid);
ZEND_METHOD(Cassandra_Type, inet);
ZEND_METHOD(Cassandra_Type, collection);
ZEND_METHOD(Cassandra_Type, set);
ZEND_METHOD(Cassandra_Type, map);
ZEND_METHOD(Cassandra_Type, tuple);
ZEND_METHOD(Cassandra_Type, userType);


static const zend_function_entry class_Cassandra_Type_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Type, name, arginfo_class_Cassandra_Type_name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Type, __toString, arginfo_class_Cassandra_Type___toString, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ME(Cassandra_Type, ascii, arginfo_class_Cassandra_Type_ascii, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, bigint, arginfo_class_Cassandra_Type_bigint, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, smallint, arginfo_class_Cassandra_Type_smallint, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, tinyint, arginfo_class_Cassandra_Type_tinyint, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, blob, arginfo_class_Cassandra_Type_blob, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, boolean, arginfo_class_Cassandra_Type_boolean, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, counter, arginfo_class_Cassandra_Type_counter, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, decimal, arginfo_class_Cassandra_Type_decimal, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, double, arginfo_class_Cassandra_Type_double, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, duration, arginfo_class_Cassandra_Type_duration, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, float, arginfo_class_Cassandra_Type_float, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, int, arginfo_class_Cassandra_Type_int, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, text, arginfo_class_Cassandra_Type_text, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, timestamp, arginfo_class_Cassandra_Type_timestamp, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, date, arginfo_class_Cassandra_Type_date, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, time, arginfo_class_Cassandra_Type_time, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, uuid, arginfo_class_Cassandra_Type_uuid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, varchar, arginfo_class_Cassandra_Type_varchar, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, varint, arginfo_class_Cassandra_Type_varint, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, timeuuid, arginfo_class_Cassandra_Type_timeuuid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, inet, arginfo_class_Cassandra_Type_inet, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, collection, arginfo_class_Cassandra_Type_collection, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, set, arginfo_class_Cassandra_Type_set, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, map, arginfo_class_Cassandra_Type_map, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, tuple, arginfo_class_Cassandra_Type_tuple, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_ME(Cassandra_Type, userType, arginfo_class_Cassandra_Type_userType, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Type", class_Cassandra_Type_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
