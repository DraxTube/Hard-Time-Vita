#include "ai.h"
// =============================================================================
//  world_props.cpp  –  World props  (doors, kits, trays)
// =============================================================================
#include "world_props.h"
#include "players.h"
#include "values.h"
#include "functions.h"
#include "crimes.h"
#include "render3d.h"
#include "blitz_compat.h"

// ---------------------------------------------------------------------------
//  Array definitions
// ---------------------------------------------------------------------------
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorA  {};
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorX1 {};
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorX2 {};
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorY1 {};
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorY2 {};
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorZ1 {};
std::array<std::array<float,MAX_DOORS>,MAX_LOCS_D> doorZ2 {};

std::array<float,MAX_CELLS_D> cellX1   {};
std::array<float,MAX_CELLS_D> cellX2   {};
std::array<float,MAX_CELLS_D> cellZ1   {};
std::array<float,MAX_CELLS_D> cellZ2   {};
std::array<float,MAX_CELLS_D> cellY1   {};
std::array<float,MAX_CELLS_D> cellY2   {};
std::array<float,MAX_CELLS_D> cellDoorX{};
std::array<float,MAX_CELLS_D> cellDoorZ{};

std::array<Handle,MAX_KITS> kit     {};
std::array<int,   MAX_KITS> kitType {}, kitState{};

std::array<int,MAX_TRAYS> trayState{}, trayOldState{};

std::array<GLuint,11> tScreen{};
std::array<GLuint,11> tTray    {};
GLuint tMachine=0, tPistol=0;
std::array<GLuint,4> tEyes    {};
std::array<GLuint,6> tLegScar {};
std::array<GLuint,4> tSeverBody{}, tSeverArm{}, tSeverLegs{};

// ---------------------------------------------------------------------------
//  InitWorldProps  –  set cell bounding boxes for the prison block layout
// ---------------------------------------------------------------------------
void InitWorldProps() {
    // 20 cells: 10 north (1-10), 10 south (11-20)
    // Each cell ~36 units wide, 60 deep
    for (int c = 1; c <= 20; ++c) {
        float cx = -180.0f + ((c-1) % 10) * 38.0f;
        float cz = (c <= 10) ? -310.0f : 250.0f;
        cellX1[c]    = cx - 18.0f;
        cellX2[c]    = cx + 18.0f;
        cellZ1[c]    = cz - 30.0f;
        cellZ2[c]    = cz + 30.0f;
        cellY1[c]    = 0.0f;
        cellY2[c]    = 50.0f;
        cellDoorX[c] = cx;
        cellDoorZ[c] = cz + (c<=10 ? 30.0f : -30.0f);
    }
}

// ---------------------------------------------------------------------------
//  DoorBlocked  –  returns 1 if door is closed/locked
// ---------------------------------------------------------------------------
int DoorBlocked(int loc, int door) {
    if (loc<0||loc>=MAX_LOCS_D||door<0||door>=MAX_DOORS) return 0;
    return (cellLocked[loc][door]) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  OpenDoor / CloseDoor  –  animate the door entity open or closed
// ---------------------------------------------------------------------------
void OpenDoor(int loc, int door) {
    if (loc<0||loc>=MAX_LOCS_D||door<0||door>=MAX_DOORS) return;
    cellLocked[loc][door] = 0;
    Handle hDoor = FindChild(world, "Door" + Dig(door, 10));
    if (hDoor) Animate(hDoor, 3, 2.0f, 0, 0.0f);
    if (sBuzzer) ProduceSound(world, sBuzzer, 22050, 0.5f);
}

void CloseDoor(int loc, int door) {
    if (loc<0||loc>=MAX_LOCS_D||door<0||door>=MAX_DOORS) return;
    cellLocked[loc][door] = 1;
    Handle hDoor = FindChild(world, "Door" + Dig(door, 10));
    if (hDoor) Animate(hDoor, 3, -2.0f, 0, 0.0f);
    if (sBuzzer) ProduceSound(world, sBuzzer, 22050, 0.5f);
}

// ---------------------------------------------------------------------------
//  Prop Interaction Helpers
// ---------------------------------------------------------------------------
int InsideCell(float x, float y, float z) {
    int cell = 0;
    for (int count = 1; count <= 20; ++count) {
        if (y >= cellY1[count] && y <= cellY2[count]) {
            if (x >= cellX1[count] && x <= cellX2[count] && 
                z >= cellZ1[count] && z <= cellZ2[count]) {
                cell = count;
            }
        }
    }
    // null if not in block
    if (screen == 50 && go == 0) {
        // GetBlock logic: (1->1, 3->2, 5->3, 7->4)
        int block = 0;
        int loc = gamLocation[slot];
        if (loc == 1) block = 1;
        if (loc == 3) block = 2;
        if (loc == 5) block = 3;
        if (loc == 7) block = 4;
        if (block == 0) cell = 0;
    }
    return cell;
}

int CellVisible(float x, float y, float z, int cell) {
    int value = 0;
    int loc = gamLocation[slot];
    int block = (loc==1)?1 : (loc==3)?2 : (loc==5)?3 : (loc==7)?4 : 0;
    
    if (block > 0 && cell > 0) {
        if (y >= cellY1[cell] && y <= cellY2[cell]) {
            if (cell == 5 || cell == 6 || cell == 15 || cell == 16) {
                if (x >= cellX1[cell] && x <= cellX2[cell]) value = 1;
            } else {
                if (z >= cellZ1[cell] && z <= cellZ2[cell]) value = 1;
            }
        }
    }
    return value;
}

int ChairProximity(int cyc, int chair) {
    int value = 0;
    Handle limb = FindChild(world, "Chair" + Dig(chair, 10));
    if (limb) {
        if (pX[cyc] > EntityX(limb, 1) - 18 && pX[cyc] < EntityX(limb, 1) + 18 &&
            pY[cyc] > EntityY(limb, 1) - 30 && pY[cyc] < EntityY(limb, 1) - 5 &&
            pZ[cyc] > EntityZ(limb, 1) - 18 && pZ[cyc] < EntityZ(limb, 1) + 18) {
            value = 1; // Simplified InLine check for Vita
        }
    }
    return value;
}

int ChairTaken(int chair) {
    int value = 0;
    for (int v = 1; v <= no_plays; ++v) {
        if (pSeat[v] == chair) value = 1;
    }
    return value;
}

int BedProximity(int cyc, int bed) {
    int value = 0;
    Handle limb = FindChild(world, "Mat" + Dig(bed, 10));
    if (limb) {
        if (pX[cyc] > EntityX(limb, 1) - 25 && pX[cyc] < EntityX(limb, 1) + 25 &&
            pY[cyc] > EntityY(limb, 1) - 25 && pY[cyc] < EntityY(limb, 1) &&
            pZ[cyc] > EntityZ(limb, 1) - 25 && pZ[cyc] < EntityZ(limb, 1) + 25) {
            value = 1; // Simplified InLine
        }
    }
    return value;
}

int BedTaken(int bed) {
    int value = 0;
    for (int v = 1; v <= no_plays; ++v) {
        if (pBed[v] == bed) value = 1;
    }
    return value;
}


int NearBasket(int cyc) {
    int value = 0;
    if (gamLocation[slot] == 2 && pWeapon[cyc] > 0) {
        Handle limb = FindChild(world, "Rim");
        if (limb) {
            if (pX[cyc] > EntityX(limb, 1) - 100 && pX[cyc] < EntityX(limb, 1) + 100 &&
                pZ[cyc] > EntityZ(limb, 1) - 100 && pZ[cyc] < EntityZ(limb, 1) + 100) {
                value = 1;
            }
        }
    }
    return value;
}

// ---------------------------------------------------------------------------
//  Interaction Helpers
// ---------------------------------------------------------------------------

int PhoneProximity(int cyc) {
    int value = 0;
    if (gamLocation[slot] == 9) {
        for (int v = 1; v <= 4; ++v) {
            Handle limb = FindChild(world, "Pad" + Dig(v, 10));
            if (limb) {
                if (pX[cyc] > EntityX(limb, 1) - 20.0f && pX[cyc] < EntityX(limb, 1) + 20.0f &&
                    pZ[cyc] > EntityZ(limb, 1) - 15.0f && pZ[cyc] < EntityZ(limb, 1) + 15.0f) {
                    value = v;
                }
            }
        }
    }
    return value;
}

int PhoneTaken(int phone) {
    int value = 0;
    for (int v = 1; v <= no_plays; ++v) {
        if (pPhone[v] == phone) value = 1;
    }
    return value;
}

int OnComputer(int cyc) {
    int value = 0;
    if (gamLocation[slot] == 4 && pSeat[cyc] == 5) value = 1;
    if (gamLocation[slot] == 6 && pSeat[cyc] == 5) value = 1;
    if (gamLocation[slot] == 9 && pSeat[cyc] == 7) value = 1;
    return value;
}

void EnterDoor(int cyc, int door) {
    oldLocation = gamLocation[slot];
    int loc = gamLocation[slot];
    int charId = pChar[cyc];

    // north -> hall
    if (loc == 1) {
        charX[charId] = -150.0f; charZ[charId] = 280.0f;
        charY[charId] = 20.0f;   charA[charId] = 180.0f;
        charLocation[charId] = 9;
    }
    // yard -> hall
    if (loc == 2) {
        charX[charId] = 150.0f; charZ[charId] = 280.0f;
        charY[charId] = 20.0f;   charA[charId] = 180.0f;
        charLocation[charId] = 9;
    }
    // east -> hall
    if (loc == 3) {
        charX[charId] = 280.0f; charZ[charId] = 150.0f;
        charY[charId] = 20.0f;   charA[charId] = 90.0f;
        charLocation[charId] = 9;
    }
    // study -> hall
    if (loc == 4 && door == 1) {
        charX[charId] = 280.0f; charZ[charId] = -150.0f;
        charY[charId] = 20.0f;   charA[charId] = 90.0f;
        charLocation[charId] = 9;
    }
    // study -> workshop
    if (loc == 4 && door == 2) {
        charX[charId] = 0.0f; charZ[charId] = -105.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = 10;
    }
    // south -> hall
    if (loc == 5) {
        charX[charId] = 150.0f; charZ[charId] = -280.0f;
        charY[charId] = 20.0f;   charA[charId] = 0.0f;
        charLocation[charId] = 9;
    }
    // hospital -> hall
    if (loc == 6 && door == 1) {
        charX[charId] = -150.0f; charZ[charId] = -280.0f;
        charY[charId] = 20.0f;   charA[charId] = 0.0f;
        charLocation[charId] = 9;
    }
    // hospital -> toilets
    if (loc == 6 && door == 2) {
        charX[charId] = 90.0f; charZ[charId] = -55.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = 11;
    }
    // west -> hall
    if (loc == 7) {
        charX[charId] = -280.0f; charZ[charId] = -150.0f;
        charY[charId] = 20.0f;   charA[charId] = 270.0f;
        charLocation[charId] = 9;
    }
    // kitchen -> hall
    if (loc == 8) {
        charX[charId] = -280.0f; charZ[charId] = 150.0f;
        charY[charId] = 20.0f;   charA[charId] = 270.0f;
        charLocation[charId] = 9;
    }
    // hall -> block
    if (loc == 9 && (door == 1 || door == 3 || door == 5 || door == 7)) {
        charX[charId] = 0.0f; charZ[charId] = -325.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = door;
    }
    // hall -> yard
    if (loc == 9 && door == 2) {
        charX[charId] = 80.0f; charZ[charId] = 215.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = door; // 2 -> yard
    }
    // hall -> study
    if (loc == 9 && door == 4) {
        charX[charId] = 5.0f; charZ[charId] = -130.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = 4;
    }
    // hall -> hospital
    if (loc == 9 && door == 6) {
        charX[charId] = 0.0f; charZ[charId] = -130.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = 6;
    }
    // hall -> kitchen
    if (loc == 9 && door == 8) {
        charX[charId] = 0.0f; charZ[charId] = -330.0f;
        charY[charId] = 20.0f; charA[charId] = 0.0f;
        charLocation[charId] = 8;
    }
    // workshop -> study
    if (loc == 10) {
        charX[charId] = 135.0f; charZ[charId] = 0.0f;
        charY[charId] = 20.0f;  charA[charId] = 90.0f;
        charLocation[charId] = 4;
    }
    // toilets -> hospital
    if (loc == 11) {
        charX[charId] = 0.0f; charZ[charId] = 130.0f;
        charY[charId] = 20.0f; charA[charId] = 180.0f;
        charLocation[charId] = 6;
    }
    
    // Proceed
    go = 1;
}

// ---------------------------------------------------------------------------
//  PropCycle  –  kits + food trays
// ---------------------------------------------------------------------------
void PropCycle() {
    int loc = gamLocation[slot];
    
    // Manage Food Trays (Canteen)
    if (loc == 8) {
        for (int tray = 1; tray <= 50; ++tray) {
            Handle limb = FindChild(world, "Tray" + Dig(tray, 10));
            if (limb) {
                if (trayState[tray] > 0) {
                    EntityTexture(limb, tTray[trayState[tray]]);
                    EntityAlpha(limb, 1.0f);
                } else {
                    EntityAlpha(limb, 0.0f);
                }
            }
        }
    }

    // Kit respawn: if kit has been picked up, respawn after 1000 frames
    for (int k = 1; k < MAX_KITS; ++k) {
        if (kitState[k] == 0) continue;
        // Check if player is adjacent
        int plr = gamPlayer[slot];
        if (plr < 1 || plr > no_plays) continue;
        if (kit[k] == 0) continue;

        float dx = EntityX(kit[k]) - pX[plr];
        float dz = EntityZ(kit[k]) - pZ[plr];
        if (dx*dx+dz*dz < 20.0f*20.0f && cPickUp[plr]) {
            // Apply kit effect
            if (kitType[k] == 1) {
                // Medical kit: restore health
                pHealth[plr] = std::min(pHealthLimit[plr], pHealth[plr] + 30);
                pHP[plr] = pHealth[plr];
            } else if (kitType[k] == 2) {
                // Food kit: restore happiness
                charHappiness[pChar[plr]] = std::min(100, charHappiness[pChar[plr]] + 20);
            }
            HideEntity(kit[k]);
            kitState[k] = 2;  // 2 = picked up, will respawn
        }
    }
}

// ---------------------------------------------------------------------------
//  DoorCycle  –  manage doors and location transitions
// ---------------------------------------------------------------------------
void DoorCycle() {
    if (!world) return;
    int loc = gamLocation[slot];

    // Lockdown: close all cell doors (gamHours < 7 || gamHours >= 22 = lockdown)
    int lockdown = (gamHours[slot] < 7 || gamHours[slot] >= 22) ? 1 : 0;
    
    for (int d = 1; d <= no_doors; ++d) {
        bool shouldBeLocked = lockdown;
        
        // Open cell doors for warrants
        if (gamWarrant[slot] > 0) shouldBeLocked = false;

        bool currently = (cellLocked[loc][d] != 0);
        if (shouldBeLocked && !currently) CloseDoor(loc, d);
        if (!shouldBeLocked && currently) OpenDoor(loc, d);
        
        // Proximity detection for gamPlayer to transition areas
        int plr = gamPlayer[slot];
        if (plr > 0 && plr <= no_plays && pAnim[plr] < 93 && !DoorBlocked(loc, d)) {
            if (pX[plr] > doorX1[loc][d] && pX[plr] < doorX2[loc][d] &&
                pY[plr] > doorY1[loc][d] && pY[plr] < doorY2[loc][d] &&
                pZ[plr] > doorZ1[loc][d] && pZ[plr] < doorZ2[loc][d]) {
                
                // Trigger Location Transition
                EnterDoor(plr, d);
            }
        }
    }
}
