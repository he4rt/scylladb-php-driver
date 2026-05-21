/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 04caeed8f0ce58f9f4d25147be717e0109f3c73a */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Column_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_Column_type, 0, 0, Cassandra\\Type, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Column_isReversed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Column_isStatic arginfo_class_Cassandra_Column_isReversed

#define arginfo_class_Cassandra_Column_isFrozen arginfo_class_Cassandra_Column_isReversed

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_Column_indexName, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_Column_indexOptions arginfo_class_Cassandra_Column_indexName




static const zend_function_entry class_Cassandra_Column_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, name, arginfo_class_Cassandra_Column_name, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, type, arginfo_class_Cassandra_Column_type, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, isReversed, arginfo_class_Cassandra_Column_isReversed, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, isStatic, arginfo_class_Cassandra_Column_isStatic, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, isFrozen, arginfo_class_Cassandra_Column_isFrozen, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, indexName, arginfo_class_Cassandra_Column_indexName, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_Column, indexOptions, arginfo_class_Cassandra_Column_indexOptions, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Column(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Column", class_Cassandra_Column_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}
