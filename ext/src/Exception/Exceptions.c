#include <Exception/Exceptions.h>

#include "AlreadyExistsException/AlreadyExistsException.h"
#include "AuthenticationException/AuthenticationException.h"
#include "ConfigurationException/ConfigurationException.h"
#include "DivideByZeroException/DivideByZeroException.h"
#include "DomainException/DomainException.h"
#include "ExceptionInterface/ExceptionInterface.h"
#include "ExecutionException/ExecutionException.h"
#include "InvalidQueryException/InvalidQueryException.h"
#include "InvalidSyntaxException/InvalidSyntaxException.h"
#include "IsBootstrappingException/IsBootstrappingException.h"
#include "OverloadedException/OverloadedException.h"
#include "ProtocolException/ProtocolException.h"
#include "ServerException/ServerException.h"
#include "TimeoutExceptions/ReadTimeoutException/ReadTimeoutException.h"
#include "TimeoutExceptions/TimeoutException.h"
#include "TimeoutExceptions/WriteTimeoutException/WriteTimeoutException.h"
#include "TruncateException/TruncateException.h"
#include "UnauthorizedException/UnauthorizedException.h"
#include "UnavailableException/UnavailableException.h"
#include "UnpreparedException/UnpreparedException.h"
#include "ValidationException/ValidationException.h"

void
PhpDriverDefineExceptions()
{
  PhpDriverDefineExceptionInterface();

  PhpDriverDefineAlreadyExistsException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineAuthenticationException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineValidationException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineDomainException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineExecutionException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineServerException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineDivideByZeroException(phpDriverExceptionInterfaceCe);
  PhpDriverDefineIsBootstrappingException(phpDriverExceptionInterfaceCe, phpDriverServerExceptionCe);
  PhpDriverDefineOverloadedException(phpDriverExceptionInterfaceCe, phpDriverServerExceptionCe);
  PhpDriverDefineTruncateException(phpDriverExceptionInterfaceCe, phpDriverServerExceptionCe);
  PhpDriverDefineProtocolException(phpDriverExceptionInterfaceCe, phpDriverServerExceptionCe);
  PhpDriverDefineTimeoutException(phpDriverExceptionInterfaceCe, phpDriverExecutionExceptionCe);
  PhpDriverDefineWriteTimeoutException(phpDriverExceptionInterfaceCe, phpDriverTimeoutExceptionCe);
  PhpDriverDefineReadTimeoutException(phpDriverExceptionInterfaceCe, phpDriverTimeoutExceptionCe);
  PhpDriverDefineUnavailableException(phpDriverExceptionInterfaceCe, phpDriverExecutionExceptionCe);
  PhpDriverDefineInvalidQueryException(phpDriverExceptionInterfaceCe, phpDriverValidationExceptionCe);
  PhpDriverDefineConfigurationException(phpDriverExceptionInterfaceCe, phpDriverValidationExceptionCe);
  PhpDriverDefineUnauthorizedException(phpDriverExceptionInterfaceCe, phpDriverValidationExceptionCe);
  PhpDriverDefineInvalidSyntaxException(phpDriverExceptionInterfaceCe, phpDriverValidationExceptionCe);
  PhpDriverDefineUnpreparedException(phpDriverExceptionInterfaceCe, phpDriverValidationExceptionCe);
}