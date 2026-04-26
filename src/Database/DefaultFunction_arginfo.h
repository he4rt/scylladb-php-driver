/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 792f53f41006bfd09466db67994b9868c8e8f9e0 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultFunction_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultFunction_simpleName arginfo_class_Cassandra_DefaultFunction_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultFunction_arguments, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultFunction_returnType, 0, 0, Cassandra\\Type, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultFunction_signature arginfo_class_Cassandra_DefaultFunction_name

#define arginfo_class_Cassandra_DefaultFunction_language arginfo_class_Cassandra_DefaultFunction_name

#define arginfo_class_Cassandra_DefaultFunction_body arginfo_class_Cassandra_DefaultFunction_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultFunction_isCalledOnNullInput, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_DefaultFunction, name);
ZEND_METHOD(Cassandra_DefaultFunction, simpleName);
ZEND_METHOD(Cassandra_DefaultFunction, arguments);
ZEND_METHOD(Cassandra_DefaultFunction, returnType);
ZEND_METHOD(Cassandra_DefaultFunction, signature);
ZEND_METHOD(Cassandra_DefaultFunction, language);
ZEND_METHOD(Cassandra_DefaultFunction, body);
ZEND_METHOD(Cassandra_DefaultFunction, isCalledOnNullInput);

static const zend_function_entry class_Cassandra_DefaultFunction_methods[] = {
	ZEND_ME(Cassandra_DefaultFunction, name, arginfo_class_Cassandra_DefaultFunction_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, simpleName, arginfo_class_Cassandra_DefaultFunction_simpleName, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, arguments, arginfo_class_Cassandra_DefaultFunction_arguments, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, returnType, arginfo_class_Cassandra_DefaultFunction_returnType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, signature, arginfo_class_Cassandra_DefaultFunction_signature, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, language, arginfo_class_Cassandra_DefaultFunction_language, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, body, arginfo_class_Cassandra_DefaultFunction_body, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultFunction, isCalledOnNullInput, arginfo_class_Cassandra_DefaultFunction_isCalledOnNullInput, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultFunction(zend_class_entry *class_entry_Cassandra_Function_)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultFunction", class_Cassandra_DefaultFunction_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Function_);

	return class_entry;
}
