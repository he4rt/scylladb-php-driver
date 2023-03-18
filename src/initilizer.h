#pragma once

#include <php.h>

BEGIN_EXTERN_C()
/* Exceptions */
void php_driver_define_Exception();
void php_driver_define_InvalidArgumentException();
void php_driver_define_DomainException();
void php_driver_define_LogicException();
void php_driver_define_RuntimeException();
void php_driver_define_TimeoutException();
void php_driver_define_ExecutionException();
void php_driver_define_ReadTimeoutException();
void php_driver_define_WriteTimeoutException();
void php_driver_define_UnavailableException();
void php_driver_define_TruncateException();
void php_driver_define_ValidationException();
void php_driver_define_InvalidQueryException();
void php_driver_define_InvalidSyntaxException();
void php_driver_define_UnauthorizedException();
void php_driver_define_UnpreparedException();
void php_driver_define_ConfigurationException();
void php_driver_define_AlreadyExistsException();
void php_driver_define_AuthenticationException();
void php_driver_define_ProtocolException();
void php_driver_define_ServerException();
void php_driver_define_IsBootstrappingException();
void php_driver_define_OverloadedException();
void php_driver_define_DivideByZeroException();
void php_driver_define_RangeException();

/* Types */
void php_driver_define_Value();
void php_driver_define_Numeric();
void php_driver_define_Bigint();
void php_driver_define_Smallint();
void php_driver_define_Tinyint();
void php_driver_define_Blob();
void php_driver_define_Collection();
void php_driver_define_Decimal();
void php_driver_define_Float();
void php_driver_define_Inet();
void php_driver_define_Map();
void php_driver_define_Set();
void php_driver_define_Timestamp();
void php_driver_define_Date();
void php_driver_define_Time();
void php_driver_define_Tuple();
void php_driver_define_UserTypeValue();
void php_driver_define_UuidInterface();
void php_driver_define_Uuid();
void php_driver_define_Timeuuid();
void php_driver_define_Varint();
void php_driver_define_Custom();
void php_driver_define_Duration();

void php_driver_define_Core();
void php_driver_define_Future();
void php_driver_define_FuturePreparedStatement();
void php_driver_define_FutureRows();
void php_driver_define_FutureSession();
void php_driver_define_FutureValue();
void php_driver_define_FutureClose();
void php_driver_define_Session();
void php_driver_define_DefaultSession();
void php_driver_define_SSLOptions();
void php_driver_define_SSLOptionsBuilder();
void php_driver_define_Statement();
void php_driver_define_SimpleStatement();
void php_driver_define_PreparedStatement();
void php_driver_define_BatchStatement();
void php_driver_define_ExecutionOptions();
void php_driver_define_Rows();

void php_driver_define_Schema();
void php_driver_define_DefaultSchema();
void php_driver_define_Keyspace();
void php_driver_define_DefaultKeyspace();
void php_driver_define_Table();
void php_driver_define_DefaultTable();
void php_driver_define_Column();
void php_driver_define_DefaultColumn();
void php_driver_define_Index();
void php_driver_define_DefaultIndex();
void php_driver_define_MaterializedView();
void php_driver_define_DefaultMaterializedView();
void php_driver_define_Function();
void php_driver_define_DefaultFunction();
void php_driver_define_Aggregate();
void php_driver_define_DefaultAggregate();


void php_driver_define_Type();
void php_driver_define_TypeScalar();
void php_driver_define_TypeCollection();
void php_driver_define_TypeSet();
void php_driver_define_TypeMap();
void php_driver_define_TypeTuple();
void php_driver_define_TypeUserType();
void php_driver_define_TypeCustom();

void php_driver_define_RetryPolicy();
void php_driver_define_RetryPolicyDefault();
void php_driver_define_RetryPolicyDowngradingConsistency();
void php_driver_define_RetryPolicyFallthrough();
void php_driver_define_RetryPolicyLogging();

void php_driver_define_TimestampGenerator();
void php_driver_define_TimestampGeneratorMonotonic();
void php_driver_define_TimestampGeneratorServerSide();

extern int php_le_php_driver_cluster();
extern int php_le_php_driver_session();
extern int php_le_php_driver_prepared_statement();

END_EXTERN_C()