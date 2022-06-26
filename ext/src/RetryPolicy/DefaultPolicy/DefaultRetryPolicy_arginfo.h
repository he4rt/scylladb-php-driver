/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: d8e905a00030c1fe8047b833c39f5b54c1ff014e */




static const zend_function_entry class_Cassandra_RetryPolicy_DefaultRetryPolicy_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_RetryPolicy_DefaultRetryPolicy(zend_class_entry *class_entry_Cassandra_RetryPolicy_RetryPolicyInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\RetryPolicy", "DefaultRetryPolicy", class_Cassandra_RetryPolicy_DefaultRetryPolicy_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_RetryPolicy_RetryPolicyInterface);

	return class_entry;
}
