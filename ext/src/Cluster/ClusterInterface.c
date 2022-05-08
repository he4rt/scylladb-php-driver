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

#include "Builder/Builder.h"
#include "Default/DefaultCluster.h"

#include "ClusterInterface_arginfo.h"

zend_class_entry* php_driver_cluster_ce = NULL;

void
php_driver_define_Cluster()
{
  php_driver_cluster_ce = register_class_Cassandra_Cluster();

  php_driver_define_DefaultCluster(php_driver_cluster_ce);
  php_driver_define_ClusterBuilder();
}
