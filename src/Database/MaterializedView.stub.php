<?php

/** @generate-class-entries */

declare(strict_types=1);

namespace Cassandra {
    abstract class MaterializedView implements \Cassandra\Table {
        /** @return Table|null */
        abstract public function baseTable(): ?Table;
    }
}
