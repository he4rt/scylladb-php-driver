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

#include <CassandraDriverAPI.h>
#include <Cluster/Cluster.h>
#include <Futures/Futures.h>

#include "DefaultCluster.h"
#include "DefaultCluster_arginfo.h"

PHP_DRIVER_API zend_class_entry* phpDriverDefaultClusterCe = NULL;

static void
FreeSession(void* session)
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

  ZEND_PARSE_PARAMETERS_START(0, 2)
  Z_PARAM_STRING(keyspace, keyspace_len)
  Z_PARAM_ZVAL(timeout)
  ZEND_PARSE_PARAMETERS_END();

  PhpDriverCluster* self = PHP_DRIVER_CLUSTER_THIS();

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

    if (zend_hash_str_find(&EG(persistent_list), hash_key, hash_key_len) && Z_RES_P(le)->type == php_le_php_driver_session()) {
      psession         = (php_driver_psession*) Z_RES_P(le)->ptr;
      session->session = php_driver_add_ref(psession->session);
      future           = psession->future;
    }
  }

  if (future == NULL) {
    zval resource;

    session->session = php_driver_new_peref(cass_session_new(), FreeSession, 1);

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
      zend_hash_str_del(&EG(persistent_list), hash_key, hash_key_len);
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

  ZEND_PARSE_PARAMETERS_START(0, 1)
  Z_PARAM_STRING(keyspace, keyspace_len)
  ZEND_PARSE_PARAMETERS_END();

  PhpDriverCluster* self = PHP_DRIVER_CLUSTER_THIS();

  object_init_ex(return_value, php_driver_future_session_ce);
  php_driver_future_session* future = PHP_DRIVER_FUTURE_SESSION_ZVAL_TO_OBJECT(return_value);

  future->persist = self->persist;

  if (self->persist) {
    zval* le;

    hash_key_len = spprintf(&hash_key, 0, "%s:session:%s",
                            self->hash_key, keyspace ? keyspace : "");

    future->session_hash_key = self->hash_key;
    future->session_keyspace = keyspace;
    future->hash_key         = hash_key;
    future->hash_key_len     = hash_key_len;

    if (zend_hash_str_find(&EG(persistent_list), hash_key, hash_key_len) && Z_RES_P(le)->type == php_le_php_driver_session()) {
      php_driver_psession* psession = (php_driver_psession*) Z_RES_P(le)->ptr;
      future->session               = php_driver_add_ref(psession->session);
      future->future                = psession->future;
      return;
    }
  }

  future->session = php_driver_new_peref(cass_session_new(), FreeSession, 1);

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
    zend_hash_str_update(&EG(persistent_list), hash_key, hash_key_len + 1, &resource);
    PHP_DRIVER_G(persistent_sessions)
    ++;
  }
}

static zend_object_handlers phpDriverDefaultClusterHandlers;

static HashTable*
PhpDriverDefaultClusterProperties(zend_object* object)
{
  HashTable* props = zend_std_get_properties(object);

  return props;
}

static int
PhpDriverDefaultClusterCompare(zval* obj1, zval* obj2)
{
  ZEND_COMPARE_OBJECTS_FALLBACK(obj1, obj2);

  if (Z_OBJCE_P(obj1) != Z_OBJCE_P(obj2)) {
    return 1; /* different classes */
  }

  return Z_OBJ_HANDLE_P(obj1) != Z_OBJ_HANDLE_P(obj1);
}

static void
PhpDriverDefaultClusterFree(zend_object* object)
{
  PhpDriverCluster* self = PHP_DRIVER_CLUSTER_OBJECT(object);

  if (self->persist) {
    efree(self->hash_key);
  } else if (self->cluster) {
    cass_cluster_free(self->cluster);
  }

  ZVAL_DESTROY(self->default_timeout);

  zend_object_std_dtor(&self->zval);
}

static zend_object*
PhpDriverDefaultClusterNew(zend_class_entry* ce)
{
  PhpDriverCluster* self = make(PhpDriverCluster);

  self->cluster             = NULL;
  self->default_consistency = PHP_DRIVER_DEFAULT_CONSISTENCY;
  self->default_page_size   = 5000;
  self->persist             = 0;
  self->hash_key            = NULL;

  ZVAL_UNDEF(&self->default_timeout);

  zend_object_std_init(&self->zval, ce);
  self->zval.handlers = &phpDriverDefaultClusterHandlers;

  return &self->zval;
}

void
PhpDriverDefineDefaultCluster(zend_class_entry* cluster_interface)
{
  phpDriverDefaultClusterCe                = register_class_Cassandra_Cluster_DefaultCluster(cluster_interface);
  phpDriverDefaultClusterCe->create_object = PhpDriverDefaultClusterNew;

  memcpy(&phpDriverDefaultClusterHandlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
  phpDriverDefaultClusterHandlers.get_properties = PhpDriverDefaultClusterProperties;
  phpDriverDefaultClusterHandlers.compare        = PhpDriverDefaultClusterCompare;
  phpDriverDefaultClusterHandlers.free_obj       = PhpDriverDefaultClusterFree;
  phpDriverDefaultClusterHandlers.offset         = XtOffsetOf(PhpDriverCluster, zval);
}
