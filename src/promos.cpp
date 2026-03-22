#include "ai.h"
// =============================================================================
//  promos.cpp  –  Dialogue / promo system  (ported from Promos.bb)
//
//  Hard Time has 300 scripted interactions. Each is driven by:
//    gamPromo       = active promo ID (0 = none)
//    promoTim       = tick counter since start
//    promoStage     = 0=intro, 1=option, 2=pos response, 3=neg response
//    promoActor[1]  = initiator player index
//    promoActor[2]  = target player index
//
//  On Vita: dialogue text is drawn with Outline() over the 3D scene.
//  Player chooses A (accept/positive) = Cross, B (decline/negative) = Circle.
// =============================================================================
#include "promos.h"
#include "players.h"
#include "values.h"
#include "anims.h"
#include "crimes.h"
#include "functions.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <vitaGL.h>

// ---------------------------------------------------------------------------
//  Speak  –  trigger speaking animation on player cyc
// ---------------------------------------------------------------------------
void Speak(int cyc, int style) {
    // style 1 = assertive (hands on hips), 2 = gesturing
    if (pSpeaking[cyc] != style) {
        pSpeaking[cyc] = style;
        int anim = (style == 1) ? 122 : 121;
        Animate(p[cyc], 1, RndF(0.25f, 0.5f), pSeq[cyc][anim], 10.0f);
        pState[cyc] = anim;
    }
}

// ---------------------------------------------------------------------------
//  StartPromo  –  begin a scripted interaction
// ---------------------------------------------------------------------------
void StartPromo(int initiator, int target, int promoId) {
    gamPromo       = promoId;
    promoTim       = 0;
    promoStage     = 0;
    promoActor[1]  = initiator;
    promoActor[2]  = target;
    pSpeaking[initiator] = 0;
    pSpeaking[target]    = 0;

    // Face each other
    FaceEntity(initiator, p[target], 1.0f);
    FaceEntity(target,    p[initiator], 1.0f);
}

// ---------------------------------------------------------------------------
//  RiskPromo  –  chance for cyc to initiate an interaction with v
//  (Abbreviated from the full 300-promo Promos.bb; key triggers are here)
// ---------------------------------------------------------------------------
void RiskPromo(int cyc, int v) {
    if (gamPromo != 0) return;
    if (pChar[v] != gamChar[slot]) return;  // only trigger toward the player
    if (AttackViable(cyc) < 1 || pDazed[cyc] > 0) return;

    float talkRange = (pSeat[cyc] > 0) ? 100.0f : 60.0f;

    // --- Law enforcement triggers ---
    if (charRole[pChar[cyc]] == 1 && charRole[pChar[v]] == 0 &&
        gamBlackout[slot] == 0) {

        // Player carrying illegal weapon
        if (pWeapon[v] > 0 && weapValue[weapType[pWeapon[v]]] > 0 &&
            InLine(cyc, p[v], talkRange) && charBribeTim[pChar[cyc]] == 0 &&
            promoUsed[1] == 0) {
            StartPromo(cyc, v, 1);  return;
        }

        // Caught out of cell during lockdown
        if (LockDown() && GetBlock(gamLocation[slot]) > 0 &&
            charBribeTim[pChar[cyc]] == 0 && InProximity(cyc, v, 50) &&
            promoUsed[2] == 0) {
            StartPromo(cyc, v, 2);  return;
        }

        // Warden assigns mission  (rare random trigger)
        if (gamWarrant[slot] == 0 && gamMission[slot] == 0 &&
            charSentence[pChar[v]] > 0 && InProximity(cyc, v, 30) &&
            InLine(cyc, p[v], talkRange)) {
            if (Rnd(0,10000) == 1 && charStrength[pChar[v]] <= 80) {
                StartPromo(cyc, v, 141); return;  // acquire strength mission
            }
        }

        // Sentence served → discharge options
        if (charSentence[pChar[v]] <= 0 && gamWarrant[slot] == 0 &&
            InProximity(cyc, v, 30) && promoUsed[60] == 0) {
            StartPromo(cyc, v, 60); return;
        }
    }

    // --- Fellow prisoner triggers ---
    if (charRole[pChar[cyc]] == 0 && charFollowTim[pChar[cyc]] == 0 &&
        AttackViable(v) >= 1 && AttackViable(v) <= 2) {

        if (charRelation[pChar[cyc]][pChar[v]] < 0 &&
            InProximity(cyc, v, 40) && Rnd(0,2000) == 0) {
            StartPromo(cyc, v, 100);  // threaten player
            return;
        }
        if (charRelation[pChar[cyc]][pChar[v]] >= 0 &&
            InProximity(cyc, v, 30) && Rnd(0,5000) == 0 &&
            charGang[pChar[cyc]] > 0 && charGang[pChar[v]] == 0) {
            StartPromo(cyc, v, 115);  // gang recruitment
            return;
        }
    }
}

// ---------------------------------------------------------------------------
//  ExecutePromo  –  drive the active dialogue tick by tick
// ---------------------------------------------------------------------------
void ExecutePromo() {
    if (gamPromo == 0) return;

    int cyc = promoActor[1];  // initiator
    int v   = promoActor[2];  // target (player)

    // Bail if either actor is invalid
    if (cyc < 1 || cyc > no_plays || v < 1 || v > no_plays) {
        gamPromo = 0; return;
    }

    // Player input to choose option
    bool chooseA = gInput.Pressed(SCE_CTRL_CROSS);
    bool chooseB = gInput.Pressed(SCE_CTRL_CIRCLE);

    // Advance stage on timer even without input
    if (promoTim > 300 && promoStage == 0) promoStage = 1;
    if (promoTim > 600 && promoStage == 1 && !chooseA && !chooseB) {
        // Auto-choose if player ignores
        if (charRelation[pChar[v]][pChar[cyc]] > 0) chooseA = true;
        else chooseB = true;
    }

    // -----------------------------------------------------------------------
    //  Render dialogue subtitles in 2D overlay (called from gameplay render)
    // -----------------------------------------------------------------------
    // (This is called via PromoCycle → ExecutePromo, and the overlay
    //  is drawn in gameplay.cpp's HUD pass)

    // -----------------------------------------------------------------------
    //  Per-promo logic
    // -----------------------------------------------------------------------
    switch (gamPromo) {

        // --- 1: Caught with weapon ---
        case 1:
            if (promoTim < 300) { Speak(cyc, 1); }
            if (promoStage == 1) {
                optionA = "Hand it over.";
                optionB = "Mind your business.";
            }
            if (promoStage == 1 && chooseA) {
                DropWeapon(v, 0);
                promoUsed[1] = 1;
                EndPromo(0);
            }
            if (promoStage == 1 && chooseB) {
                gamWarrant[slot] += 5;
                promoUsed[1] = 1;
                EndPromo(0);
            }
            break;

        // --- 2: Out of block during lockdown ---
        case 2:
            if (promoTim < 300) Speak(cyc, 1);
            if (promoStage == 1) {
                optionA = "I'll go back.";
                optionB = "I'm not moving.";
            }
            if (promoStage == 1 && chooseA) {
                screen = 50;  // TODO: navigate to own block
                promoUsed[2] = 1; EndPromo(0);
            }
            if (promoStage == 1 && chooseB) {
                gamWarrant[slot] += 3;
                promoUsed[2] = 1; EndPromo(0);
            }
            break;

        // --- 60: Sentence served ---
        case 60:
            if (promoTim < 300) Speak(cyc, 2);
            if (promoStage == 1) {
                optionA = "I'm ready to leave.";
                optionB = "I'm staying a while.";
            }
            if (promoStage == 1 && chooseA) {
                screen = 7;  // → Outro
                promoUsed[60] = 1; EndPromo(0);
            }
            if (promoStage == 1 && chooseB) {
                promoUsed[60] = 1; EndPromo(0);
            }
            break;

        // --- 100: Threat from prisoner ---
        case 100:
            if (promoTim < 300) Speak(cyc, 1);
            if (promoStage == 1) {
                optionA = "Back off!";
                optionB = "Leave me alone.";
            }
            if (promoStage == 1 && (chooseA || chooseB)) {
                if (chooseA) charRelation[pChar[v]][pChar[cyc]] = -1;
                EndPromo(0);
            }
            break;

        // --- 115: Gang recruitment ---
        case 115:
            if (promoTim < 300) Speak(cyc, 2);
            if (promoStage == 1) {
                optionA = "I'll join.";
                optionB = "No thanks.";
            }
            if (promoStage == 1 && chooseA) {
                charGang[pChar[v]] = charGang[pChar[cyc]];
                ChangeRelationship(pChar[cyc], pChar[v], 1);
                EndPromo(0);
            }
            if (promoStage == 1 && chooseB) { EndPromo(0); }
            break;

        // --- 141: Acquire strength mission ---
        case 141:
            if (promoTim < 350) {
                // assign mission
                gamMission[slot]  = 1;
                gamClient[slot]   = pChar[cyc];
                gamDeadline[slot] = (gamHours[slot] + 24) % 24;
                gamReward[slot]   = 200;
                gamTarget[slot]   = charStrength[gamChar[slot]] + 5;
            }
            if (promoTim > 350 && promoTim < 650) Speak(cyc, 1);
            if (promoTim > 650) {
                promoUsed[141] = 1;
                EndPromo(0);
            }
            break;

        default:
            // Unknown promo – just end it after a delay
            if (promoTim > 200) EndPromo(0);
            break;
    }

    promoTim++;
}

// ---------------------------------------------------------------------------
//  EndPromo  –  close active dialogue
// ---------------------------------------------------------------------------
void EndPromo(int /*effect*/) {
    pSpeaking[promoActor[1]] = 0;
    pSpeaking[promoActor[2]] = 0;
    gamPromo      = 0;
    promoTim      = 0;
    promoStage    = 0;
    promoActor[1] = 0;
    promoActor[2] = 0;
}

// ---------------------------------------------------------------------------
//  PromoCycle  –  called from gameplay loop each frame
// ---------------------------------------------------------------------------
void PromoCycle() {
    // Advance active promo
    if (gamPromo > 0) {
        ExecutePromo();
        return;
    }

    // Try to initiate new promo (scan all NPC → player)
    if (Rnd(0, 30) != 0) return;  // throttle checks
    int plr = gamPlayer[slot];
    if (plr <= 0 || plr > no_plays) return;

    for (int cyc = 1; cyc <= no_plays; ++cyc) {
        if (cyc == plr) continue;
        if (Rnd(0, no_plays) != 0) continue;
        RiskPromo(cyc, plr);
        if (gamPromo != 0) break;
    }
}
