/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 1097f96a2071df6563adfafa16189da5f32a655d */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Aggregate_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Aggregate_simpleName arginfo_class_Cassandra_Aggregate_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Aggregate_argumentTypes, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Aggregate_finalFunction, 0, 0, Cassandra\\Function_, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Aggregate_stateFunction arginfo_class_Cassandra_Aggregate_finalFunction

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Aggregate_initialCondition, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Aggregate_returnType, 0, 0, Cassandra\\Type, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Aggregate_stateType arginfo_class_Cassandra_Aggregate_returnType

#define arginfo_class_Cassandra_Aggregate_signature arginfo_class_Cassandra_Aggregate_name


static const zend_function_entry class_Cassandra_Aggregate_methods[] = {
	ZEND_RAW_FENTRY("name", NULL, arginfo_class_Cassandra_Aggregate_name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("simpleName", NULL, arginfo_class_Cassandra_Aggregate_simpleName, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("argumentTypes", NULL, arginfo_class_Cassandra_Aggregate_argumentTypes, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("finalFunction", NULL, arginfo_class_Cassandra_Aggregate_finalFunction, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("stateFunction", NULL, arginfo_class_Cassandra_Aggregate_stateFunction, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("initialCondition", NULL, arginfo_class_Cassandra_Aggregate_initialCondition, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("returnType", NULL, arginfo_class_Cassandra_Aggregate_returnType, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("stateType", NULL, arginfo_class_Cassandra_Aggregate_stateType, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_RAW_FENTRY("signature", NULL, arginfo_class_Cassandra_Aggregate_signature, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT, NULL, NULL)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Aggregate(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Aggregate", class_Cassandra_Aggregate_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
