<?php

/** @generate-class-entries */

namespace Cassandra {

    /**
     * @strict-properties
     */
    interface Cluster
    {

        /**
         * @ref-count 1
         */
        public function connect(): Session;

        /**
         * @ref-count 1
         */
        public function connectAsync(): Future;
    }
}