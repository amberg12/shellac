#include <iostream>

#include "defs.h"
#include "uci.h"

int main()
{
    std::cout << "Shellac " << shellac::BuildIdentifier << std::endl;
    shellac::UciEngine uciEngine;
    uciEngine.loop();
}
