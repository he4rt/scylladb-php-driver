/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b570b7ebb6b0aee3083e7db61b2c00c8c93f9517 */

static zend_class_entry *register_class_Cassandra_Exception(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Exception", NULL);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_RuntimeException(zend_class_entry *class_entry_RuntimeException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "RuntimeException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_RuntimeException, 0);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_LogicException(zend_class_entry *class_entry_LogicException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "LogicException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_LogicException, 0);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_DomainException(zend_class_entry *class_entry_DomainException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "DomainException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_DomainException, 0);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_InvalidArgumentException(zend_class_entry *class_entry_InvalidArgumentException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "InvalidArgumentException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_InvalidArgumentException, 0);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_RangeException(zend_class_entry *class_entry_RangeException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "RangeException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_RangeException, 0);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_DivideByZeroException(zend_class_entry *class_entry_Cassandra_Exception_RangeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "DivideByZeroException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RangeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_TimeoutException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "TimeoutException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RuntimeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ExecutionException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ExecutionException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RuntimeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ValidationException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ValidationException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RuntimeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ProtocolException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ProtocolException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RuntimeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_AuthenticationException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "AuthenticationException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RuntimeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ServerException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ServerException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_RuntimeException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ReadTimeoutException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ReadTimeoutException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ExecutionException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_WriteTimeoutException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "WriteTimeoutException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ExecutionException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_UnavailableException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "UnavailableException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ExecutionException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_TruncateException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "TruncateException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ExecutionException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_InvalidQueryException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "InvalidQueryException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ValidationException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_InvalidSyntaxException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "InvalidSyntaxException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ValidationException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_UnauthorizedException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "UnauthorizedException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ValidationException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_UnpreparedException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "UnpreparedException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ValidationException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ConfigurationException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ConfigurationException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ValidationException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_AlreadyExistsException(zend_class_entry *class_entry_Cassandra_Exception_ConfigurationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "AlreadyExistsException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ConfigurationException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_IsBootstrappingException(zend_class_entry *class_entry_Cassandra_Exception_ServerException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "IsBootstrappingException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ServerException, 0);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_OverloadedException(zend_class_entry *class_entry_Cassandra_Exception_ServerException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "OverloadedException", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, class_entry_Cassandra_Exception_ServerException, 0);

	return class_entry;
}
