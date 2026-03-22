// =============================================================================
//  missions.cpp  –  Mission system  (ported from Missions.bb)
// =============================================================================
#include "missions.h"
#include "players.h"
#include "values.h"
#include "functions.h"
#include "crimes.h"
#include "blitz_compat.h"

// ---------------------------------------------------------------------------
//  GetClient  –  0=phone,1=stranger,2=gang member,3=warden
// ---------------------------------------------------------------------------
int GetClient(int cyc, int v) {
    if (pChar[cyc] == 0) return 0;                               // phone
    if (charRole[pChar[cyc]] == 1) return 3;                     // warden
    if (charGang[pChar[cyc]] > 0 &&
        charGang[pChar[cyc]] == charGang[pChar[v]]) return 2;    // gang
    return 1;                                                     // stranger
}

// ---------------------------------------------------------------------------
//  AssignMission  –  set mission state from a promo interaction
// ---------------------------------------------------------------------------
void AssignMission(int cyc, int id) {
    gamMission[slot] = id;
    gamClient[slot]  = pChar[cyc];
    gamReward[slot]  = 200 + id * 50;
    // Deadline 24 game-hours from now
    gamDeadline[slot] = (gamHours[slot] + 24) % 24;
}

// ---------------------------------------------------------------------------
//  CheckMission  –  returns 1 if the objective has been met
// ---------------------------------------------------------------------------
int CheckMission() {
    if (gamMission[slot] == 0) return 0;
    int plr = gamPlayer[slot];
    if (plr <= 0 || plr > no_plays) return 0;

    switch (gamMission[slot]) {
        // 1: Acquire strength
        case 1:
            return (charStrength[gamChar[slot]] >= gamTarget[slot]) ? 1 : 0;
        // 2: Acquire agility
        case 2:
            return (charAgility[gamChar[slot]] >= gamTarget[slot]) ? 1 : 0;
        // 3: Acquire intelligence
        case 3:
            return (charIntelligence[gamChar[slot]] >= gamTarget[slot]) ? 1 : 0;
        // 4: Acquire reputation
        case 4:
            return (charReputation[gamChar[slot]] >= gamTarget[slot]) ? 1 : 0;
        // 5: Lose reputation
        case 5:
            return (charReputation[gamChar[slot]] <= gamTarget[slot]) ? 1 : 0;
        // 6: Get out of debt
        case 6:
            return (gamMoney[slot] >= 0) ? 1 : 0;
        // 7: Acquire money
        case 7:
            return (gamMoney[slot] >= gamTarget[slot]) ? 1 : 0;
        // 8: Change hairstyle
        case 8:
            return (charHairStyle[gamChar[slot]] == gamTarget[slot]) ? 1 : 0;
        // 9: Change costume
        case 9:
            return (charCostume[gamChar[slot]] == gamTarget[slot]) ? 1 : 0;
        // 10: Assault target
        case 10:
            return (charHealth[gamTarget[slot]] <= 0 ||
                    charStatus[gamTarget[slot]] == 1) ? 1 : 0;
        // 11: Kill target
        case 11:
            return (charStatus[gamTarget[slot]] == 2) ? 1 : 0;
        // 12: Steal weapon from target
        case 12:
            return (pWeapon[plr] == gamItem[slot]) ? 1 : 0;
        // 13: Deliver item to client
        case 13: {
            int client = gamClient[slot];
            for (int c = 1; c <= no_plays; ++c) {
                if (pChar[c] == client && InProximity(plr, c, 30) &&
                    pWeapon[plr] == gamItem[slot])
                    return 1;
            }
            return 0;
        }
        default:
            return 0;
    }
}

// ---------------------------------------------------------------------------
//  MissionComplete
// ---------------------------------------------------------------------------
void MissionComplete() {
    gamMoney[slot] += gamReward[slot];

    // Relationship boost with client
    for (int c = 1; c <= no_chars; ++c) {
        if (c == gamClient[slot])
            ChangeRelationship(gamChar[slot], c, 1);
    }

    // Small stat boost based on mission type
    switch (gamMission[slot]) {
        case 1:  charStrength[gamChar[slot]]     = std::min(100, charStrength[gamChar[slot]]+1);     break;
        case 2:  charAgility[gamChar[slot]]       = std::min(100, charAgility[gamChar[slot]]+1);      break;
        case 3:  charIntelligence[gamChar[slot]]  = std::min(100, charIntelligence[gamChar[slot]]+1); break;
        case 4:  charReputation[gamChar[slot]]    = std::min(100, charReputation[gamChar[slot]]+1);   break;
        default: break;
    }

    gamMission[slot] = 0;
    gamClient[slot]  = 0;
    gamTarget[slot]  = 0;
    gamReward[slot]  = 0;
}

// ---------------------------------------------------------------------------
//  MissionFail
// ---------------------------------------------------------------------------
void MissionFail() {
    // Relationship penalty with client
    for (int c = 1; c <= no_chars; ++c) {
        if (c == gamClient[slot])
            ChangeRelationship(gamChar[slot], c, -1);
    }
    gamMission[slot] = 0;
    gamClient[slot]  = 0;
}

// ---------------------------------------------------------------------------
//  MissionCycle  –  called each frame from gameplay loop
// ---------------------------------------------------------------------------
void MissionCycle() {
    if (gamMission[slot] == 0) return;

    // Check deadline expiry (game time)
    if (gamHours[slot] == gamDeadline[slot] && gamMins[slot] == 0) {
        if (CheckMission()) MissionComplete();
        else                MissionFail();
        return;
    }

    // Check mid-cycle completion
    if (CheckMission()) MissionComplete();
}
