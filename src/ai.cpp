// =============================================================================
//  ai.cpp  –  Input routing + AI  (converted from AI.bb)
//
//  Key changes vs original Blitz3D:
//    KeyDown(200/208/203/205)     → left-stick Y/X axis
//    JoyYDir() / JoyXDir()        → left-stick axis (same)
//    JoyDown(buttXxx)             → SceCtrl button held
// =============================================================================
#include "ai.h"
#include "players.h"
#include "values.h"
#include "functions.h"
#include "blitz_compat.h"
#include "anims.h"
#include "world_props.h"

// (LowestValue, HighestValue, SatisfiedAngle defined in functions.cpp)

// Returns > 0 when a lockdown is in effect
// Lockdown = hours 22:00-07:00 (night) OR alarm triggered
int LockDown() {
    // Night lockdown
    if (gamHours[slot] >= 22 || gamHours[slot] < 7) return 1;
    // Alarm lockdown (gamBlackout = alarm active)
    if (gamBlackout[slot] > 0) return 1;
    return 0;
}

// Returns the prison-block index for the given location (1-based), 0 if outdoor
int GetBlock(int location) {
    if (location == 1 || location == 3 || location == 5 || location == 7) return location;
    return 0;
}

// ---------------------------------------------------------------------------
//  GetInput  –  replaces Blitz3D keyboard + JoyXxx calls
// ---------------------------------------------------------------------------
void GetInput(int cyc) {
    // Reset all command flags
    cUp[cyc] = cDown[cyc] = cLeft[cyc] = cRight[cyc] = 0;
    cAttack[cyc] = cDefend[cyc] = cThrow[cyc] = cPickUp[cyc] = 0;

    bool isHuman = (pControl[cyc] == 1 || pControl[cyc] == 2 || pControl[cyc] == 3);
    bool broken  = (charBreakdown[pChar[cyc]] > 0);

    if (isHuman && !broken) {
        // ---- Vita analogue stick (left stick) replaces KB arrows + JoyDir ----
        static constexpr float DEADZONE = 0.25f;
        float lx = gInput.LeftX();
        float ly = gInput.LeftY();

        if (ly < -DEADZONE) cUp[cyc]    = 1;
        if (ly >  DEADZONE) cDown[cyc]  = 1;
        if (lx < -DEADZONE) cLeft[cyc]  = 1;
        if (lx >  DEADZONE) cRight[cyc] = 1;

        // ---- Face buttons ----
        if (gInput.Held(static_cast<SceCtrlButtons>(buttAttack)))  cAttack[cyc] = 1;
        if (gInput.Held(static_cast<SceCtrlButtons>(buttDefend)))  cDefend[cyc] = 1;
        if (gInput.Held(static_cast<SceCtrlButtons>(buttThrow)))   cThrow[cyc]  = 1;
        if (gInput.Held(static_cast<SceCtrlButtons>(buttPickUp)))  cPickUp[cyc] = 1;
    }

    // ---- CPU AI ----
    if (pControl[cyc] == 0 || broken) {
        AI(cyc);
    }
}

// ---------------------------------------------------------------------------
//  AI  –  direct port of AI() from AI.bb
// ---------------------------------------------------------------------------
void AI(int cyc) {
    // --- Determine agenda ---
    pOldAgenda[cyc] = pAgenda[cyc];

    bool inPromo = (cyc == promoActor[1] || cyc == promoActor[2]);
    if (!inPromo) {
        int randy = Rnd(0, 1000);
        if (randy == 0) pAgenda[cyc] = 0;
        if (randy == 1) pAgenda[cyc] = 1;
        if (randy == 2 || charPromo[pChar[cyc]][gamChar[slot]] > 0 || charFollowTim[pChar[cyc]] > 0) {
            pAgenda[cyc] = 2; pFollowFoc[cyc] = 0;
        }
        if (randy == 3 || LockDown()) {
            if (charRole[pChar[cyc]] == 0 && GetBlock(gamLocation[slot]) > 0 && charFollowTim[pChar[cyc]] == 0)
                pAgenda[cyc] = 3;
        }
        if (randy == 4 || (randy == 5 && charRole[pChar[cyc]] == 1)) {
            if (pWeapon[cyc] == 0) { pAgenda[cyc] = 4; pWeapFoc[cyc] = 0; }
        }
    }

    // Avoid promo conversation area
    if (gamPromo > 0 && promoActor[1] > 0 && promoActor[2] > 0 && !inPromo) {
        float lx = LowestValue(pX[promoActor[1]], pX[promoActor[2]]);
        float hx = HighestValue(pX[promoActor[1]], pX[promoActor[2]]);
        float lz = LowestValue(pZ[promoActor[1]], pZ[promoActor[2]]);
        float hz = HighestValue(pZ[promoActor[1]], pZ[promoActor[2]]);
        if (pTX[cyc] > lx && pTX[cyc] < hx && pTZ[cyc] > lz && pTZ[cyc] < hz)
            pNowhere[cyc] = 99;
    }

    // Rethink if stuck
    if (pNowhere[cyc] > 30) {
        if (pSubX[cyc] == 9999.0f && pSubZ[cyc] == 9999.0f) {
            pSubX[cyc] = pX[cyc] + Rnd(-200, 200);
            pSubZ[cyc] = pZ[cyc] + Rnd(-200, 200);
        } else {
            pSubX[cyc] = 9999.0f; pSubZ[cyc] = 9999.0f;
        }
        pAgenda[cyc] = 1;
    }

    // --- Execute agenda ---

    // 0: Contemplate (stand still, occasionally turn)
    if (pAgenda[cyc] == 0) {
        if (Rnd(0, 200) <= 1 && pAnim[cyc] < 20) {
            float newAngle;
            do { newAngle = static_cast<float>(Rnd(0, 360)); }
            while (SatisfiedAngle(pA[cyc], newAngle, 10));
            pTA[cyc] = newAngle;
        }
        pTX[cyc] = pX[cyc]; pTY[cyc] = pY[cyc]; pTZ[cyc] = pZ[cyc];
        pSubX[cyc] = 9999.0f; pSubZ[cyc] = 9999.0f;
        // pExploreRange[cyc] = 5;  // TODO if needed
    }

    // 1: Explore
    if (pAgenda[cyc] == 1) {
        if (Rnd(0, 500) <= 1 || pNowhere[cyc] > 30 || pAgenda[cyc] != pOldAgenda[cyc]) {
            if (GetBlock(gamLocation[slot]) > 0) {
                pExploreX[cyc] = static_cast<float>(Rnd(-290, 290));
                pExploreY[cyc] = 10.0f;
                pExploreZ[cyc] = static_cast<float>(Rnd(-330, 350));
                if (Rnd(0, 1) == 0) {
                    do {
                        pExploreX[cyc] = static_cast<float>(Rnd(-290, 290));
                        pExploreY[cyc] = 105.0f;
                        pExploreZ[cyc] = static_cast<float>(Rnd(-140, 350));
                    } while (!(pExploreX[cyc] < -150 || pExploreX[cyc] > 150 || pExploreZ[cyc] > 210));
                }
                if (pExploreX[cyc] > -50 && pExploreX[cyc] < 50 &&
                    pExploreZ[cyc] > 20  && pExploreZ[cyc] < 210)
                    pExploreY[cyc] = 9999.0f;
            }
            // Outdoor explore: location 2 (yard), etc.
            if (gamLocation[slot] == 2) {
                pExploreX[cyc] = static_cast<float>(Rnd(-350, 350));
                pExploreY[cyc] = 0.0f;
                pExploreZ[cyc] = static_cast<float>(Rnd(-350, 350));
            }
        }
        pTX[cyc] = pExploreX[cyc];
        pTY[cyc] = pExploreY[cyc];
        pTZ[cyc] = pExploreZ[cyc];
    }

    // 2: Follow (shadow another character)
    if (pAgenda[cyc] == 2) {
        // Find the player character to follow
        int followTarget = gamPlayer[slot];
        if (pFollowFoc[cyc] > 0) followTarget = pFollowFoc[cyc];
        if (followTarget >= 1 && followTarget <= no_plays) {
            // Set target position slightly offset from target
            float offsetX = pX[followTarget] + Rnd(-30, 30);
            float offsetZ = pZ[followTarget] + Rnd(-30, 30);
            // Only move if not already close
            float dx = offsetX - pX[cyc], dz = offsetZ - pZ[cyc];
            if (dx*dx + dz*dz > 40.0f*40.0f) {
                pTX[cyc] = offsetX;
                pTZ[cyc] = offsetZ;
            } else {
                // Close enough – turn to face target
                FaceEntity(cyc, p[followTarget], 5.0f);
                pTX[cyc] = pX[cyc]; pTZ[cyc] = pZ[cyc];
            }
        }
    }

    // 3: Lockdown – navigate back to own cell
    if (pAgenda[cyc] == 3) {
        int cell = charCell[pChar[cyc]];
        if (cell >= 1 && cell <= 20) {
            // Cell positions from world_props
            pTX[cyc] = cellDoorX[cell];
            pTZ[cyc] = cellDoorZ[cell];
        }
    }

    // 4: Weapon seek – navigate to nearest unclaimed weapon
    if (pAgenda[cyc] == 4) {
        float bestDist = 9999.0f;
        for (int w = 1; w <= no_weaps; ++w) {
            if (weapLocation[w] != gamLocation[slot]) continue;
            if (weapState[w] == 0 || weapCarrier[w] != 0) continue;
            float dx = weapX[w] - pX[cyc], dz = weapZ[w] - pZ[cyc];
            float d = dx*dx + dz*dz;
            if (d < bestDist) {
                bestDist = d;
                pTX[cyc] = weapX[w];
                pTZ[cyc] = weapZ[w];
                pWeapFoc[cyc] = w;
            }
        }
        // Already have a weapon? Stop seeking
        if (pWeapon[cyc] > 0) pAgenda[cyc] = 0;
    }

    // Derive movement commands from target position
    float dx = pTX[cyc] - pX[cyc];
    float dz = pTZ[cyc] - pZ[cyc];
    float dist = std::sqrt(dx*dx + dz*dz);

    if (dist > 5.0f) {
        // Simple 8-directional AI movement
        float targetAngle = ATan2(dx, dz);
        float diff = CleanAngle(targetAngle - pA[cyc]);

        if (std::fabs(diff) > 20.0f) {
            cLeft[cyc]  = (diff < 0) ? 1 : 0;
            cRight[cyc] = (diff > 0) ? 1 : 0;
        }
        cUp[cyc] = 1;  // always walk forward toward target
    }
}
