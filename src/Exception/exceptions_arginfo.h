/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: b570b7ebb6b0aee3083e7db61b2c00c8c93f9517 */




static const zend_function_entry class_Cassandra_Exception_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_RuntimeException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_LogicException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_DomainException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_InvalidArgumentException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_RangeException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_DivideByZeroException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_TimeoutException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_ExecutionException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_ValidationException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_ProtocolException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_AuthenticationException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_ServerException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_ReadTimeoutException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_WriteTimeoutException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_UnavailableException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_TruncateException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_InvalidQueryException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_InvalidSyntaxException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_UnauthorizedException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_UnpreparedException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_ConfigurationException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_AlreadyExistsException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_IsBootstrappingException_methods[] = {
	ZEND_FE_END
};


static const zend_function_entry class_Cassandra_Exception_OverloadedException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exception(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "Exception", class_Cassandra_Exception_methods);
	class_entry = zend_register_internal_interface(&ce);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_RuntimeException(zend_class_entry *class_entry_RuntimeException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "RuntimeException", class_Cassandra_Exception_RuntimeException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_RuntimeException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_LogicException(zend_class_entry *class_entry_LogicException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "LogicException", class_Cassandra_Exception_LogicException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_LogicException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_DomainException(zend_class_entry *class_entry_DomainException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "DomainException", class_Cassandra_Exception_DomainException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_DomainException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_InvalidArgumentException(zend_class_entry *class_entry_InvalidArgumentException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "InvalidArgumentException", class_Cassandra_Exception_InvalidArgumentException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_InvalidArgumentException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_RangeException(zend_class_entry *class_entry_RangeException, zend_class_entry *class_entry_Cassandra_Exception)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "RangeException", class_Cassandra_Exception_RangeException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_RangeException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exception);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_DivideByZeroException(zend_class_entry *class_entry_Cassandra_Exception_RangeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "DivideByZeroException", class_Cassandra_Exception_DivideByZeroException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RangeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_TimeoutException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "TimeoutException", class_Cassandra_Exception_TimeoutException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ExecutionException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ExecutionException", class_Cassandra_Exception_ExecutionException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ValidationException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ValidationException", class_Cassandra_Exception_ValidationException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ProtocolException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ProtocolException", class_Cassandra_Exception_ProtocolException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_AuthenticationException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "AuthenticationException", class_Cassandra_Exception_AuthenticationException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ServerException(zend_class_entry *class_entry_Cassandra_Exception_RuntimeException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ServerException", class_Cassandra_Exception_ServerException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_RuntimeException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ReadTimeoutException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ReadTimeoutException", class_Cassandra_Exception_ReadTimeoutException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ExecutionException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_WriteTimeoutException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "WriteTimeoutException", class_Cassandra_Exception_WriteTimeoutException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ExecutionException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_UnavailableException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "UnavailableException", class_Cassandra_Exception_UnavailableException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ExecutionException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_TruncateException(zend_class_entry *class_entry_Cassandra_Exception_ExecutionException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "TruncateException", class_Cassandra_Exception_TruncateException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ExecutionException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_InvalidQueryException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "InvalidQueryException", class_Cassandra_Exception_InvalidQueryException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ValidationException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_InvalidSyntaxException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "InvalidSyntaxException", class_Cassandra_Exception_InvalidSyntaxException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ValidationException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_UnauthorizedException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "UnauthorizedException", class_Cassandra_Exception_UnauthorizedException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ValidationException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_UnpreparedException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "UnpreparedException", class_Cassandra_Exception_UnpreparedException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ValidationException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_ConfigurationException(zend_class_entry *class_entry_Cassandra_Exception_ValidationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "ConfigurationException", class_Cassandra_Exception_ConfigurationException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ValidationException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_AlreadyExistsException(zend_class_entry *class_entry_Cassandra_Exception_ConfigurationException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "AlreadyExistsException", class_Cassandra_Exception_AlreadyExistsException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ConfigurationException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_IsBootstrappingException(zend_class_entry *class_entry_Cassandra_Exception_ServerException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "IsBootstrappingException", class_Cassandra_Exception_IsBootstrappingException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ServerException);

	return class_entry;
}

static zend_class_entry *register_class_Cassandra_Exception_OverloadedException(zend_class_entry *class_entry_Cassandra_Exception_ServerException)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exception", "OverloadedException", class_Cassandra_Exception_OverloadedException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exception_ServerException);

	return class_entry;
}
