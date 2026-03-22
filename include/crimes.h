#pragma once
// =============================================================================
//  crimes.h  –  Crime detection, court case, warrant system  (from Crimes.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"

// Warrant levels: 0=none, 1-5=minor, 6-10=assault, 11-20=weapon, max=prosecuted
// Sentence increments per crime level

// Helpers used throughout codebase
int  AttackViable(int cyc);           // 1=standing,2=kneeling,3=on ground,0=dead
int  Friendly(int cyc, int v);        // 1 if they are allies
int  InProximity(int cyc, int v, float range);
int  InRange(int cyc, int v, float range);
int  InLine(int cyc, Handle entity, float fov); // within FOV cone
void RiskAnger(int cyc, int victim);
void DamageRep(int cyc, int victim, int severe);
void ChangeRelationship(int charA, int charB, int delta);
void GainStrength(int cyc, int amount);
void RiskInjury(int v, int chance);
void ScarArea(int v, int zone, int x, int y, int style);
int  CountScars(int cyc);
void GroundReaction(int v);
void DropWeapon(int cyc, int range);
int  GetPower(int cyc);
int  BlockPower(int cyc);
void ScarLimb(int cyc, int limb, int chance);

// Court case
void CourtCase();

// Crime cycle (called once per frame from gameplay loop)
void CrimeCycle();
