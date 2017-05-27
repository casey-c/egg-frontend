#include "quantumbool.h"

bool QuantumBool::checkAndClear()
{
    bool ret = b;
    b = false;
    return ret;
}

bool QuantumBool::checkAndSet()
{
    bool ret = b;
    b = true;
    return ret;
}
