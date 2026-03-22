#pragma once
// =============================================================================
//  promos.h  –  Dialogue / promo system  (from Promos.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"

// Dialogue rendering
void Speak(int cyc, int style);     // trigger speaking anim
void PromoCycle();                  // called each frame from gameplay
void RiskPromo(int cyc, int v);     // chance to start a promo
void StartPromo(int initiator, int target, int promoId);
void ExecutePromo();                // drive active promo each tick
void EndPromo(int effect);          // apply outcome and close promo
