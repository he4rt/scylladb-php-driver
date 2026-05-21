/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 7ab69ba96cdae8122d9db58561bb28256c3e52ac */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Decimal___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\Decimal, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Decimal_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal_value, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal_scale, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal_add, 0, 1, IS_STATIC, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\\116umeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Decimal_sub arginfo_class_Cassandra_Decimal_add

#define arginfo_class_Cassandra_Decimal_mul arginfo_class_Cassandra_Decimal_add

#define arginfo_class_Cassandra_Decimal_div arginfo_class_Cassandra_Decimal_add

#define arginfo_class_Cassandra_Decimal_mod arginfo_class_Cassandra_Decimal_add

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal_abs, 0, 0, IS_STATIC, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Decimal_neg arginfo_class_Cassandra_Decimal_abs

#define arginfo_class_Cassandra_Decimal_sqrt arginfo_class_Cassandra_Decimal_abs

#define arginfo_class_Cassandra_Decimal_toInt arginfo_class_Cassandra_Decimal_scale

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Decimal_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Decimal___toString arginfo_class_Cassandra_Decimal_value


ZEND_METHOD(Cassandra_Decimal, __construct);
ZEND_METHOD(Cassandra_Decimal, type);
ZEND_METHOD(Cassandra_Decimal, value);
ZEND_METHOD(Cassandra_Decimal, scale);
ZEND_METHOD(Cassandra_Decimal, add);
ZEND_METHOD(Cassandra_Decimal, sub);
ZEND_METHOD(Cassandra_Decimal, mul);
ZEND_METHOD(Cassandra_Decimal, div);
ZEND_METHOD(Cassandra_Decimal, mod);
ZEND_METHOD(Cassandra_Decimal, abs);
ZEND_METHOD(Cassandra_Decimal, neg);
ZEND_METHOD(Cassandra_Decimal, sqrt);
ZEND_METHOD(Cassandra_Decimal, toInt);
ZEND_METHOD(Cassandra_Decimal, toDouble);
ZEND_METHOD(Cassandra_Decimal, __toString);


static const zend_function_entry class_Cassandra_Decimal_methods[] = {
	ZEND_ME(Cassandra_Decimal, __construct, arginfo_class_Cassandra_Decimal___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, type, arginfo_class_Cassandra_Decimal_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, value, arginfo_class_Cassandra_Decimal_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, scale, arginfo_class_Cassandra_Decimal_scale, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, add, arginfo_class_Cassandra_Decimal_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, sub, arginfo_class_Cassandra_Decimal_sub, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, mul, arginfo_class_Cassandra_Decimal_mul, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, div, arginfo_class_Cassandra_Decimal_div, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, mod, arginfo_class_Cassandra_Decimal_mod, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, abs, arginfo_class_Cassandra_Decimal_abs, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, neg, arginfo_class_Cassandra_Decimal_neg, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, sqrt, arginfo_class_Cassandra_Decimal_sqrt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, toInt, arginfo_class_Cassandra_Decimal_toInt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, toDouble, arginfo_class_Cassandra_Decimal_toDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Decimal, __toString, arginfo_class_Cassandra_Decimal___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Decimal(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Decimal", class_Cassandra_Decimal_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
