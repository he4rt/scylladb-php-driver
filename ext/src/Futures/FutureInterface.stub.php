<?php

/** @generate-class-entries */

namespace Cassandra {

    interface Future
    {
        public function get(int|float|null $timeout): mixed;
    }
}