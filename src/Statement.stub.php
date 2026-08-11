<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    interface Statement
    {
        /**
         * Marks the statement as safe to run more than once.
         *
         * The driver only retries a statement after a timeout, and only runs it
         * speculatively, when the statement is idempotent.
         */
        public function setIdempotent(bool $idempotent = true): static;

        /** Returns null when the statement carries no explicit setting. */
        public function isIdempotent(): ?bool;
    }
}
