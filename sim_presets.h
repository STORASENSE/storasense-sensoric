#pragma once
#include "sim_generator.h"

namespace SimPreset {
  const SimParams TEMP  {22.0f,0.2f,   8, 12, 100000,30000, 5000,   0, 0.01f, 3, 3};
  const SimParams HUMI  {45.0f,1.5f,  25, 40, 100000,30000, 5000,   0, 0.05f, 3, 3};
  const SimParams GAS   {80.0f,5.0f, 200,400, 100000,30000, 5000,180000,0.01f, 3, 3}; // warmup 3min
  const SimParams ULTRA {100.0f,1.0f,-85,-70, 100000,30000, 5000,   0, 0.02f, 3, 3}; // negative Offset = „näher“
}