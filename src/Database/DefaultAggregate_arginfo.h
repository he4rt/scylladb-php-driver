/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8a0376befe073a2cfb8baacf34fc53c8045604c9 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultAggregate_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultAggregate_simpleName arginfo_class_Cassandra_DefaultAggregate_name

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultAggregate_argumentTypes, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultAggregate_finalFunction, 0, 0, Cassandra\\Function_, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultAggregate_stateFunction arginfo_class_Cassandra_DefaultAggregate_finalFunction

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultAggregate_initialCondition, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultAggregate_returnType, 0, 0, Cassandra\\Type, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultAggregate_stateType arginfo_class_Cassandra_DefaultAggregate_returnType

#define arginfo_class_Cassandra_DefaultAggregate_signature arginfo_class_Cassandra_DefaultAggregate_name


ZEND_METHOD(Cassandra_DefaultAggregate, name);
ZEND_METHOD(Cassandra_DefaultAggregate, simpleName);
ZEND_METHOD(Cassandra_DefaultAggregate, argumentTypes);
ZEND_METHOD(Cassandra_DefaultAggregate, finalFunction);
ZEND_METHOD(Cassandra_DefaultAggregate, stateFunction);
ZEND_METHOD(Cassandra_DefaultAggregate, initialCondition);
ZEND_METHOD(Cassandra_DefaultAggregate, returnType);
ZEND_METHOD(Cassandra_DefaultAggregate, stateType);
ZEND_METHOD(Cassandra_DefaultAggregate, signature);


static const zend_function_entry class_Cassandra_DefaultAggregate_methods[] = {
	ZEND_ME(Cassandra_DefaultAggregate, name, arginfo_class_Cassandra_DefaultAggregate_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, simpleName, arginfo_class_Cassandra_DefaultAggregate_simpleName, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, argumentTypes, arginfo_class_Cassandra_DefaultAggregate_argumentTypes, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, finalFunction, arginfo_class_Cassandra_DefaultAggregate_finalFunction, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, stateFunction, arginfo_class_Cassandra_DefaultAggregate_stateFunction, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, initialCondition, arginfo_class_Cassandra_DefaultAggregate_initialCondition, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, returnType, arginfo_class_Cassandra_DefaultAggregate_returnType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, stateType, arginfo_class_Cassandra_DefaultAggregate_stateType, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultAggregate, signature, arginfo_class_Cassandra_DefaultAggregate_signature, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultAggregate(zend_class_entry *class_entry_Cassandra_Aggregate)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultAggregate", class_Cassandra_DefaultAggregate_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Aggregate);

	return class_entry;
}
