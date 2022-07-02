#include <TimestampGenerators/TimestampGenerators.h>

#include "MonotonicGenerator/MonotonicGenerator.h"
#include "ServerSideGenerator/ServerSideGenerator.h"
#include "TimestampGeneratorInterface/TimestampGeneratorInterface.h"

void
PhpDriverDefineTimestampGenerators()
{
  PhpDriverDefineTimestampGeneratorInterface();

  PhpDriverDefineServerSideTimestampGenerator(phpDriverTimestampGeneratorInterfaceCe);
  PhpDriverDefineMonotonicTimestampGenerator(phpDriverTimestampGeneratorInterfaceCe);
}
