/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 28700aee9c543495460ce3eafbd1f25b6154aa1a */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Index_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Index_kind arginfo_class_Cassandra_Index_name

#define arginfo_class_Cassandra_Index_target arginfo_class_Cassandra_Index_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Index_option, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Index_options, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_Cassandra_Index_className, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Index_isCustom, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Index_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, name, arginfo_class_Cassandra_Index_name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, kind, arginfo_class_Cassandra_Index_kind, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, target, arginfo_class_Cassandra_Index_target, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, option, arginfo_class_Cassandra_Index_option, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, options, arginfo_class_Cassandra_Index_options, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, className, arginfo_class_Cassandra_Index_className, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Index, isCustom, arginfo_class_Cassandra_Index_isCustom, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Index(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Index", class_Cassandra_Index_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
