/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b1a83765fcd2db94f007a352e5a1a8ebe900770e */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Bigint___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\Bigint, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Bigint___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Bigint_value arginfo_class_Cassandra_Bigint___toString

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Bigint_min, 0, 0, Cassandra\\Bigint, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Bigint_max arginfo_class_Cassandra_Bigint_min

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Bigint_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Bigint_add, 0, 1, Cassandra\\Numeric, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Bigint_sub arginfo_class_Cassandra_Bigint_add

#define arginfo_class_Cassandra_Bigint_mul arginfo_class_Cassandra_Bigint_add

#define arginfo_class_Cassandra_Bigint_div arginfo_class_Cassandra_Bigint_add

#define arginfo_class_Cassandra_Bigint_mod arginfo_class_Cassandra_Bigint_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Bigint_abs, 0, 0, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Bigint_neg arginfo_class_Cassandra_Bigint_abs

#define arginfo_class_Cassandra_Bigint_sqrt arginfo_class_Cassandra_Bigint_abs

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Bigint_toInt, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Bigint_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Bigint, __construct);
ZEND_METHOD(Cassandra_Bigint, __toString);
ZEND_METHOD(Cassandra_Bigint, value);
ZEND_METHOD(Cassandra_Bigint, min);
ZEND_METHOD(Cassandra_Bigint, max);
ZEND_METHOD(Cassandra_Bigint, type);
ZEND_METHOD(Cassandra_Bigint, add);
ZEND_METHOD(Cassandra_Bigint, sub);
ZEND_METHOD(Cassandra_Bigint, mul);
ZEND_METHOD(Cassandra_Bigint, div);
ZEND_METHOD(Cassandra_Bigint, mod);
ZEND_METHOD(Cassandra_Bigint, abs);
ZEND_METHOD(Cassandra_Bigint, neg);
ZEND_METHOD(Cassandra_Bigint, sqrt);
ZEND_METHOD(Cassandra_Bigint, toInt);
ZEND_METHOD(Cassandra_Bigint, toDouble);


static const zend_function_entry class_Cassandra_Bigint_methods[] = {
	ZEND_ME(Cassandra_Bigint, __construct, arginfo_class_Cassandra_Bigint___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, __toString, arginfo_class_Cassandra_Bigint___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, value, arginfo_class_Cassandra_Bigint_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, min, arginfo_class_Cassandra_Bigint_min, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Bigint, max, arginfo_class_Cassandra_Bigint_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Bigint, type, arginfo_class_Cassandra_Bigint_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, add, arginfo_class_Cassandra_Bigint_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, sub, arginfo_class_Cassandra_Bigint_sub, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, mul, arginfo_class_Cassandra_Bigint_mul, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, div, arginfo_class_Cassandra_Bigint_div, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, mod, arginfo_class_Cassandra_Bigint_mod, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, abs, arginfo_class_Cassandra_Bigint_abs, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, neg, arginfo_class_Cassandra_Bigint_neg, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, sqrt, arginfo_class_Cassandra_Bigint_sqrt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, toInt, arginfo_class_Cassandra_Bigint_toInt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Bigint, toDouble, arginfo_class_Cassandra_Bigint_toDouble, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Bigint(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Bigint", class_Cassandra_Bigint_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
