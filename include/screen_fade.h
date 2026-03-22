#pragma once
// =============================================================================
//  screen_fade.h  –  Screen fade in/out  (used by Credits.bb + Gameplay.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"

void InitFader();
void SetFadeTarget(float target, float speed);
void FaderCycle();    // call each frame; blends fadeAlpha toward fadeTarget
void DrawFade();      // draw black quad over screen using current fadeAlpha
