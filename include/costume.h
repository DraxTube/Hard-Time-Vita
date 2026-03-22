#pragma once
// =============================================================================
//  costume.h  –  Costume / appearance system  (from Editor.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"
#include <array>

// Texture handle arrays (filled in LoadTextures inside functions.cpp)
extern std::array<GLuint, 21>  tHair;
extern std::array<GLuint, 101> tFace;
extern std::array<GLuint, 101> tBody;
extern std::array<GLuint, 101> tArm;
extern std::array<GLuint, 101> tLegs;
extern std::array<GLuint, 11>  tBodyShade;
extern std::array<GLuint, 11>  tArmShade;
extern std::array<GLuint, 4>   tSpecs;
extern std::array<GLuint, 6>   tMouth;
extern std::array<GLuint, 6>   tFaceScar;
extern std::array<GLuint, 6>   tBodyScar;
extern std::array<GLuint, 6>   tArmScar;
extern std::array<GLuint, 6>   tLegScar;
extern std::array<GLuint, 4>   tSeverBody;
extern std::array<GLuint, 4>   tSeverArm;
extern std::array<GLuint, 4>   tSeverLegs;
extern std::array<GLuint, 7>   tTattooBody;
extern std::array<GLuint, 7>   tTattooVest;
extern std::array<GLuint, 7>   tTattooArm;
extern std::array<GLuint, 7>   tTattooTee;
extern std::array<GLuint, 7>   tTattooSleeve;
extern GLuint tShaved, tEars, tSeverEars;

// Helper functions (used by Players.bb + Editor.bb)
int  GetRace(int charIndex);           // 0=white,1=black,2=asian,3=arab
int  BaggyTop(int costume);
void ApplyCostume(int cyc);
void ApplyHair(int cyc);
void ApplyEyewear(int cyc);
void ApplyAccessories(int cyc);
void ApplyClothing(int cyc);
void GangAdjust(int charIndex);
void ReloadModel(int cyc);             // reload + reapply everything
void GenerateCharacter(int charIndex, int role);  // randomise a new character
BBString CellName(int charIndex);      // "John Smith" or nickname
BBString GetRaceStr(int charIndex);
int  InsideCell(float x, float y, float z);  // returns cell number or 0
int  CellVisible(float cx, float cy, float cz, int cell);
