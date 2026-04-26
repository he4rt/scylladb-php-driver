/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: db1223a5e487aa161a81af4b1b97cf5f30f854e6 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Numeric_add, 0, 1, Cassandra\\\116umeric, 0)
	ZEND_ARG_OBJ_INFO(0, num, Cassandra\\\116umeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Numeric_sub arginfo_class_Cassandra_Numeric_add

#define arginfo_class_Cassandra_Numeric_mul arginfo_class_Cassandra_Numeric_add

#define arginfo_class_Cassandra_Numeric_div arginfo_class_Cassandra_Numeric_add

#define arginfo_class_Cassandra_Numeric_mod arginfo_class_Cassandra_Numeric_add

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Numeric_abs, 0, 0, Cassandra\\\116umeric, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Numeric_neg arginfo_class_Cassandra_Numeric_abs

#define arginfo_class_Cassandra_Numeric_sqrt arginfo_class_Cassandra_Numeric_abs

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Numeric_toInt, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Numeric_toDouble, 0, 0, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()


static const zend_function_entry class_Cassandra_Numeric_methods[] = {
	ZEND_RAW_FENTRY("add", NULL, arginfo_class_Cassandra_Numeric_add, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("sub", NULL, arginfo_class_Cassandra_Numeric_sub, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("mul", NULL, arginfo_class_Cassandra_Numeric_mul, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("div", NULL, arginfo_class_Cassandra_Numeric_div, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("mod", NULL, arginfo_class_Cassandra_Numeric_mod, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("abs", NULL, arginfo_class_Cassandra_Numeric_abs, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("neg", NULL, arginfo_class_Cassandra_Numeric_neg, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("sqrt", NULL, arginfo_class_Cassandra_Numeric_sqrt, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("toInt", NULL, arginfo_class_Cassandra_Numeric_toInt, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("toDouble", NULL, arginfo_class_Cassandra_Numeric_toDouble, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Numeric(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Numeric", class_Cassandra_Numeric_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
