/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 68dff8d8090398cfbc898dbb0117813b6f92b119 */

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_MaterializedView_baseTable, 0, 0, Cassandra\\Table, 1)
ZEND_END_ARG_INFO()




static const zend_function_entry class_Cassandra_MaterializedView_methods[] = {
	ZEND_ABSTRACT_ME_WITH_FLAGS(Cassandra_MaterializedView, baseTable, arginfo_class_Cassandra_MaterializedView_baseTable, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_MaterializedView(zend_class_entry *class_entry_Cassandra_Table)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "MaterializedView", class_Cassandra_MaterializedView_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Table);

	return class_entry;
}
