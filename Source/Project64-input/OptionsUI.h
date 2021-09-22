#pragma once
#include <stdint.h>
#if defined(LEGACY)
#include "ControllerSpec1.0.h"
#else
#include "ControllerSpec1.1.h"
#endif
#include "N64Controller.h"

void ConfigOption(uint32_t ControlIndex, CONTROL & ControlInfo, N64CONTROLLER & Controller);
