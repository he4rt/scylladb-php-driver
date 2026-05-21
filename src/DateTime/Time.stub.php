<?php

/**
* @generate-class-entries
*/
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Time implements Value {
        public function __construct(int|string $nanoseconds = UNKNOWN) {}

        public function type(): Type {}
        public function seconds(): int {}
        public static function fromDateTime(\DateTimeInterface $datetime): static {}

        public function __toString(): string {}
    }
}