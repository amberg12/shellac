#include <iostream>

#include "defs.h"
#include "uci.h"

int main()
{
    shellac::init_hash();
    shellac::init_magics();

    std::cout << "Shellac " << shellac::BuildIdentifier << std::endl;
    shellac::UciEngine uciEngine;
    uciEngine.loop();
}
