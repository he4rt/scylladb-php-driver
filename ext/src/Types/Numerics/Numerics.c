#include <Types/Numerics/Numerics.h>

#include "Bigint/Bigint.h"
#include "Decimal/Decimal.h"
#include "Float/Float.h"
#include "Numeric/Numeric.h"
#include "Smallint/Smallint.h"
#include "Tinyint/Tinyint.h"
#include "Varint/Varint.h"

void
PhpDriverDefineNumerics(zend_class_entry* valueInterface)
{
  PhpDriverDefineNumericInterface();

  PhpDriverDefineBigint(valueInterface, phpDriverNumericInterfaceCe);
  PhpDriverDefineDecimal(valueInterface, phpDriverNumericInterfaceCe);
  PhpDriverDefineFloat(valueInterface, phpDriverNumericInterfaceCe);
  PhpDriverDefineSmallint(valueInterface, phpDriverNumericInterfaceCe);
  PhpDriverDefineTinyint(valueInterface, phpDriverNumericInterfaceCe);
  PhpDriverDefineVarint(valueInterface, phpDriverNumericInterfaceCe);
}
