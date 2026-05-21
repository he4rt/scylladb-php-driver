<?php

/** @generate-class-entries */

namespace Cassandra {
    abstract class MaterializedView implements \Cassandra\Table {
        /** @return Table|null */
        abstract public function baseTable(): ?Table;
    }
}
