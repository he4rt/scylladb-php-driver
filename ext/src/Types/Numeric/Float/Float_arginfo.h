/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 2c6ba567652cb00f644637f7c308d1695fb07475 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Float___construct, 0, 0, 1)
	ZEND_ARG_TYPE_MASK(0, value, MAY_BE_DOUBLE|MAY_BE_LONG|MAY_BE_STRING|MAY_BE_DOUBLE, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_value arginfo_class_Cassandra_Float___toString

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float_isInfinite, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_isFinite arginfo_class_Cassandra_Float_isInfinite

#define arginfo_class_Cassandra_Float_isNaN arginfo_class_Cassandra_Float_isInfinite

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float_min, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_max arginfo_class_Cassandra_Float_min

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Float_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Float_add, 0, 1, Cassandra\\Numeric, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_sub arginfo_class_Cassandra_Float_add

#define arginfo_class_Cassandra_Float_mul arginfo_class_Cassandra_Float_add

#define arginfo_class_Cassandra_Float_div arginfo_class_Cassandra_Float_add

#define arginfo_class_Cassandra_Float_mod arginfo_class_Cassandra_Float_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Float_abs, 0, 0, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_neg arginfo_class_Cassandra_Float_abs

#define arginfo_class_Cassandra_Float_sqrt arginfo_class_Cassandra_Float_abs

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Float_toInt, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Float_toDouble arginfo_class_Cassandra_Float_min


ZEND_METHOD(Cassandra_Float, __construct);
ZEND_METHOD(Cassandra_Float, __toString);
ZEND_METHOD(Cassandra_Float, value);
ZEND_METHOD(Cassandra_Float, isInfinite);
ZEND_METHOD(Cassandra_Float, isFinite);
ZEND_METHOD(Cassandra_Float, isNaN);
ZEND_METHOD(Cassandra_Float, min);
ZEND_METHOD(Cassandra_Float, max);
ZEND_METHOD(Cassandra_Float, type);
ZEND_METHOD(Cassandra_Float, add);
ZEND_METHOD(Cassandra_Float, sub);
ZEND_METHOD(Cassandra_Float, mul);
ZEND_METHOD(Cassandra_Float, div);
ZEND_METHOD(Cassandra_Float, mod);
ZEND_METHOD(Cassandra_Float, abs);
ZEND_METHOD(Cassandra_Float, neg);
ZEND_METHOD(Cassandra_Float, sqrt);
ZEND_METHOD(Cassandra_Float, toInt);
ZEND_METHOD(Cassandra_Float, toDouble);


static const zend_function_entry class_Cassandra_Float_methods[] = {
	ZEND_ME(Cassandra_Float, __construct, arginfo_class_Cassandra_Float___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, __toString, arginfo_class_Cassandra_Float___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, value, arginfo_class_Cassandra_Float_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, isInfinite, arginfo_class_Cassandra_Float_isInfinite, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, isFinite, arginfo_class_Cassandra_Float_isFinite, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, isNaN, arginfo_class_Cassandra_Float_isNaN, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, min, arginfo_class_Cassandra_Float_min, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Float, max, arginfo_class_Cassandra_Float_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Float, type, arginfo_class_Cassandra_Float_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, add, arginfo_class_Cassandra_Float_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, sub, arginfo_class_Cassandra_Float_sub, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, mul, arginfo_class_Cassandra_Float_mul, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, div, arginfo_class_Cassandra_Float_div, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, mod, arginfo_class_Cassandra_Float_mod, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, abs, arginfo_class_Cassandra_Float_abs, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, neg, arginfo_class_Cassandra_Float_neg, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, sqrt, arginfo_class_Cassandra_Float_sqrt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, toInt, arginfo_class_Cassandra_Float_toInt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Float, toDouble, arginfo_class_Cassandra_Float_toDouble, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Float(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Float", class_Cassandra_Float_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
