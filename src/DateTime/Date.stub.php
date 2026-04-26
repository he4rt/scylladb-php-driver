<?php

/**
* @generate-class-entries
*/
namespace Cassandra {
    /**
     * @strict-properties
     */
    final class Date implements Value {
        public function __construct(int|string $value = UNKNOWN) {}

        public static function fromDateTime(\DateTimeInterface $datetime): static {}
        public function toDateTime(?Time $time = null): \DateTime {}
        public function seconds(): int {}
        public function type(): Type {}

        public function __toString(): string {}
    }
}