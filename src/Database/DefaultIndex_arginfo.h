/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 79923875f15316ed26a9830c79a150d2335de012 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultIndex_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultIndex_kind arginfo_class_Cassandra_DefaultIndex_name

#define arginfo_class_Cassandra_DefaultIndex_target arginfo_class_Cassandra_DefaultIndex_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultIndex_option, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultIndex_options, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_class_Cassandra_DefaultIndex_className, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultIndex_isCustom, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_DefaultIndex, name);
ZEND_METHOD(Cassandra_DefaultIndex, kind);
ZEND_METHOD(Cassandra_DefaultIndex, target);
ZEND_METHOD(Cassandra_DefaultIndex, option);
ZEND_METHOD(Cassandra_DefaultIndex, options);
ZEND_METHOD(Cassandra_DefaultIndex, className);
ZEND_METHOD(Cassandra_DefaultIndex, isCustom);

static const zend_function_entry class_Cassandra_DefaultIndex_methods[] = {
	ZEND_ME(Cassandra_DefaultIndex, name, arginfo_class_Cassandra_DefaultIndex_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultIndex, kind, arginfo_class_Cassandra_DefaultIndex_kind, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultIndex, target, arginfo_class_Cassandra_DefaultIndex_target, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultIndex, option, arginfo_class_Cassandra_DefaultIndex_option, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultIndex, options, arginfo_class_Cassandra_DefaultIndex_options, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultIndex, className, arginfo_class_Cassandra_DefaultIndex_className, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultIndex, isCustom, arginfo_class_Cassandra_DefaultIndex_isCustom, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultIndex(zend_class_entry *class_entry_Cassandra_Index)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultIndex", class_Cassandra_DefaultIndex_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Index);

	return class_entry;
}
