#pragma once
// =============================================================================
//  moves.h  –  Movement physics declarations  (from Moves.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"
#include <array>

// Extra movement arrays (definitions in moves.cpp)
extern std::array<float, 51> pOldY;
extern std::array<float, 51> pLeaveX, pLeaveY, pLeaveZ, pLeaveA;
extern std::array<float, 51> pHurtA, pStagger, pSpeed;
extern std::array<int,   51> pSting, pOldHealth, pFireTim;
extern std::array<int,   51> pTarget, pExploreRange, pOldAnim, pFoodTim;
extern std::array<std::array<int, 11>, 51> pDoorFriction;
extern std::array<std::array<int, 51>, 51> pMultiSting;
extern std::array<int, 3> pHighlight;

// Camera aux
extern Handle camPivot;
extern int    camOldFoc;
extern float  camX, camY, camZ;

// Weapon extra
extern std::array<float, 101> weapOldY;
extern std::array<int,    26> weapHabitat;
extern std::array<int,    26> weapCreate;

// World textures
extern Handle tShower, tCrowd;

// Extra sounds
extern std::array<BBSound*, 7> sStep;
extern std::array<BBSound*, 6> sAgony;
extern std::array<BBSound*, 6> sShot;
extern std::array<BBSound*, 6> sRicochet;

void LoadMoveSounds();
void MovePlayers();
