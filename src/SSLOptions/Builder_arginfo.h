/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 61e7e5afb565fe6aa8cfcdffedcf83aa87acfc83 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_SSLOptions_Builder_withTrustedCerts, 0, 0, IS_STATIC, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_SSLOptions_Builder_withVerifyFlags, 0, 1, IS_STATIC, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_SSLOptions_Builder_withClientCert, 0, 1, IS_STATIC, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Cassandra_SSLOptions_Builder_withPrivateKey, 0, 1, IS_STATIC, 0)
	ZEND_ARG_TYPE_INFO(0, path, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, passphrase, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Cassandra_SSLOptions_Builder_build, 0, 0, Cassandra\\SSLOptions, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(Cassandra_SSLOptions_Builder, withTrustedCerts);
ZEND_METHOD(Cassandra_SSLOptions_Builder, withVerifyFlags);
ZEND_METHOD(Cassandra_SSLOptions_Builder, withClientCert);
ZEND_METHOD(Cassandra_SSLOptions_Builder, withPrivateKey);
ZEND_METHOD(Cassandra_SSLOptions_Builder, build);

static const zend_function_entry class_Cassandra_SSLOptions_Builder_methods[] = {
	ZEND_ME(Cassandra_SSLOptions_Builder, withTrustedCerts, arginfo_class_Cassandra_SSLOptions_Builder_withTrustedCerts, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_SSLOptions_Builder, withVerifyFlags, arginfo_class_Cassandra_SSLOptions_Builder_withVerifyFlags, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_SSLOptions_Builder, withClientCert, arginfo_class_Cassandra_SSLOptions_Builder_withClientCert, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_SSLOptions_Builder, withPrivateKey, arginfo_class_Cassandra_SSLOptions_Builder_withPrivateKey, ZEND_ACC_PUBLIC)
	ZEND_ME(Cassandra_SSLOptions_Builder, build, arginfo_class_Cassandra_SSLOptions_Builder_build, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_SSLOptions_Builder(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\SSLOptions", "Builder", class_Cassandra_SSLOptions_Builder_methods);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);

	return class_entry;
}
