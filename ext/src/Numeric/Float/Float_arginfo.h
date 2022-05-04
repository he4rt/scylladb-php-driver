/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: af7e62d67b9df694599ace8d0f081438569d42ce */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Float___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\double, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_DOUBLE, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float_value, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float_isInfinite, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_isFinite arginfo_class_Cassandra_Float_isInfinite

#define arginfo_class_Cassandra_Float_isNaN arginfo_class_Cassandra_Float_isInfinite

#define arginfo_class_Cassandra_Float_min arginfo_class_Cassandra_Float_value

#define arginfo_class_Cassandra_Float_max arginfo_class_Cassandra_Float_value


ZEND_METHOD(Cassandra_Float, __construct);
ZEND_METHOD(Cassandra_Float, __toString);
ZEND_METHOD(Cassandra_Float, value);
ZEND_METHOD(Cassandra_Float, isInfinite);
ZEND_METHOD(Cassandra_Float, isFinite);
ZEND_METHOD(Cassandra_Float, isNaN);
ZEND_METHOD(Cassandra_Float, min);
ZEND_METHOD(Cassandra_Float, max);


static const zend_function_entry class_Cassandra_Float_methods[] = {
	ZEND_ME(Cassandra_Float, __construct, arginfo_class_Cassandra_Float___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, __toString, arginfo_class_Cassandra_Float___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, value, arginfo_class_Cassandra_Float_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, isInfinite, arginfo_class_Cassandra_Float_isInfinite, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, isFinite, arginfo_class_Cassandra_Float_isFinite, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, isNaN, arginfo_class_Cassandra_Float_isNaN, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, min, arginfo_class_Cassandra_Float_min, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Float, max, arginfo_class_Cassandra_Float_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Float(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Float", class_Cassandra_Float_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
