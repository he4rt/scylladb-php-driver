/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 6fd76586d5920ae4eb2e3088334867d35d74976a */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Smallint___construct, 0, 0, 1)
	ZEND_ARG_TYPE_MASK(0, value, MAY_BE_STRING|MAY_BE_LONG|MAY_BE_DOUBLE, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Smallint_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Smallint_value, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Smallint_add, 0, 1, Cassandra\\\116umeric, 0)
	ZEND_ARG_TYPE_INFO(0, num, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Smallint_sub arginfo_class_Cassandra_Smallint_add

#define arginfo_class_Cassandra_Smallint_mul arginfo_class_Cassandra_Smallint_add

#define arginfo_class_Cassandra_Smallint_div arginfo_class_Cassandra_Smallint_add

#define arginfo_class_Cassandra_Smallint_mod arginfo_class_Cassandra_Smallint_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Smallint_abs, 0, 0, Cassandra\\\116umeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Smallint_neg arginfo_class_Cassandra_Smallint_abs

#define arginfo_class_Cassandra_Smallint_sqrt arginfo_class_Cassandra_Smallint_abs

#define arginfo_class_Cassandra_Smallint_toInt arginfo_class_Cassandra_Smallint_value

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Smallint_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Smallint_min, 0, 0, Cassandra\\Smallint, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Smallint_max arginfo_class_Cassandra_Smallint_min

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Smallint___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_Smallint, __construct);
ZEND_METHOD(Cassandra_Smallint, type);
ZEND_METHOD(Cassandra_Smallint, value);
ZEND_METHOD(Cassandra_Smallint, add);
ZEND_METHOD(Cassandra_Smallint, sub);
ZEND_METHOD(Cassandra_Smallint, mul);
ZEND_METHOD(Cassandra_Smallint, div);
ZEND_METHOD(Cassandra_Smallint, mod);
ZEND_METHOD(Cassandra_Smallint, abs);
ZEND_METHOD(Cassandra_Smallint, neg);
ZEND_METHOD(Cassandra_Smallint, sqrt);
ZEND_METHOD(Cassandra_Smallint, toInt);
ZEND_METHOD(Cassandra_Smallint, toDouble);
ZEND_METHOD(Cassandra_Smallint, min);
ZEND_METHOD(Cassandra_Smallint, max);
ZEND_METHOD(Cassandra_Smallint, __toString);

static const zend_function_entry class_Cassandra_Smallint_methods[] = {
	ZEND_ME(Cassandra_Smallint, __construct, arginfo_class_Cassandra_Smallint___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, type, arginfo_class_Cassandra_Smallint_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, value, arginfo_class_Cassandra_Smallint_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, add, arginfo_class_Cassandra_Smallint_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, sub, arginfo_class_Cassandra_Smallint_sub, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, mul, arginfo_class_Cassandra_Smallint_mul, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, div, arginfo_class_Cassandra_Smallint_div, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, mod, arginfo_class_Cassandra_Smallint_mod, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, abs, arginfo_class_Cassandra_Smallint_abs, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, neg, arginfo_class_Cassandra_Smallint_neg, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, sqrt, arginfo_class_Cassandra_Smallint_sqrt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, toInt, arginfo_class_Cassandra_Smallint_toInt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, toDouble, arginfo_class_Cassandra_Smallint_toDouble, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Smallint, min, arginfo_class_Cassandra_Smallint_min, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Smallint, max, arginfo_class_Cassandra_Smallint_max, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Cassandra_Smallint, __toString, arginfo_class_Cassandra_Smallint___toString, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Smallint(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Smallint", class_Cassandra_Smallint_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
