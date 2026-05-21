/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 9122af5a2e259072037be5cb40a9da2864eedce8 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultColumn_name, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_DefaultColumn_type, 0, 0, Cassandra\\Type, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultColumn_isReversed, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultColumn_isStatic arginfo_class_Cassandra_DefaultColumn_isReversed

#define arginfo_class_Cassandra_DefaultColumn_isFrozen arginfo_class_Cassandra_DefaultColumn_isReversed

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_DefaultColumn_indexName, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Cassandra_DefaultColumn_indexOptions arginfo_class_Cassandra_DefaultColumn_indexName


ZEND_METHOD(Cassandra_DefaultColumn, name);
ZEND_METHOD(Cassandra_DefaultColumn, type);
ZEND_METHOD(Cassandra_DefaultColumn, isReversed);
ZEND_METHOD(Cassandra_DefaultColumn, isStatic);
ZEND_METHOD(Cassandra_DefaultColumn, isFrozen);
ZEND_METHOD(Cassandra_DefaultColumn, indexName);
ZEND_METHOD(Cassandra_DefaultColumn, indexOptions);


static const zend_function_entry class_Cassandra_DefaultColumn_methods[] = {
	ZEND_ME(Cassandra_DefaultColumn, name, arginfo_class_Cassandra_DefaultColumn_name, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultColumn, type, arginfo_class_Cassandra_DefaultColumn_type, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultColumn, isReversed, arginfo_class_Cassandra_DefaultColumn_isReversed, ZEND_ACC_PUBLIC|ZEND_ACC_DEPRECATED)
	ZEND_ME(Cassandra_DefaultColumn, isStatic, arginfo_class_Cassandra_DefaultColumn_isStatic, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultColumn, isFrozen, arginfo_class_Cassandra_DefaultColumn_isFrozen, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultColumn, indexName, arginfo_class_Cassandra_DefaultColumn_indexName, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_DefaultColumn, indexOptions, arginfo_class_Cassandra_DefaultColumn_indexOptions, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_DefaultColumn(zend_class_entry *class_entry_Cassandra_Column)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "DefaultColumn", class_Cassandra_DefaultColumn_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Column);

	return class_entry;
}
