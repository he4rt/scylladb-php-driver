/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9f8299276fd0c3f37193b439087bb21ae630f61f */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Function__name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Function__simpleName arginfo_class_Cassandra_Function__name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Function__arguments, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Function__returnType, 0, 0, Cassandra\\Type, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Function__signature arginfo_class_Cassandra_Function__name

#define arginfo_class_Cassandra_Function__language arginfo_class_Cassandra_Function__name

#define arginfo_class_Cassandra_Function__body arginfo_class_Cassandra_Function__name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Function__isCalledOnNullInput, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_Function__methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, name, arginfo_class_Cassandra_Function__name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, simpleName, arginfo_class_Cassandra_Function__simpleName, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, arguments, arginfo_class_Cassandra_Function__arguments, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, returnType, arginfo_class_Cassandra_Function__returnType, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, signature, arginfo_class_Cassandra_Function__signature, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, language, arginfo_class_Cassandra_Function__language, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, body, arginfo_class_Cassandra_Function__body, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Function_, isCalledOnNullInput, arginfo_class_Cassandra_Function__isCalledOnNullInput, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Function_(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Function_", class_Cassandra_Function__methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
