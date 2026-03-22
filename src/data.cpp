// =============================================================================
//  data.cpp  –  Save / Load / Generate functions  (converted from Data.bb)
//  Saves to ux0:data/HardTime/ on the Vita memory card
// =============================================================================
#include "data.h"
#include "values.h"
#include "blitz_compat.h"
#include "players.h"
#include "functions.h"
#include "texts.h"
#include "moves.h"
#include "world_props.h"
#include "costume.h"
#include "ai.h"
#include "crimes.h"
#include <psp2/io/stat.h>
#include <algorithm>

static const BBString SAVE_ROOT = "ux0:data/HardTime/";

static void EnsureDir(const BBString& path) {
    sceIoMkdir(path.c_str(), 0777);
}

// Forward declarations for functions defined later in this file
void GenerateWeapon(int cyc, int style, int area, float x, float y, float z);
static BBString GenerateName(int charId);

// ---------------------------------------------------------------------------
//  Helper functions  (from Data.bb / Gameplay.bb)
// ---------------------------------------------------------------------------

// TranslateBlock: converts a block index (1-4) to its location number (1,3,5,7)
int TranslateBlock(int block) {
    if (block == 1) return 1;
    if (block == 2) return 3;
    if (block == 3) return 5;
    if (block == 4) return 7;
    return 0;
}

// CellPopulation: counts how many characters are in a given block+cell
int CellPopulation(int block, int cell) {
    int count = 0;
    for (int v = 1; v <= no_chars; ++v) {
        if (charBlock[v] == block && charCell[v] == cell && charRole[v] == 0)
            count++;
    }
    return count;
}

// AreaPopulation: counts how many characters with a given role are in an area
int AreaPopulation(int area, int role) {
    int count = 0;
    for (int v = 1; v <= no_chars; ++v) {
        if (charLocation[v] == area && charRole[v] == role)
            count++;
    }
    return count;
}

// FindCarrier: returns the character carrying weapon cyc, or 0 if none
int FindCarrier(int cyc) {
    for (int v = 1; v <= no_chars; ++v) {
        if (charWeapon[v] == cyc) return v;
    }
    return 0;
}

// ---------------------------------------------------------------------------
//  SaveOptions  (mirrors Data.bb SaveOptions())
// ---------------------------------------------------------------------------
void SaveOptions() {
    EnsureDir(SAVE_ROOT);
    BBFile* f = WriteFile(SAVE_ROOT + "Options.dat");
    if (!f || f->fd < 0) return;

    WriteInt(f, optRes);
    WriteInt(f, optPopulation);
    WriteInt(f, optFog);
    WriteInt(f, optShadows);
    WriteInt(f, optFX);
    WriteInt(f, optGore);
    WriteInt(f, keyAttack);
    WriteInt(f, keyDefend);
    WriteInt(f, keyThrow);
    WriteInt(f, keyPickUp);
    WriteInt(f, buttAttack);
    WriteInt(f, buttDefend);
    WriteInt(f, buttThrow);
    WriteInt(f, buttPickUp);
    for (int i = 1; i <= 3; ++i) WriteString(f, gamName[i]);
    CloseFile(f);
}

// ---------------------------------------------------------------------------
//  LoadOptions  (mirrors Data.bb LoadOptions())
// ---------------------------------------------------------------------------
void LoadOptions() {
    BBString path = SAVE_ROOT + "Options.dat";
    if (!FileExists(path)) return;

    BBFile* f = ReadFile(path);
    if (!f || f->fd < 0) return;

    optRes        = ReadInt(f);
    optPopulation = ReadInt(f);
    optFog        = ReadInt(f);
    optShadows    = ReadInt(f);
    optFX         = ReadInt(f);
    optGore       = ReadInt(f);
    keyAttack     = ReadInt(f);
    keyDefend     = ReadInt(f);
    keyThrow      = ReadInt(f);
    keyPickUp     = ReadInt(f);
    buttAttack    = ReadInt(f);
    buttDefend    = ReadInt(f);
    buttThrow     = ReadInt(f);
    buttPickUp    = ReadInt(f);
    for (int i = 1; i <= 3; ++i) gamName[i] = ReadString(f);
    CloseFile(f);
}

// ---------------------------------------------------------------------------
//  SaveProgress  (mirrors Data.bb SaveProgress() — COMPLETE)
// ---------------------------------------------------------------------------
void SaveProgress() {
    BBString slotDir = SAVE_ROOT + "Data/Slot0" + Str(slot) + "/";
    EnsureDir(SAVE_ROOT + "Data/");
    EnsureDir(slotDir);

    BBFile* f = WriteFile(slotDir + "Progress.dat");
    if (!f || f->fd < 0) return;

    // status
    WriteInt(f, no_chars);
    WriteInt(f, gamChar[slot]);
    WriteInt(f, gamPlayer[slot]);
    WriteInt(f, gamLocation[slot]);
    WriteInt(f, gamMoney[slot]);
    // time
    WriteInt(f, gamSpeed[slot]);
    WriteInt(f, gamSecs[slot]);
    WriteInt(f, gamMins[slot]);
    WriteInt(f, gamHours[slot]);
    // missions
    WriteInt(f, gamMission[slot]);
    WriteInt(f, gamClient[slot]);
    WriteInt(f, gamTarget[slot]);
    WriteInt(f, gamDeadline[slot]);
    WriteInt(f, gamReward[slot]);
    // handles
    WriteInt(f, gamWarrant[slot]);
    WriteInt(f, gamVictim[slot]);
    WriteInt(f, gamItem[slot]);
    WriteInt(f, gamArrival[slot]);
    WriteInt(f, gamFatality[slot]);
    WriteInt(f, gamRelease[slot]);
    WriteInt(f, gamEscape[slot]);
    WriteInt(f, gamGrowth[slot]);
    WriteInt(f, gamBlackout[slot]);
    WriteInt(f, gamBombThreat[slot]);
    // promos
    WriteInt(f, phonePromo);
    for (int i = 1; i <= NO_PROMOS; ++i) WriteInt(f, promoUsed[i]);
    // camera
    WriteFloat(f, camX);
    WriteFloat(f, camY);
    WriteFloat(f, camZ);
    WriteFloat(f, camPivX);
    WriteFloat(f, camPivY);
    WriteFloat(f, camPivZ);
    // atmosphere
    WriteFloat(f, lightR);
    WriteFloat(f, lightG);
    WriteFloat(f, lightB);
    WriteFloat(f, ambR);
    WriteFloat(f, ambG);
    WriteFloat(f, ambB);
    WriteFloat(f, atmosR);
    WriteFloat(f, atmosG);
    WriteFloat(f, atmosB);
    WriteFloat(f, skyR);
    WriteFloat(f, skyG);
    WriteFloat(f, skyB);
    // dinner trays
    for (int i = 1; i <= 50; ++i) WriteInt(f, trayState[i]);
    CloseFile(f);
}

// ---------------------------------------------------------------------------
//  LoadProgress  (mirrors Data.bb LoadProgress() — COMPLETE)
// ---------------------------------------------------------------------------
void LoadProgress() {
    BBString path = SAVE_ROOT + "Data/Slot0" + Str(slot) + "/Progress.dat";
    if (!FileExists(path)) return;

    BBFile* f = ReadFile(path);
    if (!f || f->fd < 0) return;

    // status
    no_chars           = ReadInt(f);
    gamChar[slot]      = ReadInt(f);
    gamPlayer[slot]    = ReadInt(f);
    gamLocation[slot]  = ReadInt(f);
    gamMoney[slot]     = ReadInt(f);
    // time
    gamSpeed[slot]     = ReadInt(f);
    gamSecs[slot]      = ReadInt(f);
    gamMins[slot]      = ReadInt(f);
    gamHours[slot]     = ReadInt(f);
    // missions
    gamMission[slot]   = ReadInt(f);
    gamClient[slot]    = ReadInt(f);
    gamTarget[slot]    = ReadInt(f);
    gamDeadline[slot]  = ReadInt(f);
    gamReward[slot]    = ReadInt(f);
    // handles
    gamWarrant[slot]   = ReadInt(f);
    gamVictim[slot]    = ReadInt(f);
    gamItem[slot]      = ReadInt(f);
    gamArrival[slot]   = ReadInt(f);
    gamFatality[slot]  = ReadInt(f);
    gamRelease[slot]   = ReadInt(f);
    gamEscape[slot]    = ReadInt(f);
    gamGrowth[slot]    = ReadInt(f);
    gamBlackout[slot]  = ReadInt(f);
    gamBombThreat[slot]= ReadInt(f);
    // promos
    phonePromo         = ReadInt(f);
    for (int i = 1; i <= NO_PROMOS; ++i) promoUsed[i] = ReadInt(f);
    // camera
    camX    = ReadFloat(f);
    camY    = ReadFloat(f);
    camZ    = ReadFloat(f);
    camPivX = ReadFloat(f);
    camPivY = ReadFloat(f);
    camPivZ = ReadFloat(f);
    // atmosphere
    lightR  = ReadFloat(f);
    lightG  = ReadFloat(f);
    lightB  = ReadFloat(f);
    ambR    = ReadFloat(f);
    ambG    = ReadFloat(f);
    ambB    = ReadFloat(f);
    atmosR  = ReadFloat(f);
    atmosG  = ReadFloat(f);
    atmosB  = ReadFloat(f);
    skyR    = ReadFloat(f);
    skyG    = ReadFloat(f);
    skyB    = ReadFloat(f);
    // dinner trays
    for (int i = 1; i <= 50; ++i) trayState[i] = ReadInt(f);
    CloseFile(f);

    // Load characters too
    LoadChars();
}

// ---------------------------------------------------------------------------
//  SaveChars  (mirrors Data.bb SaveChars())
// ---------------------------------------------------------------------------
void SaveChars() {
    for (int charId = 1; charId <= no_chars; ++charId) {
        BBString path = SAVE_ROOT + "Data/Slot0" + Str(slot) + "/Character" + Dig(charId, 100) + ".dat";
        BBFile* f = WriteFile(path);
        if (!f || f->fd < 0) continue;

        // appearance
        WriteString(f, charName[charId]);
        WriteInt(f, charSnapped[charId]);
        WriteInt(f, charModel[charId]);
        WriteInt(f, charHeight[charId]);
        WriteInt(f, charSpecs[charId]);
        WriteInt(f, charAccessory[charId]);
        WriteInt(f, charHairStyle[charId]);
        WriteInt(f, charHair[charId]);
        WriteInt(f, charFace[charId]);
        WriteInt(f, charCostume[charId]);
        for (int count = 1; count <= 40; ++count)
            WriteInt(f, charScar[charId][count]);

        // attributes
        WriteInt(f, charHealth[charId]);
        WriteInt(f, charHP[charId]);
        WriteInt(f, charInjured[charId]);
        WriteInt(f, charStrength[charId]);
        WriteInt(f, charAgility[charId]);
        WriteInt(f, charHappiness[charId]);
        WriteInt(f, charBreakdown[charId]);
        WriteInt(f, charIntelligence[charId]);
        WriteInt(f, charReputation[charId]);
        WriteInt(f, charWeapon[charId]);
        for (int count = 1; count <= 30; ++count)
            WriteInt(f, charWeapHistory[charId][count]);

        // status
        WriteInt(f, charRole[charId]);
        WriteInt(f, charSentence[charId]);
        WriteInt(f, charCrime[charId]);
        WriteInt(f, charLocation[charId]);
        WriteInt(f, charBlock[charId]);
        WriteInt(f, charCell[charId]);
        WriteInt(f, charExperience[charId]);
        WriteFloat(f, charX[charId]);
        WriteFloat(f, charY[charId]);
        WriteFloat(f, charZ[charId]);
        WriteFloat(f, charA[charId]);

        // relationships
        WriteInt(f, charGang[charId]);
        for (int gang = 1; gang <= 6; ++gang)
            WriteInt(f, charGangHistory[charId][gang]);
        WriteInt(f, charAttacker[charId]);
        WriteInt(f, charWitness[charId]);
        WriteInt(f, charPromoRef[charId]);
        WriteInt(f, charFollowTim[charId]);
        WriteInt(f, charBribeTim[charId]);
        for (int v = 1; v <= no_chars; ++v) {
            WriteInt(f, charRelation[charId][v]);
            WriteInt(f, charAngerTim[charId][v]);
            WriteInt(f, charPromo[charId][v]);
        }
        CloseFile(f);
    }
}

// ---------------------------------------------------------------------------
//  LoadChars  (mirrors Data.bb LoadChars())
// ---------------------------------------------------------------------------
void LoadChars() {
    for (int charId = 1; charId <= no_chars; ++charId) {
        BBString path = SAVE_ROOT + "Data/Slot0" + Str(slot) + "/Character" + Dig(charId, 100) + ".dat";
        if (!FileExists(path)) continue;

        BBFile* f = ReadFile(path);
        if (!f || f->fd < 0) continue;

        // appearance
        charName[charId]      = ReadString(f);
        charSnapped[charId]   = ReadInt(f);
        charModel[charId]     = ReadInt(f);
        charHeight[charId]    = ReadInt(f);
        charSpecs[charId]     = ReadInt(f);
        charAccessory[charId] = ReadInt(f);
        charHairStyle[charId] = ReadInt(f);
        charHair[charId]      = ReadInt(f);
        charFace[charId]      = ReadInt(f);
        charCostume[charId]   = ReadInt(f);
        for (int count = 1; count <= 40; ++count)
            charScar[charId][count] = ReadInt(f);

        // attributes
        charHealth[charId]       = ReadInt(f);
        charHP[charId]           = ReadInt(f);
        charInjured[charId]      = ReadInt(f);
        charStrength[charId]     = ReadInt(f);
        charAgility[charId]      = ReadInt(f);
        charHappiness[charId]    = ReadInt(f);
        charBreakdown[charId]    = ReadInt(f);
        charIntelligence[charId] = ReadInt(f);
        charReputation[charId]   = ReadInt(f);
        charWeapon[charId]       = ReadInt(f);
        for (int count = 1; count <= 30; ++count)
            charWeapHistory[charId][count] = ReadInt(f);

        // status
        charRole[charId]       = ReadInt(f);
        charSentence[charId]   = ReadInt(f);
        charCrime[charId]      = ReadInt(f);
        charLocation[charId]   = ReadInt(f);
        charBlock[charId]      = ReadInt(f);
        charCell[charId]       = ReadInt(f);
        charExperience[charId] = ReadInt(f);
        charX[charId]          = ReadFloat(f);
        charY[charId]          = ReadFloat(f);
        charZ[charId]          = ReadFloat(f);
        charA[charId]          = ReadFloat(f);

        // relationships
        charGang[charId] = ReadInt(f);
        for (int gang = 1; gang <= 6; ++gang)
            charGangHistory[charId][gang] = ReadInt(f);
        charAttacker[charId]  = ReadInt(f);
        charWitness[charId]   = ReadInt(f);
        charPromoRef[charId]  = ReadInt(f);
        charFollowTim[charId] = ReadInt(f);
        charBribeTim[charId]  = ReadInt(f);
        for (int v = 1; v <= no_chars; ++v) {
            charRelation[charId][v] = ReadInt(f);
            charAngerTim[charId][v] = ReadInt(f);
            charPromo[charId][v]    = ReadInt(f);
        }
        CloseFile(f);
    }
}

// ---------------------------------------------------------------------------
//  SaveItems  (mirrors Data.bb SaveItems())
// ---------------------------------------------------------------------------
void SaveItems() {
    BBString slotDir = SAVE_ROOT + "Data/Slot0" + Str(slot) + "/";
    EnsureDir(slotDir);
    BBFile* f = WriteFile(slotDir + "Items.dat");
    if (!f || f->fd < 0) return;

    WriteInt(f, no_weaps);
    for (int cyc = 1; cyc <= no_weaps; ++cyc) {
        WriteInt(f, weapType[cyc]);
        WriteInt(f, weapState[cyc]);
        WriteInt(f, weapLocation[cyc]);
        WriteFloat(f, weapX[cyc]);
        WriteFloat(f, weapY[cyc]);
        WriteFloat(f, weapZ[cyc]);
        WriteFloat(f, weapA[cyc]);
        WriteInt(f, weapCarrier[cyc]);
        WriteInt(f, weapClip[cyc]);
        WriteInt(f, weapAmmo[cyc]);
        WriteInt(f, weapScar[cyc]);
    }
    // kits
    for (int count = 1; count <= 6; ++count) {
        WriteInt(f, kitType[count]);
        WriteInt(f, kitState[count]);
    }
    CloseFile(f);
}

// ---------------------------------------------------------------------------
//  LoadItems  (mirrors Data.bb LoadItems())
// ---------------------------------------------------------------------------
void LoadItems() {
    BBString path = SAVE_ROOT + "Data/Slot0" + Str(slot) + "/Items.dat";
    if (!FileExists(path)) return;

    BBFile* f = ReadFile(path);
    if (!f || f->fd < 0) return;

    no_weaps = ReadInt(f);
    for (int cyc = 1; cyc <= no_weaps; ++cyc) {
        weapType[cyc]     = ReadInt(f);
        weapState[cyc]    = ReadInt(f);
        weapLocation[cyc] = ReadInt(f);
        weapX[cyc]        = ReadFloat(f);
        weapY[cyc]        = ReadFloat(f);
        weapZ[cyc]        = ReadFloat(f);
        weapA[cyc]        = ReadFloat(f);
        weapCarrier[cyc]  = ReadInt(f);
        weapClip[cyc]     = ReadInt(f);
        weapAmmo[cyc]     = ReadInt(f);
        weapScar[cyc]     = ReadInt(f);
    }
    // kits
    for (int count = 1; count <= 6; ++count) {
        kitType[count]  = ReadInt(f);
        kitState[count] = ReadInt(f);
    }
    CloseFile(f);
}

// ---------------------------------------------------------------------------
//  SavePhotos / LoadPhotos  (stubs — Vita cannot do BMP I/O like Blitz3D)
// ---------------------------------------------------------------------------
void SavePhotos() {
    // On Vita, photo saving is not supported in the same way as Blitz3D.
    // Photos would need a separate screenshot-capture implementation.
}
void LoadPhotos() {
    // Reset photo state; actual photo loading is not supported on Vita.
    for (int charId = 1; charId <= no_chars; ++charId) {
        charPhoto[charId] = 0;
    }
}

// ---------------------------------------------------------------------------
//  AssignCell  (mirrors Data.bb AssignCell())
// ---------------------------------------------------------------------------
void AssignCell(int charId) {
    int its = 0;
    int block, cell;
    do {
        its++;
        int satisfied = 1;
        block = Rnd(1, 4);
        cell  = Rnd(1, 20);
        if (its < 10 && CellPopulation(block, cell) > 0) satisfied = 0;
        if (CellPopulation(block, cell) > 1) satisfied = 0;
        if (its < 10 && AreaPopulation(TranslateBlock(block), 0) >= optPopulation / 5) satisfied = 0;
        if (satisfied == 1) break;
    } while (its < 1000);
    charBlock[charId] = block;
    charCell[charId]  = cell;
}

// ---------------------------------------------------------------------------
//  FindCellMates  (mirrors Data.bb FindCellMates())
// ---------------------------------------------------------------------------
void FindCellMates() {
    int charId = gamChar[slot];
    for (int v = 1; v <= no_chars; ++v) {
        if (v != charId && charRole[v] == 0 &&
            charCell[v] == charCell[charId] && charBlock[v] == charBlock[charId]) {
            int randy = Rnd(0, 2);
            if (randy == 1 || (randy == 0 && charReputation[v] < charReputation[charId]))
                charPromo[v][charId] = Rnd(202, 203);
            if (randy == 2 || (randy == 0 && charReputation[v] >= charReputation[charId]))
                charPromo[v][charId] = Rnd(203, 204);
        }
    }
}

// ---------------------------------------------------------------------------
//  GenerateName  (mirrors Data.bb GenerateName$())
// ---------------------------------------------------------------------------
static BBString GenerateName(int charId) {
    BBString name;
    int conflict;
    do {
        name = "Character" + Dig(charId, 100);
        // inmates
        if (charRole[charId] == 0) {
            int randy = Rnd(0, 1);
            if (randy == 0) name = textNickName[Rnd(0, 80)];
            if (randy == 1) name = textFirstName[Rnd(0, 65)] + " " + textSurName[Rnd(0, 65)];
        }
        // officials
        if (charRole[charId] >= 1) {
            if (charRole[charId] == 1) name = "Warden ";
            if (charRole[charId] == 2) name = "Lawyer ";
            if (charRole[charId] == 3) name = "Judge ";
            name = name + textSurName[Rnd(0, 65)];
        }
        // find conflicts
        conflict = 0;
        for (int v = 1; v <= no_chars; ++v) {
            if (charName[v] == name) conflict = 1;
        }
    } while (conflict != 0);
    return name;
}

// ---------------------------------------------------------------------------
//  GenerateCharacter  (mirrors Data.bb GenerateCharacter())
// ---------------------------------------------------------------------------
void GenerateCharacter(int charId, int role) {
    // appearance
    charRole[charId] = role;
    charName[charId] = GenerateName(charId);
    charPhoto[charId] = 0; charSnapped[charId] = 0;
    int randy = Rnd(0, 5);
    if (randy == 0) charModel[charId] = 2; else charModel[charId] = Rnd(1, no_models);
    randy = Rnd(0, 2);
    if (randy == 0) charHeight[charId] = Rnd(10, 15); else charHeight[charId] = Rnd(5, 24);
    charSpecs[charId] = Rnd(-10, 4);
    if (charSpecs[charId] < 0) charSpecs[charId] = 0;
    randy = Rnd(0, 2);
    if (randy == 0 && charRole[charId] == 1) charAccessory[charId] = 7; else charAccessory[charId] = 0;
    charFace[charId] = Rnd(1, no_faces);
    charHair[charId] = Rnd(1, no_hairs);
    randy = Rnd(0, 2);
    if (randy <= 1 && charHair[charId] >= 8) charHair[charId] = Rnd(1, 7);
    if (GetRace(charId) == 1 && charHair[charId] >= 3 && charHair[charId] <= 7) charHair[charId] = Rnd(1, 2);
    if (GetRace(charId) == 2 && charHair[charId] >= 2 && charHair[charId] <= 7) charHair[charId] = 1;
    charHairStyle[charId] = Rnd(-30, 31);
    if (charHairStyle[charId] < 0 || charRole[charId] >= 2) charHairStyle[charId] = Rnd(0, 10);
    charCostume[charId] = Rnd(0, 8);
    if (charRole[charId] == 1) charCostume[charId] = 5;
    if (charRole[charId] >= 2) charCostume[charId] = 7;
    charWeapon[charId] = 0;
    for (int count = 1; count <= 30; ++count) charWeapHistory[charId][count] = 0;
    for (int limb = 1; limb <= 40; ++limb) charScar[charId][limb] = 0;

    // inmate location
    if (charRole[charId] == 0) {
        AssignCell(charId);
        charLocation[charId] = TranslateBlock(charBlock[charId]);
        charX[charId] = GetCentre(cellX1[charCell[charId]], cellX2[charCell[charId]]);
        charZ[charId] = GetCentre(cellZ1[charCell[charId]], cellZ2[charCell[charId]]);
        charY[charId] = cellY1[charCell[charId]] + 20;
        charA[charId] = (float)Rnd(0, 360);
    }
    // warden location
    if (charRole[charId] == 1) {
        int its = 0, area;
        do {
            area = Rnd(1, 10); its++;
        } while (AreaPopulation(area, 1) != 0 && its < 100);
        charLocation[charId] = area;
        charX[charId] = (float)Rnd(-100, 100); charZ[charId] = (float)Rnd(-100, 100);
        if (charLocation[charId] == 2) { charX[charId] = (float)Rnd(250, 450); charZ[charId] = (float)Rnd(250, 450); }
        charY[charId] = 50; charA[charId] = (float)Rnd(0, 360);
    }

    // attributes
    charHealth[charId] = Rnd(10, 100); charHP[charId] = 10;
    if (charModel[charId] == 1) { charStrength[charId] = Rnd(40, 70); charAgility[charId] = Rnd(70, 100); }
    if (charModel[charId] == 2) { charStrength[charId] = Rnd(50, 80); charAgility[charId] = Rnd(60, 90); }
    if (charModel[charId] == 3) { charStrength[charId] = Rnd(60, 90); charAgility[charId] = Rnd(60, 90); }
    if (charModel[charId] >= 4) { charStrength[charId] = Rnd(60, 90); charAgility[charId] = Rnd(50, 80); }
    charStrength[charId] += charHeight[charId] / 2;
    charAgility[charId]  -= charHeight[charId] / 2;
    charHappiness[charId]    = Rnd(10, 100);
    charIntelligence[charId] = Rnd(50, 100);
    charReputation[charId]   = Rnd(50, 100);
    if (charRole[charId] > 0) { charIntelligence[charId] = Rnd(70, 100); charReputation[charId] = Rnd(70, 100); }
    if (charRole[charId] > 0) charSentence[charId] = 0; else charSentence[charId] = Rnd(1, 365);
    if (charRole[charId] > 0) charCrime[charId] = 0; else charCrime[charId] = Rnd(1, 15);
    charExperience[charId] = 0;

    // gang membership
    for (int gang = 1; gang <= 6; ++gang) charGangHistory[charId][gang] = 0;
    charGang[charId] = Rnd(-1, 6);
    if (charGang[charId] < 0 || charRole[charId] > 0 || charId == gamChar[slot]) charGang[charId] = 0;
    if (charGang[charId] == 1 && GetRace(charId) > 0) charGang[charId] = 0;
    if (charGang[charId] == 2 && GetRace(charId) != 1) charGang[charId] = 0;
    if (charGang[charId] == 3 && GetRace(charId) != 2) charGang[charId] = 0;
    if (charGang[charId] == 4 && charIntelligence[charId] < 70) charGang[charId] = 0;
    if (charGang[charId] == 5 && charStrength[charId] + charAgility[charId] < 140) charGang[charId] = 0;
    if (charGang[charId] == 6 && charReputation[charId] > 80) charGang[charId] = 0;
    if (charGang[charId] > 0) charGangHistory[charId][charGang[charId]] = 1;
    GangAdjust(charId);

    // relationships
    for (int v = 1; v <= no_chars; ++v) {
        ChangeRelationship(charId, v, 0);
        if (charId != gamChar[slot] && v != gamChar[slot]) {
            randy = Rnd(0, 20);
            if (randy == 0) ChangeRelationship(charId, v, 1);
            if (randy == 1) ChangeRelationship(charId, v, -1);
            if (randy <= 5 && charRole[charId] == 1 && charRole[v] == 1) ChangeRelationship(charId, v, 1);
            if (randy <= 5 && charGang[charId] > 0 && charGang[charId] == charGang[v]) ChangeRelationship(charId, v, 1);
        }
        charAngerTim[charId][v] = 0;
        charPromo[charId][v] = 0;
    }
    charAttacker[charId] = 0;
    charWitness[charId] = 0;
    charFollowTim[charId] = 0;
    charBribeTim[charId] = 0;

    // risk dead status
    randy = Rnd(0, 20);
    if (randy == 0 && charId != gamChar[slot]) {
        charLocation[charId] = 0;
        charHealth[charId] = Rnd(0, 1);
    }
}

// ---------------------------------------------------------------------------
//  GenerateWeapon  (mirrors Data.bb GenerateWeapon())
// ---------------------------------------------------------------------------
void GenerateWeapon(int cyc, int style, int area, float x, float y, float z) {
    // type
    weapType[cyc] = style;
    if (weapType[cyc] == 0) {
        weapType[cyc] = Rnd(1, weapList);
        int randy = Rnd(1, 20);
        if (randy == 1) weapType[cyc] = Rnd(24, 25);
        if (randy == 2) weapType[cyc] = 15;
        if (randy >= 3 && randy <= 5) weapType[cyc] = 16;
        if (randy >= 6 && randy <= 8) weapType[cyc] = Rnd(16, 18);
    }
    // general location
    weapLocation[cyc] = area;
    if (area == 0) {
        int randy = Rnd(0, 20);
        if (randy <= 1)                    weapLocation[cyc] = 1;
        if (randy >= 2 && randy <= 4)      weapLocation[cyc] = 2;
        if (randy >= 4 && randy <= 5)      weapLocation[cyc] = 3;
        if (randy == 6)                    weapLocation[cyc] = 4;
        if (randy >= 7 && randy <= 8)      weapLocation[cyc] = 5;
        if (randy == 9)                    weapLocation[cyc] = 6;
        if (randy >= 10 && randy <= 11)    weapLocation[cyc] = 7;
        if (randy >= 12 && randy <= 13)    weapLocation[cyc] = 8;
        if (randy >= 14 && randy <= 16)    weapLocation[cyc] = 9;
        if (randy == 17)                   weapLocation[cyc] = 10;
        if (randy >= 18 && randy <= 20)    weapLocation[cyc] = 11;
    }
    int randy = Rnd(0, 4);
    if (randy == 0 && style == 0 && area == 0) weapLocation[cyc] = 0;
    // favour habitat
    if (weapLocation[cyc] > 0 && GetBlock(weapLocation[cyc]) == 0 && weapType[cyc] != 16) {
        randy = Rnd(0, 2);
        if (randy > 0 && weapHabitat[weapType[cyc]] > 0 && weapHabitat[weapType[cyc]] != 99)
            weapLocation[cyc] = weapHabitat[weapType[cyc]];
        if (randy > 0 && weapType[cyc] == 14) weapLocation[cyc] = 2;
        if (randy == 0 && weapType[cyc] >= 24 && weapType[cyc] <= 25) weapLocation[cyc] = 11;
    }
    // pinpoint location
    weapX[cyc] = x; weapY[cyc] = y; weapZ[cyc] = z;
    weapA[cyc] = (float)Rnd(0, 360);
    if (weapX[cyc] == 0 && weapY[cyc] == 0 && weapZ[cyc] == 0) {
        weapY[cyc] = 50;
        // cell block locations
        if (GetBlock(weapLocation[cyc]) > 0) {
            randy = Rnd(0, 9);
            if (randy <= 5) {
                int its = 0;
                do {
                    weapX[cyc] = (float)Rnd(-300, 300);
                    weapZ[cyc] = (float)Rnd(-140, 350);
                    if (randy == 0) weapY[cyc] = 50; else weapY[cyc] = 150;
                    its++;
                } while (InsideCell(weapX[cyc], weapY[cyc], weapZ[cyc]) == 0 && its < 100);
            }
            if (randy == 6) { weapX[cyc] = (float)Rnd(-190, 60);  weapZ[cyc] = (float)Rnd(-140, 250); }
            if (randy == 7) { weapX[cyc] = (float)Rnd(60, 190);   weapZ[cyc] = (float)Rnd(-140, 250); }
            if (randy == 8) { weapX[cyc] = (float)Rnd(-115, 115); weapZ[cyc] = (float)Rnd(-335, 15);  }
            if (randy == 9) { weapX[cyc] = (float)Rnd(-80, 80);   weapZ[cyc] = (float)Rnd(220, 350);  weapY[cyc] = 150; }
        }
        // yard
        if (weapLocation[cyc] == 2) {
            randy = Rnd(1, 2);
            if (randy == 1) { weapX[cyc] = (float)Rnd(-20, 475);  weapZ[cyc] = (float)Rnd(210, 475); }
            if (randy == 2) { weapX[cyc] = (float)Rnd(210, 475);  weapZ[cyc] = (float)Rnd(-50, 475); }
            if (weapType[cyc] == 11) { weapX[cyc] = (float)Rnd(210, 475); weapZ[cyc] = (float)Rnd(-50, 200); }
            if (weapType[cyc] == 14) { weapX[cyc] = (float)Rnd(-30, 100); weapZ[cyc] = (float)Rnd(270, 425); }
        }
        // study
        if (weapLocation[cyc] == 4) {
            randy = Rnd(1, 5);
            if (randy == 1) { weapX[cyc] = (float)Rnd(-135, 135); weapZ[cyc] = (float)Rnd(-130, -40); }
            if (randy == 2) { weapX[cyc] = (float)Rnd(-120, 135); weapZ[cyc] = (float)Rnd(40, 120); }
            if (randy == 3) { weapX[cyc] = (float)Rnd(-120, -40); weapZ[cyc] = (float)Rnd(-135, 120); }
            if (randy == 4) { weapX[cyc] = (float)Rnd(40, 135);   weapZ[cyc] = (float)Rnd(-125, 105); }
            if (randy == 5) { weapX[cyc] = (float)Rnd(-140, 140); weapZ[cyc] = (float)Rnd(-140, 140); }
        }
        // hospital
        if (weapLocation[cyc] == 6) { weapX[cyc] = (float)Rnd(-140, 140); weapZ[cyc] = (float)Rnd(-140, 140); }
        // kitchen
        if (weapLocation[cyc] == 8) {
            randy = Rnd(1, 4);
            if (randy == 1) { weapX[cyc] = (float)Rnd(-105, 105); weapZ[cyc] = (float)Rnd(-325, -160); }
            if (randy == 2) { weapX[cyc] = (float)Rnd(-105, 250); weapZ[cyc] = (float)Rnd(-160, 250); }
            if (randy == 3) { weapX[cyc] = (float)Rnd(-250, 250); weapZ[cyc] = (float)Rnd(170, 325); }
            if (randy == 4) { weapX[cyc] = (float)Rnd(-240, -145); weapZ[cyc] = (float)Rnd(-120, 140); }
        }
        // hall
        if (weapLocation[cyc] == 9) { weapX[cyc] = (float)Rnd(-295, 295); weapZ[cyc] = (float)Rnd(-295, 295); }
        // workshop
        if (weapLocation[cyc] == 10) {
            randy = Rnd(1, 4);
            if (randy == 1) { weapX[cyc] = (float)Rnd(-95, 95);  weapZ[cyc] = (float)Rnd(-115, 115); }
            if (randy == 2) { weapX[cyc] = (float)Rnd(-65, -30); weapZ[cyc] = -114; weapY[cyc] = (float)Rnd(20, 35); }
            if (randy == 3) { weapX[cyc] = (float)Rnd(30, 70);   weapZ[cyc] = -114; weapY[cyc] = (float)Rnd(20, 35); }
            if (randy == 4) { weapX[cyc] = (float)Rnd(-20, 20);  weapZ[cyc] = 119;  weapY[cyc] = (float)Rnd(20, 35); }
        }
        // toilet
        if (weapLocation[cyc] == 11) {
            randy = Rnd(1, 7);
            if (randy >= 1 && randy <= 2) { weapX[cyc] = (float)Rnd(-140, 50);  weapZ[cyc] = (float)Rnd(-65, 10); }
            if (randy >= 3 && randy <= 4) { weapX[cyc] = (float)Rnd(50, 140);   weapZ[cyc] = (float)Rnd(-65, 70); }
            if (randy == 5) { weapX[cyc] = (float)Rnd(-140, -115); weapZ[cyc] = (float)Rnd(10, 70); }
            if (randy == 6) { weapX[cyc] = (float)Rnd(-70, -40);   weapZ[cyc] = (float)Rnd(10, 70); }
            if (randy == 7) { weapX[cyc] = (float)Rnd(0, 30);      weapZ[cyc] = (float)Rnd(10, 70); }
        }
    }
    // reset status
    weapState[cyc] = 1; weapCarrier[cyc] = 0;
    weapScar[cyc] = 0;  weapOldScar[cyc] = -1;
    weapAmmo[cyc] = 100; weapClip[cyc] = 10;
    if (weapStyle[weapType[cyc]] == 6) weapClip[cyc] = 0;
}

// ---------------------------------------------------------------------------
//  GenerateGame  (mirrors Data.bb GenerateGame())
// ---------------------------------------------------------------------------
void GenerateGame() {
    // initiate characters
    no_chars = optPopulation + 3;
    int no_wardens = optPopulation / 5;
    if (no_wardens < 10) no_wardens = 10;
    for (int charId = 1; charId <= no_chars; ++charId) {
        charRole[charId] = 0;
        charLocation[charId] = 0;
        charBlock[charId] = 0;
        charCell[charId] = 0;
        charName[charId] = "Character" + Dig(charId, 100);
    }
    gamChar[slot] = Rnd(no_wardens + 4, no_chars);
    for (int charId = 1; charId <= no_chars; ++charId) {
        if (charId <= 2) GenerateCharacter(charId, 2);
        if (charId == 3) GenerateCharacter(charId, 3);
        if (charId >= 4) {
            if (charId - 3 <= no_wardens) GenerateCharacter(charId, 1);
            else GenerateCharacter(charId, 0);
        }
    }
    // reset player status
    int charId = gamChar[slot];
    charHealth[charId] = 50;
    charStrength[charId] = 50;
    charAgility[charId] = 50;
    charHappiness[charId] = 50;
    charIntelligence[charId] = 50;
    charReputation[charId] = 50;
    gamMoney[slot] = 0;
    // reset clock
    gamSecs[slot] = 0; gamMins[slot] = 0; gamHours[slot] = Rnd(8, 20);
    // missions
    gamMission[slot] = 0; gamClient[slot] = 0; gamTarget[slot] = 0;
    gamDeadline[slot] = 0; gamReward[slot] = 0;
    // reset game status
    gamWarrant[slot] = 0; gamArrival[slot] = 0; gamFatality[slot] = 0;
    gamRelease[slot] = 0; gamEscape[slot] = 0; gamGrowth[slot] = 0;
    // reset promos
    for (int i = 1; i <= NO_PROMOS; ++i) promoUsed[i] = 0;
    // find cell mates
    FindCellMates();
    // initial location
    charLocation[charId] = 9;
    gamLocation[slot] = charLocation[charId];
    charX[charId] = 0; charZ[charId] = 0;
    camX = 0; camY = 75; camZ = 0;
    camPivX = camX; camPivY = camY; camPivZ = camZ;
    // generate weapons
    no_weaps = optPopulation;
    for (int cyc = 1; cyc <= no_weaps; ++cyc) {
        if (cyc == 1) GenerateWeapon(cyc, 14, 2, (float)Rnd(-30, 100), 50, (float)Rnd(270, 425));
        if (cyc == 2 || cyc == 3) GenerateWeapon(cyc, 5, 9, (float)Rnd(-30, 40), 50, (float)Rnd(235, 280));
        if (cyc >= 4) GenerateWeapon(cyc, 0, 0, 0, 0, 0);
    }
    // distribute weapons
    for (int cyc = 1; cyc <= no_weaps; ++cyc) {
        if (weapLocation[cyc] > 0 && (weapType[cyc] == 7 || weapType[cyc] == 8 || weapType[cyc] == 12)) {
            for (int v = 1; v <= no_chars; ++v) {
                if (charRole[v] == 1 && charWeapon[v] == 0 && FindCarrier(cyc) == 0) {
                    charWeapon[v] = cyc;
                    weapLocation[cyc] = charLocation[v];
                    weapX[cyc] = charX[v]; weapY[cyc] = charY[v] + 10; weapZ[cyc] = charZ[v];
                }
            }
        }
        if (weapLocation[cyc] > 0) {
            for (int v = 1; v <= no_chars; ++v) {
                int randy = Rnd(0, 100);
                if (randy == 0 && charRole[v] == 0 && v != gamChar[slot] && charWeapon[v] == 0 &&
                    FindCarrier(cyc) == 0 && weapState[cyc] > 0) {
                    charWeapon[v] = cyc;
                    weapLocation[cyc] = charLocation[v];
                    weapX[cyc] = charX[v]; weapY[cyc] = charY[v] + 10; weapZ[cyc] = charZ[v];
                }
            }
        }
    }
    // generate kits
    for (int cyc = 1; cyc <= 6; ++cyc) {
        do { kitType[cyc] = Rnd(1, weapList); } while (weapCreate[kitType[cyc]] != 1);
        int randy = Rnd(0, 2);
        if (randy <= 1) kitState[cyc] = 1;
    }
    // save generation
    SaveProgress();
    SaveChars();
    SavePhotos();
    SaveItems();
}
