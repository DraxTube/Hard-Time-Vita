#pragma once
// =============================================================================
//  ai.h  –  AI & input declarations  (from AI.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"

// AI helpers
float LowestValue(float a, float b);
float HighestValue(float a, float b);
int   SatisfiedAngle(float current, float target, int tolerance);
int   LockDown();
int   GetBlock(int location);

// Main AI/input functions
void GetInput(int cyc);
void AI(int cyc);
