/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 77e7cdb40569c33df11477c90dc69328b5a8bd80 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Tinyint___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\Tinyint, MAY_BE_LONG|MAY_BE_DOUBLE|MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tinyint___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Tinyint_value arginfo_class_Cassandra_Tinyint___toString

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Tinyint_min, 0, 0, Cassandra\\Tinyint, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Tinyint_max arginfo_class_Cassandra_Tinyint_min

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Tinyint_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Tinyint_add, 0, 1, Cassandra\\Numeric, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Tinyint_sub arginfo_class_Cassandra_Tinyint_add

#define arginfo_class_Cassandra_Tinyint_mul arginfo_class_Cassandra_Tinyint_add

#define arginfo_class_Cassandra_Tinyint_div arginfo_class_Cassandra_Tinyint_add

#define arginfo_class_Cassandra_Tinyint_mod arginfo_class_Cassandra_Tinyint_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Tinyint_abs, 0, 0, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Tinyint_neg arginfo_class_Cassandra_Tinyint_abs

#define arginfo_class_Cassandra_Tinyint_sqrt arginfo_class_Cassandra_Tinyint_abs

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tinyint_toInt, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Tinyint_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Tinyint, __construct);
ZEND_METHOD(Cassandra_Tinyint, __toString);
ZEND_METHOD(Cassandra_Tinyint, value);
ZEND_METHOD(Cassandra_Tinyint, min);
ZEND_METHOD(Cassandra_Tinyint, max);
ZEND_METHOD(Cassandra_Tinyint, type);
ZEND_METHOD(Cassandra_Tinyint, add);
ZEND_METHOD(Cassandra_Tinyint, sub);
ZEND_METHOD(Cassandra_Tinyint, mul);
ZEND_METHOD(Cassandra_Tinyint, div);
ZEND_METHOD(Cassandra_Tinyint, mod);
ZEND_METHOD(Cassandra_Tinyint, abs);
ZEND_METHOD(Cassandra_Tinyint, neg);
ZEND_METHOD(Cassandra_Tinyint, sqrt);
ZEND_METHOD(Cassandra_Tinyint, toInt);
ZEND_METHOD(Cassandra_Tinyint, toDouble);


static const zend_function_entry class_Cassandra_Tinyint_methods[] = {
	ZEND_ME(Cassandra_Tinyint, __construct, arginfo_class_Cassandra_Tinyint___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, __toString, arginfo_class_Cassandra_Tinyint___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, value, arginfo_class_Cassandra_Tinyint_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, min, arginfo_class_Cassandra_Tinyint_min, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Tinyint, max, arginfo_class_Cassandra_Tinyint_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Tinyint, type, arginfo_class_Cassandra_Tinyint_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, add, arginfo_class_Cassandra_Tinyint_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, sub, arginfo_class_Cassandra_Tinyint_sub, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, mul, arginfo_class_Cassandra_Tinyint_mul, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, div, arginfo_class_Cassandra_Tinyint_div, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, mod, arginfo_class_Cassandra_Tinyint_mod, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, abs, arginfo_class_Cassandra_Tinyint_abs, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, neg, arginfo_class_Cassandra_Tinyint_neg, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, sqrt, arginfo_class_Cassandra_Tinyint_sqrt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, toInt, arginfo_class_Cassandra_Tinyint_toInt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Tinyint, toDouble, arginfo_class_Cassandra_Tinyint_toDouble, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Tinyint(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Tinyint", class_Cassandra_Tinyint_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
