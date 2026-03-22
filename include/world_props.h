#pragma once
// =============================================================================
//  world_props.h  –  World props: doors, chairs, beds, kits, trays, phones
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"
#include <array>

// Door data (per location, per door)
static constexpr int MAX_LOCS_D  = 16;
static constexpr int MAX_DOORS   = 11;
static constexpr int MAX_CELLS_D = 21;

extern std::array<std::array<float, MAX_DOORS>, MAX_LOCS_D> doorA;
extern std::array<std::array<float, MAX_DOORS>, MAX_LOCS_D> doorX1, doorX2;
extern std::array<std::array<float, MAX_DOORS>, MAX_LOCS_D> doorY1, doorY2;
extern std::array<std::array<float, MAX_DOORS>, MAX_LOCS_D> doorZ1, doorZ2;

// Cell bounding boxes
extern std::array<float, MAX_CELLS_D> cellX1, cellX2;
extern std::array<float, MAX_CELLS_D> cellZ1, cellZ2;
extern std::array<float, MAX_CELLS_D> cellY1, cellY2;
extern std::array<float, MAX_CELLS_D> cellDoorX, cellDoorZ;

// Kits (medicine, weapon boxes, etc.)
static constexpr int MAX_KITS = 7;
extern std::array<Handle, MAX_KITS> kit;
extern std::array<int,    MAX_KITS> kitType, kitState;

// Tray state (food trays in canteen)
static constexpr int MAX_TRAYS = 51;
extern std::array<int, MAX_TRAYS> trayState, trayOldState;

// Extra textures
extern std::array<GLuint, 11> tScreen;
extern std::array<GLuint, 11> tTray;
extern GLuint tMachine, tPistol;
extern std::array<GLuint, 4> tEyes;
extern std::array<GLuint, 6> tLegScar;
extern std::array<GLuint, 4> tSeverBody, tSeverArm, tSeverLegs;

// Functions
void InitWorldProps();
void DoorCycle();          // open/close doors based on lockdown state
void PropCycle();          // kits + trays
int  DoorBlocked(int loc, int door);
void OpenDoor(int loc, int door);
void CloseDoor(int loc, int door);

// Interaction helpers ported from World.bb
void EnterDoor(int cyc, int door);
int  InsideCell(float x, float y, float z);
int  CellVisible(float x, float y, float z, int cell);
int  ChairProximity(int cyc, int chair);
int  ChairTaken(int chair);
int  BedProximity(int cyc, int bed);
int  BedTaken(int bed);
int  PhoneProximity(int cyc);
int  PhoneTaken(int phone);
int  OnComputer(int cyc);
int  NearBasket(int cyc);
