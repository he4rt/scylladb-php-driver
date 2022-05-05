/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: e3666de2eb9b832aa753c99017cee32f5b7e0415 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_Varint___construct, 0, 0, 1)
	ZEND_ARG_OBJ_TYPE_MASK(0, value, Cassandra\\Varint, MAY_BE_STRING, NULL)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Varint___toString, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Varint_value arginfo_class_Cassandra_Varint___toString

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Varint_type, 0, 0, Cassandra\\Type, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Varint_add, 0, 1, Cassandra\\Numeric, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Varint_sub arginfo_class_Cassandra_Varint_add

#define arginfo_class_Cassandra_Varint_mul arginfo_class_Cassandra_Varint_add

#define arginfo_class_Cassandra_Varint_div arginfo_class_Cassandra_Varint_add

#define arginfo_class_Cassandra_Varint_mod arginfo_class_Cassandra_Varint_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Varint_abs, 0, 0, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Varint_neg arginfo_class_Cassandra_Varint_abs

#define arginfo_class_Cassandra_Varint_sqrt arginfo_class_Cassandra_Varint_abs

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Varint_toInt, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Varint_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_Varint, __construct);
ZEND_METHOD(Cassandra_Varint, __toString);
ZEND_METHOD(Cassandra_Varint, value);
ZEND_METHOD(Cassandra_Varint, type);
ZEND_METHOD(Cassandra_Varint, add);
ZEND_METHOD(Cassandra_Varint, sub);
ZEND_METHOD(Cassandra_Varint, mul);
ZEND_METHOD(Cassandra_Varint, div);
ZEND_METHOD(Cassandra_Varint, mod);
ZEND_METHOD(Cassandra_Varint, abs);
ZEND_METHOD(Cassandra_Varint, neg);
ZEND_METHOD(Cassandra_Varint, sqrt);
ZEND_METHOD(Cassandra_Varint, toInt);
ZEND_METHOD(Cassandra_Varint, toDouble);


static const zend_function_entry class_Cassandra_Varint_methods[] = {
	ZEND_ME(Cassandra_Varint, __construct, arginfo_class_Cassandra_Varint___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, __toString, arginfo_class_Cassandra_Varint___toString, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, value, arginfo_class_Cassandra_Varint_value, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, type, arginfo_class_Cassandra_Varint_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, add, arginfo_class_Cassandra_Varint_add, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, sub, arginfo_class_Cassandra_Varint_sub, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, mul, arginfo_class_Cassandra_Varint_mul, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, div, arginfo_class_Cassandra_Varint_div, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, mod, arginfo_class_Cassandra_Varint_mod, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, abs, arginfo_class_Cassandra_Varint_abs, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, neg, arginfo_class_Cassandra_Varint_neg, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, sqrt, arginfo_class_Cassandra_Varint_sqrt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, toInt, arginfo_class_Cassandra_Varint_toInt, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_Varint, toDouble, arginfo_class_Cassandra_Varint_toDouble, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Varint(zend_class_entry *class_entry_Cassandra_Value, zend_class_entry *class_entry_Cassandra_Numeric)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Varint", class_Cassandra_Varint_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Cassandra_Value, class_entry_Cassandra_Numeric);

	return class_entry;
}
