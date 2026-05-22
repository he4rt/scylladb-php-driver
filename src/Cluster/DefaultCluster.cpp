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
#include <php_driver_cache_key.h>
#include <php_driver_globals.h>
#include <php_driver_types.h>
#include "../FutureUtil.h"

#include "Cluster.h"
#include "DefaultClusterHandlers.h"

BEGIN_EXTERN_C()

#include "DefaultCluster_arginfo.h"

zend_class_entry *php_driver_default_cluster_ce = nullptr;

ZEND_METHOD(Cassandra_DefaultCluster, connect)
{
    char *keyspace = nullptr;
    size_t keyspace_len;
    zval *timeout = nullptr;
    php_driver_cluster *self;
    php_driver_session *session;
    CassFuture *future = nullptr;
    zend_ulong cache_key = 0;
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
    /* Derive the session cache_key from the parent cluster + keyspace.
       Carrying just a uint64_t avoids string lifetime concerns when the
       cluster is dropped before this session. */
    session->cache_key = self->cache_key;
    session->keyspace = keyspace ? estrndup(keyspace, keyspace_len) : nullptr;

    if (!Z_ISUNDEF(session->default_timeout))
    {
        ZVAL_COPY(&session->default_timeout, &self->default_timeout);
    }

    if (session->persist)
    {
        /* Integer-keyed: mix the cluster's cache_key with the keyspace
           string. No allocation per call, even on cache hit. */
        cache_key = php_driver_cache_key_mix_ulong(php_driver_cache_key_init(), self->cache_key);
        cache_key = php_driver_cache_key_mix_cstr(cache_key, ":session:");
        cache_key = php_driver_cache_key_mix_cstr(cache_key, SAFE_STR(keyspace));
        session->cache_key = cache_key;

        zval *le = zend_hash_index_find(&EG(persistent_list), cache_key);
        if (le != NULL && Z_RES_P(le)->type == php_le_php_driver_session())
        {
            psession = (php_driver_psession *)Z_RES_P(le)->ptr;
            session->session = psession->session;   /* borrowed; psession owns */
            future = psession->future;
        }
    }

    if (future == NULL)
    {
        zval resource;

        session->session = cass_session_new();

        if (keyspace)
        {
            future = cass_session_connect_keyspace(session->session, self->cluster, keyspace);
        }
        else
        {
            future = cass_session_connect(session->session, self->cluster);
        }

        if (session->persist)
        {
            psession = (php_driver_psession *)pecalloc(1, sizeof(php_driver_psession), 1);
            psession->session = session->session;   /* psession becomes the owner */
            psession->future = future;

            ZVAL_NEW_PERSISTENT_RES(&resource, 0, psession, php_le_php_driver_session());
            (void)zend_hash_index_update(&EG(persistent_list), cache_key, &resource);
            PHP_DRIVER_G(persistent_sessions)++;
        }
    }

    if (php_driver_future_wait_timed(future, timeout) == FAILURE)
    {
        if (session->persist)
        {
            /* Remove the broken/timed-out session so the next request gets
               a fresh connection attempt instead of reusing a stale future. */
            (void)zend_hash_index_del(&EG(persistent_list), cache_key);
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
            (void)zend_hash_index_del(&EG(persistent_list), cache_key);
        }
        else
        {
            cass_future_free(future);
        }
        return;
    }
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
        future->session_keyspace = keyspace ? estrndup(keyspace, keyspace_len) : nullptr;
        /* Same cache_key derivation as the sync connect path so a cache
           entry created via either API is found by both. */
        future->cache_key = php_driver_cache_key_mix_ulong(php_driver_cache_key_init(), self->cache_key);
        future->cache_key = php_driver_cache_key_mix_cstr(future->cache_key, ":session:");
        future->cache_key = php_driver_cache_key_mix_cstr(future->cache_key, SAFE_STR(keyspace));

        zval *le = zend_hash_index_find(&EG(persistent_list), future->cache_key);
        if (le != NULL && Z_RES_P(le)->type == php_le_php_driver_session())
        {
            php_driver_psession *psession = (php_driver_psession *)Z_RES_P(le)->ptr;
            future->session = psession->session;   /* borrowed; psession owns */
            future->future = psession->future;
            return;
        }
    }

    future->session = cass_session_new();

    if (keyspace)
    {
        future->future = cass_session_connect_keyspace(future->session, self->cluster, keyspace);
    }
    else
    {
        future->future = cass_session_connect(future->session, self->cluster);
    }

    if (self->persist)
    {
        zval resource;
        auto *psession = (php_driver_psession *)pecalloc(1, sizeof(php_driver_psession), 1);
        psession->session = future->session;   /* psession becomes the owner */
        psession->future = future->future;

        ZVAL_NEW_PERSISTENT_RES(&resource, 0, psession, php_le_php_driver_session());
        (void)zend_hash_index_update(&EG(persistent_list), future->cache_key, &resource);
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