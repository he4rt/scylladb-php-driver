<?php

/** @generate-class-entries */

namespace Cassandra\Cluster {

    /**
     * @strict-properties
     */
    interface ClusterInterface
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