/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <php_driver.h>
#include <php_driver_globals.h>
#include <php_driver_types.h>
#include <util/future.h>
#include <util/ref.h>

#include "Cluster.h"
#include "DefaultClusterHandlers.h"

BEGIN_EXTERN_C()

#include "DefaultCluster_arginfo.h"

zend_class_entry *php_driver_default_cluster_ce = nullptr;

static void free_session(void *session)
{
    cass_session_free((CassSession *)session);
}

ZEND_METHOD(Cassandra_DefaultCluster, connect)
{
    char *keyspace = nullptr;
    size_t keyspace_len;
    zval *timeout = nullptr;
    php_driver_cluster *self;
    php_driver_session *session;
    CassFuture *future = nullptr;
    zend_string *hash_key = nullptr;
    php_driver_psession *psession;

    ZEND_PARSE_PARAMETERS_START(0, 2)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(keyspace, keyspace_len)
        Z_PARAM_ZVAL(timeout)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_DRIVER_GET_CLUSTER(getThis());

    object_init_ex(return_value, php_driver_default_session_ce);
    session = PHP_DRIVER_GET_SESSION(return_value);

    session->default_consistency = self->default_consistency;
    session->default_page_size = self->default_page_size;
    session->persist = self->persist;
    /* Refcounted copy — cluster can be GC'd before this session. */
    session->hash_key = self->hash_key ? zend_string_copy(self->hash_key) : nullptr;
    session->keyspace = keyspace ? estrndup(keyspace, keyspace_len) : nullptr;

    if (!Z_ISUNDEF(session->default_timeout))
    {
        ZVAL_COPY(&session->default_timeout, &self->default_timeout);
    }

    if (session->persist)
    {
        zval *le;

        hash_key = zend_strpprintf(0, "%s:session:%s", ZSTR_VAL(self->hash_key), SAFE_STR(keyspace));

        if ((le = zend_hash_find(&EG(persistent_list), hash_key)) != NULL &&
            Z_RES_P(le)->type == php_le_php_driver_session())
        {
            psession = (php_driver_psession *)Z_RES_P(le)->ptr;
            session->session = php_driver_add_ref(psession->session);
            future = psession->future;
        }
    }

    if (future == NULL)
    {
        zval resource;

        session->session = php_driver_new_peref(cass_session_new(), free_session, 1);

        if (keyspace)
        {
            future = cass_session_connect_keyspace((CassSession *)session->session->data, self->cluster, keyspace);
        }
        else
        {
            future = cass_session_connect((CassSession *)session->session->data, self->cluster);
        }

        if (session->persist)
        {
            psession = (php_driver_psession *)pecalloc(1, sizeof(php_driver_psession), 1);
            psession->session = php_driver_add_ref(session->session);
            psession->future = future;

            ZVAL_NEW_PERSISTENT_RES(&resource, 0, psession, php_le_php_driver_session());
            (void)zend_hash_update(&EG(persistent_list), hash_key, &resource);
            PHP_DRIVER_G(persistent_sessions)++;
        }
    }

    if (php_driver_future_wait_timed(future, timeout) == FAILURE)
    {
        if (session->persist)
        {
            /* Remove the broken/timed-out session so the next request gets a
               fresh connection attempt instead of reusing a stale future. */
            (void)(zend_hash_del(&EG(persistent_list), hash_key) == SUCCESS);
            zend_string_release(hash_key);
        }
        else
        {
            cass_future_free(future);
        }

        return;
    }

    if (php_driver_future_is_error(future) == FAILURE)
    {
        if (session->persist)
        {
            (void)(zend_hash_del(&EG(persistent_list), hash_key) == SUCCESS);
            zend_string_release(hash_key);
        }
        else
        {
            cass_future_free(future);
        }

        return;
    }

    if (session->persist && hash_key)
        zend_string_release(hash_key);
}

ZEND_METHOD(Cassandra_DefaultCluster, connectAsync)
{
    char *keyspace = NULL;
    size_t keyspace_len;
    php_driver_cluster *self = NULL;
    php_driver_future_session *future = NULL;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STRING(keyspace, keyspace_len)
    ZEND_PARSE_PARAMETERS_END();

    self = PHP_DRIVER_GET_CLUSTER(getThis());

    object_init_ex(return_value, php_driver_future_session_ce);
    future = PHP_DRIVER_GET_FUTURE_SESSION(return_value);

    future->persist = self->persist;

    if (self->persist)
    {
        zval *le;

        future->session_keyspace = keyspace ? estrndup(keyspace, keyspace_len) : nullptr;
        future->hash_key = zend_strpprintf(0, "%s:session:%s", ZSTR_VAL(self->hash_key), SAFE_STR(keyspace));

        if ((le = zend_hash_find(&EG(persistent_list), future->hash_key)) != NULL &&
            Z_RES_P(le)->type == php_le_php_driver_session())
        {
            php_driver_psession *psession = (php_driver_psession *)Z_RES_P(le)->ptr;
            future->session = php_driver_add_ref(psession->session);
            future->future = psession->future;
            return;
        }
    }

    future->session = php_driver_new_peref(cass_session_new(), free_session, 1);

    if (keyspace)
    {
        future->future = cass_session_connect_keyspace((CassSession *)future->session->data, self->cluster, keyspace);
    }
    else
    {
        future->future = cass_session_connect((CassSession *)future->session->data, self->cluster);
    }

    if (self->persist)
    {
        zval resource;
        auto *psession = (php_driver_psession *)pecalloc(1, sizeof(php_driver_psession), 1);
        psession->session = php_driver_add_ref(future->session);
        psession->future = future->future;

        ZVAL_NEW_PERSISTENT_RES(&resource, 0, psession, php_le_php_driver_session());
        (void)zend_hash_update(&EG(persistent_list), future->hash_key, &resource);
        PHP_DRIVER_G(persistent_sessions)++;
    }
}

END_EXTERN_C()

void php_driver_define_DefaultCluster()
{
    php_driver_default_cluster_ce = register_class_Cassandra_DefaultCluster(php_driver_cluster_ce);
    php_driver_initialize_default_cluster_handlers();

    php_driver_default_cluster_ce->create_object = php_driver_default_cluster_new;
}