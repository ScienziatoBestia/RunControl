#ifndef UTILS
#define UTILS

#include <stdint.h>
#include <stdio.h>
#include "SharedMemorySetup.h"

int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask) ;

double getTime(); 

#endif
