<?php

/** @generate-class-entries */

namespace Cassandra {
    abstract class Type
    {
        abstract public function name(): string|null;
        abstract public function __toString(): string;

        final public static function ascii(): \Cassandra\Type\Scalar {}
        final public static function bigint(): \Cassandra\Type\Scalar {}
        final public static function smallint(): \Cassandra\Type\Scalar {}
        final public static function tinyint(): \Cassandra\Type\Scalar {}
        final public static function blob(): \Cassandra\Type\Scalar {}
        final public static function boolean(): \Cassandra\Type\Scalar {}
        final public static function counter(): \Cassandra\Type\Scalar {}
        final public static function decimal(): \Cassandra\Type\Scalar {}
        final public static function double(): \Cassandra\Type\Scalar {}
        final public static function duration(): \Cassandra\Type\Scalar {}
        final public static function float(): \Cassandra\Type\Scalar {}
        final public static function int(): \Cassandra\Type\Scalar {}
        final public static function text(): \Cassandra\Type\Scalar {}
        final public static function timestamp(): \Cassandra\Type\Scalar {}
        final public static function date(): \Cassandra\Type\Scalar {}
        final public static function time(): \Cassandra\Type\Scalar {}
        final public static function uuid(): \Cassandra\Type\Scalar {}
        final public static function varchar(): \Cassandra\Type\Scalar {}
        final public static function varint(): \Cassandra\Type\Scalar {}
        final public static function timeuuid(): \Cassandra\Type\Scalar {}
        final public static function inet(): \Cassandra\Type\Scalar {}

        final public static function collection(Type $type): \Cassandra\Type\Collection {}
        final public static function set(Type $type): \Cassandra\Type\Set {}
        final public static function map(Type $keyType, Type $valueType): \Cassandra\Type\Map {}
        final public static function tuple(mixed ...$types): \Cassandra\Type\Tuple {}
        final public static function userType(mixed ...$types): \Cassandra\Type\UserType {}
    }
}
