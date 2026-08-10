<?php

/**
 * Copyright 2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace Cassandra;

/**
 * A PHP representation of the CQL `timestamp` datatype
 */
final class Timestamp implements Value {

    /**
     * Creates a new timestamp from a unix timestamp and microseconds, from a
     * DateTimeInterface, or from the current time by default.
     *
     * @param int|\DateTimeInterface $seconds The number of seconds, or a date and time object
     * @param int $microseconds The number of microseconds. Not allowed with a DateTimeInterface
     */
    public function __construct($seconds, $microseconds) { }

    /**
     * Creates a timestamp for the current time.
     *
     * @return static
     */
    public static function now() { }

    /**
     * Alias of now(). A timestamp holds a UTC epoch value, so it carries no zone.
     *
     * @return static
     */
    public static function nowUtc() { }

    /**
     * Creates a timestamp from a PHP date and time object.
     *
     * @param \DateTimeInterface $datetime The date and time to convert
     *
     * @return static
     */
    public static function fromDateTime($datetime) { }

    /**
     * The type of this timestamp.
     *
     * @return \Cassandra\Type
     */
    public function type() { }

    /**
     * Unix timestamp.
     *
     * @return int seconds
     *
     * @see time
     */
    public function time() { }

    /**
     * Microtime from this timestamp
     *
     * @param bool $get_as_float Whether to get this value as float
     *
     * @return float|string Float or string representation
     *
     * @see microtime
     */
    public function microtime($get_as_float) { }

    /**
     * Converts current timestamp to PHP DateTime.
     *
     * @return \DateTime PHP representation
     */
    public function toDateTime() { }

    /**
     * Returns a string representation of this timestamp.
     *
     * @return string timestamp
     */
    public function __toString() { }

}
