<?php

/**
 * Declaration-only stub.
 *
 * src/Custom.c registers Cassandra\Custom by hand, so this file is NOT passed
 * to php_scylladb_generate_arginfo() and produces no Custom_arginfo.h. It
 * exists so the class appears in the generated IDE stubs and so the port to
 * the canonical stub pattern has a starting point. Keep it in step with
 * php_scylladb_register_custom().
 */

declare(strict_types=1);

namespace Cassandra {
    abstract class Custom implements Value
    {
    }
}
