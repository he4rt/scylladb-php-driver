<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    interface Future {
        public function get(int|float|null $timeout = null): mixed;
    }
}
