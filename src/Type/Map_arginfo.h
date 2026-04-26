/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 72f2d4dc5377dec871a458007264e1a9663c4cb9 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Type_Map___construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Type_Map_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Map_keyType, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Type_Map_valueType arginfo_class_Cassandra_Type_Map_keyType

#define arginfo_class_Cassandra_Type_Map___toString arginfo_class_Cassandra_Type_Map_name

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Type_Map_create, 0, 0, Cassandra\\Map, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_Type_Map, __construct);
ZEND_METHOD(Cassandra_Type_Map, name);
ZEND_METHOD(Cassandra_Type_Map, keyType);
ZEND_METHOD(Cassandra_Type_Map, valueType);
ZEND_METHOD(Cassandra_Type_Map, __toString);
ZEND_METHOD(Cassandra_Type_Map, create);

static const zend_function_entry class_Cassandra_Type_Map_methods[] = {
	ZEND_ME(Cassandra_Type_Map, __construct, arginfo_class_Cassandra_Type_Map___construct, ZEND_ACC_PRIVATE)
	ZEND_ME(Cassandra_Type_Map, name, arginfo_class_Cassandra_Type_Map_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Map, keyType, arginfo_class_Cassandra_Type_Map_keyType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Map, valueType, arginfo_class_Cassandra_Type_Map_valueType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Map, __toString, arginfo_class_Cassandra_Type_Map___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Type_Map, create, arginfo_class_Cassandra_Type_Map_create, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Type_Map(zend_class_entry *class_entry_Cassandra_Type)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Type", "Map", class_Cassandra_Type_Map_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Type, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);

	return class_entry;
}
