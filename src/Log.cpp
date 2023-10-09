#include "Log.hpp"

namespace Log
{
    const int Debug = 0;
    const int Warn = 1;
    const int Info = 2;
    const int Error = 3;
    const int None = 100;

    int globalLevel = Debug;
}