<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra;

interface Exception {}

namespace Cassandra\Exception;

class RuntimeException extends \RuntimeException implements \Cassandra\Exception {}

class LogicException extends \LogicException implements \Cassandra\Exception {}

class DomainException extends \DomainException implements \Cassandra\Exception {}

class InvalidArgumentException extends \InvalidArgumentException implements \Cassandra\Exception {}

class RangeException extends \RangeException implements \Cassandra\Exception {}

class DivideByZeroException extends \Cassandra\Exception\RangeException {}

class TimeoutException extends \Cassandra\Exception\RuntimeException {}

class ExecutionException extends \Cassandra\Exception\RuntimeException {}

class ValidationException extends \Cassandra\Exception\RuntimeException {}

class ProtocolException extends \Cassandra\Exception\RuntimeException {}

class AuthenticationException extends \Cassandra\Exception\RuntimeException {}

class ServerException extends \Cassandra\Exception\RuntimeException {}

class ReadTimeoutException extends \Cassandra\Exception\ExecutionException {}

class WriteTimeoutException extends \Cassandra\Exception\ExecutionException {}

class UnavailableException extends \Cassandra\Exception\ExecutionException {}

class TruncateException extends \Cassandra\Exception\ExecutionException {}

class InvalidQueryException extends \Cassandra\Exception\ValidationException {}

class InvalidSyntaxException extends \Cassandra\Exception\ValidationException {}

class UnauthorizedException extends \Cassandra\Exception\ValidationException {}

class UnpreparedException extends \Cassandra\Exception\ValidationException {}

class ConfigurationException extends \Cassandra\Exception\ValidationException {}

class AlreadyExistsException extends \Cassandra\Exception\ConfigurationException {}

class IsBootstrappingException extends \Cassandra\Exception\ServerException {}

class OverloadedException extends \Cassandra\Exception\ServerException {}
