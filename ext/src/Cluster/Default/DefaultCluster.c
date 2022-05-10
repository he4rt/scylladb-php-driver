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

#include "php_driver.h"
#include "php_driver_globals.h"
#include "php_driver_types.h"
#include "util/FutureInterface.h"
#include "util/ref.h"

#include <Cluster/Cluster.h>
#include <Futures/Futures.h>

#include "DefaultCluster_arginfo.h"

zend_class_entry* php_driver_default_cluster_ce = NULL;

static void
free_session(void* session)
{
  cass_session_free((CassSession*) session);
}

ZEND_METHOD(Cassandra_DefaultCluster, connect)
{
  char* keyspace = NULL;
  size_t keyspace_len;
  zval* timeout               = NULL;
  php_driver_session* session = NULL;
  CassFuture* future          = NULL;
  char* hash_key;
  size_t hash_key_len = 0;
  php_driver_psession* psession;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|sz", &keyspace, &keyspace_len, &timeout) == FAILURE) {
    return;
  }

  php_driver_cluster* self = PHP_DRIVER_CLUSTER_THIS();

  object_init_ex(return_value, php_driver_default_session_ce);
  session = PHP_DRIVER_GET_SESSION(return_value);

  session->default_consistency = self->default_consistency;
  session->default_page_size   = self->default_page_size;
  session->persist             = self->persist;
  session->hash_key            = self->hash_key;
  session->keyspace            = keyspace;

  if (!Z_ISUNDEF(session->default_timeout)) {
    ZVAL_COPY(&session->default_timeout, &self->default_timeout);
  }

  if (session->persist) {
    zval* le;

    hash_key_len = spprintf(&hash_key, 0, "%s:session:%s",
                            self->hash_key, SAFE_STR(keyspace));

    if (PHP5TO7_ZEND_HASH_FIND(&EG(persistent_list), hash_key, hash_key_len + 1, le) && Z_RES_P(le)->type == php_le_php_driver_session()) {
      psession         = (php_driver_psession*) Z_RES_P(le)->ptr;
      session->session = php_driver_add_ref(psession->session);
      future           = psession->future;
    }
  }

  if (future == NULL) {
    zval resource;

    session->session = php_driver_new_peref(cass_session_new(), free_session, 1);

    if (keyspace) {
      future = cass_session_connect_keyspace((CassSession*) session->session->data,
                                             self->cluster,
                                             keyspace);
    } else {
      future = cass_session_connect((CassSession*) session->session->data,
                                    self->cluster);
    }

    if (session->persist) {
      psession          = (php_driver_psession*) pecalloc(1, sizeof(php_driver_psession), 1);
      psession->session = php_driver_add_ref(session->session);
      psession->future  = future;
      ZVAL_NEW_PERSISTENT_RES(&resource, 0, psession, php_le_php_driver_session());
      PHP5TO7_ZEND_HASH_UPDATE(&EG(persistent_list), hash_key, hash_key_len + 1, &resource, sizeof(zval));
      PHP_DRIVER_G(persistent_sessions)
      ++;
    }
  }

  if (php_driver_future_wait_timed(future, timeout) == FAILURE) {
    if (session->persist) {
      efree(hash_key);
    } else {
      cass_future_free(future);
    }

    return;
  }

  if (php_driver_future_is_error(future) == FAILURE) {
    if (session->persist) {
      PHP5TO7_ZEND_HASH_DEL(&EG(persistent_list), hash_key, hash_key_len + 1);
      efree(hash_key);
    } else {
      cass_future_free(future);
    }

    return;
  }

  if (session->persist) {
    efree(hash_key);
  }
}

ZEND_METHOD(Cassandra_DefaultCluster, connectAsync)
{
  char* hash_key      = NULL;
  size_t hash_key_len = 0;
  char* keyspace      = NULL;
  size_t keyspace_len;

  if (zend_parse_parameters(ZEND_NUM_ARGS(), "|s", &keyspace, &keyspace_len) == FAILURE) {
    return;
  }

  php_driver_cluster* self = PHP_DRIVER_CLUSTER_THIS();

  object_init_ex(return_value, php_driver_future_session_ce);
  php_driver_future_session* future = PHP_DRIVER_FUTURE_SESSION_ZVAL_TO_OBJECT(return_value);

  future->persist = self->persist;

  if (self->persist) {
    zval* le;

    hash_key_len = spprintf(&hash_key, 0, "%s:session:%s",
                            self->hash_key, SAFE_STR(keyspace));

    future->session_hash_key = self->hash_key;
    future->session_keyspace = keyspace;
    future->hash_key         = hash_key;
    future->hash_key_len     = hash_key_len;

    if (PHP5TO7_ZEND_HASH_FIND(&EG(persistent_list), hash_key, hash_key_len + 1, le) && Z_RES_P(le)->type == php_le_php_driver_session()) {
      php_driver_psession* psession = (php_driver_psession*) Z_RES_P(le)->ptr;
      future->session               = php_driver_add_ref(psession->session);
      future->future                = psession->future;
      return;
    }
  }

  future->session = php_driver_new_peref(cass_session_new(), free_session, 1);

  if (keyspace) {
    future->future = cass_session_connect_keyspace((CassSession*) future->session->data,
                                                   self->cluster,
                                                   keyspace);
  } else {
    future->future = cass_session_connect((CassSession*) future->session->data,
                                          self->cluster);
  }

  if (self->persist) {
    zval resource;
    php_driver_psession* psession =
      (php_driver_psession*) pecalloc(1, sizeof(php_driver_psession), 1);
    psession->session = php_driver_add_ref(future->session);
    psession->future  = future->future;

    ZVAL_NEW_PERSISTENT_RES(&resource, 0, psession, php_le_php_driver_session());
    PHP5TO7_ZEND_HASH_UPDATE(&EG(persistent_list), hash_key, hash_key_len + 1, &resource, sizeof(zval));
    PHP_DRIVER_G(persistent_sessions)
    ++;
  }
}

static zend_object_handlers php_driver_default_cluster_handlers;

static HashTable*
php_driver_default_cluster_properties(
#if PHP_MAJOR_VERSION >= 8
  zend_object* object
#else
  zval* object
#endif
)
{
  HashTable* props = zend_std_get_properties(object);

  return props;
}

static int
php_driver_default_cluster_compare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
php_driver_default_cluster_free(zend_object* object)
{
  php_driver_cluster* self = PHP_DRIVER_CLUSTER_OBJECT(object);

  if (self->persist) {
    efree(self->hash_key);
  } else {
    if (self->cluster) {
      cass_cluster_free(self->cluster);
    }
  }

  ZVAL_DESTROY(self->default_timeout);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
php_driver_default_cluster_new(zend_class_entry* ce)
{
  php_driver_cluster* self = make(php_driver_cluster);

  self->cluster             = NULL;
  self->default_consistency = PHP_DRIVER_DEFAULT_CONSISTENCY;
  self->default_page_size   = 5000;
  self->persist             = 0;
  self->hash_key            = NULL;

  ZVAL_UNDEF(&self->default_timeout);

  zend_object_std_init(&self->zval, ce);
  self->zval.handlers = &php_driver_default_cluster_handlers;

  return &self->zval;
}

void
php_driver_define_DefaultCluster(zend_class_entry* cluster_interface)
{
  php_driver_default_cluster_ce                = register_class_Cassandra_DefaultCluster(cluster_interface);
  php_driver_default_cluster_ce->create_object = php_driver_default_cluster_new;

  memcpy(&php_driver_default_cluster_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  php_driver_default_cluster_handlers.get_properties = php_driver_default_cluster_properties;
  php_driver_default_cluster_handlers.compare        = php_driver_default_cluster_compare;
  php_driver_default_cluster_handlers.free_obj       = php_driver_default_cluster_free;
  php_driver_default_cluster_handlers.offset         = XtOffsetOf(php_driver_cluster, zval);
}
