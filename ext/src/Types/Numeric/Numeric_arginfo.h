/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1552a18df062f59c931b6c9eca919c98cc470053 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Numeric_add, 0, 1, Cassandra\\Numeric, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Numeric_sub arginfo_class_Cassandra_Numeric_add

#define arginfo_class_Cassandra_Numeric_mul arginfo_class_Cassandra_Numeric_add

#define arginfo_class_Cassandra_Numeric_div arginfo_class_Cassandra_Numeric_add

#define arginfo_class_Cassandra_Numeric_mod arginfo_class_Cassandra_Numeric_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Numeric_abs, 0, 0, Cassandra\\Numeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Numeric_neg arginfo_class_Cassandra_Numeric_abs

#define arginfo_class_Cassandra_Numeric_sqrt arginfo_class_Cassandra_Numeric_abs

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Numeric_toInt, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Numeric_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Numeric_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, add, arginfo_class_Cassandra_Numeric_add, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, sub, arginfo_class_Cassandra_Numeric_sub, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, mul, arginfo_class_Cassandra_Numeric_mul, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, div, arginfo_class_Cassandra_Numeric_div, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, mod, arginfo_class_Cassandra_Numeric_mod, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, abs, arginfo_class_Cassandra_Numeric_abs, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, neg, arginfo_class_Cassandra_Numeric_neg, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, sqrt, arginfo_class_Cassandra_Numeric_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, toInt, arginfo_class_Cassandra_Numeric_toInt, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Numeric, toDouble, arginfo_class_Cassandra_Numeric_toDouble, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Numeric(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Numeric", class_Cassandra_Numeric_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
