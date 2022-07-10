#include <Cluster/Cluster.h>

#include "Builder/Builder.h"
#include "ClusterInterface/ClusterInterface.h"
#include "Default/DefaultCluster.h"

void
PhpDriverDefineCluster()
{
  PhpDriverDefineClusterInterface();

  PhpDriverDefineClusterBuilder();

  PhpDriverDefineDefaultCluster(phpDriverClusterInterfaceCe);
}