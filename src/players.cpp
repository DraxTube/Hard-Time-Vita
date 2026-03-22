// =============================================================================
//  players.cpp  –  Player / character / weapon array definitions
//                  LoadPlayers, LoadWeapons, PlayerCycle  (from Players.bb + Weapons.bb)
// =============================================================================
#include "players.h"
#include "values.h"
#include "functions.h"
#include "costume.h"
#include "texts.h"
#include "anims.h"
#include "ai.h"
#include "blitz_compat.h"
#include "render3d.h"

// ---------------------------------------------------------------------------
//  Array definitions
// ---------------------------------------------------------------------------
std::array<Handle, P> p             {};
std::array<Handle, P> pPivot        {};
std::array<Handle, P> pMovePivot    {};
std::array<float,  P> pX{}, pY{}, pZ{}, pA{};
std::array<float,  P> pTX{}, pTY{}, pTZ{}, pTA{};
std::array<float,  P> pOldX{}, pOldZ{};
std::array<float,  P> pGravity{}, pGround{};
std::array<float,  P> pSubX{}, pSubZ{};
std::array<float,  P> pExploreX{}, pExploreY{}, pExploreZ{};
std::array<float,  P> pCharge{};
std::array<float,  P> pSeatX{}, pSeatY{}, pSeatZ{}, pSeatA{};
std::array<float,  P> pAnimSpeed{};

std::array<int, P> pChar{}, pControl{}, pAgenda{}, pOldAgenda{};
std::array<int, P> pState{}, pAnim{}, pAnimTim{};
std::array<int, P> pHealth{}, pHealthLimit{}, pHP{}, pInjured{};
std::array<int, P> pWeapon{}, pPhone{}, pSeat{}, pBed{};
std::array<int, P> pGrappling{}, pGrappler{};
std::array<int, P> pNowhere{}, pSatisfied{}, pRunTim{}, pControlTim{};
std::array<int, P> pEyes{}, pOldEyes{}, pCollisions{};
std::array<int, P> pFollowFoc{}, pWeapFoc{};

std::array<std::array<Handle, LIMBS>, P> pLimb  {};
std::array<std::array<int,    LIMBS>, P> pScar   {};
std::array<std::array<int,    LIMBS>, P> pOldScar{};
std::array<std::array<Handle, LIMBS>, P> pShadow {};
std::array<std::array<int, SEQ_MAX>,  P> pSeq    {};
std::array<std::array<int, P>,        P> pInteract{};
std::array<std::array<int, 51>,       P> pSeatFriction{};
std::array<std::array<int, 21>,       P> pBedFriction{};
std::array<std::array<int, WL>,       P> pWeaponTim{};

std::array<int, P> cUp{}, cDown{}, cLeft{}, cRight{};
std::array<int, P> cAttack{}, cDefend{}, cThrow{}, cPickUp{};

int camFoc = 0;

// Character arrays
std::array<std::string, C> charName{};
std::array<int, C> charModel{}, charHeight{}, charSpecs{};
std::array<int, C> charAccessory{}, charHairStyle{}, charHair{}, charFace{}, charCostume{};
std::array<int, C> charHealth{}, charHP{};
std::array<int, C> charStrength{}, charAgility{}, charHappiness{}, charBreakdown{};
std::array<int, C> charIntelligence{}, charReputation{};
std::array<int, C> charOldStrength{}, charOldAgility{}, charOldIntelligence{}, charOldReputation{};
std::array<int, C> charExperience{}, charPlayer{};
std::array<int, C> charPhoto{}, charSnapped{};
std::array<int, C> charRole{}, charSentence{}, charCrime{};
std::array<int, C> charLocation{}, charBlock{}, charCell{}, charGang{};
std::array<int, C> charAttacker{}, charWitness{};
std::array<int, C> charFollowTim{}, charBribeTim{};
std::array<int, C> charWeapon{}, charInjured{};
std::array<float, C> charX{}, charY{}, charZ{}, charA{};

std::array<std::array<int, LIMBS>, C> charScar     {};
std::array<std::array<int, 7>,     C> charGangHistory{};
std::array<std::array<int, C>,     C> charRelation {};
std::array<std::array<int, C>,     C> charAngerTim {};
std::array<std::array<int, 31>,    C> charWeapHistory{};
std::array<std::array<int, C>,     C> charPromo     {};

// Weapon runtime
std::array<Handle, WL> weap{}, weapGround{}, weapWall{};
std::array<int, WL>    weapType{}, weapCarrier{}, weapThrower{};
std::array<int, WL>    weapClip{}, weapAmmo{}, weapScar{}, weapOldScar{};
std::array<int, WL>    weapState{}, weapLocation{};
std::array<float, WL>  weapX{}, weapY{}, weapZ{};
std::array<float, WL>  weapOldX{}, weapOldZ{}; // weapOldY defined in moves.cpp
std::array<float, WL>  weapA{}, weapFlight{}, weapFlightA{}, weapGravity{}, weapBounce{};
std::array<std::array<int, P>, WL> weapSting{};

// Weapon type data
int weapList = 25;
std::array<std::string, WLS> weapName{}, weapFile{};
std::array<BBSound*, WLS>   weapSound{};
std::array<GLuint,   WLS>    weapTex{};
std::array<float, WLS>       weapSize{}, weapWeight{}, weapRange{}, weapShiny{};
std::array<int, WLS>         weapValue{}, weapDamage{}, weapStyle{};

// World
Handle world  = 0;
int no_chairs = 0, no_beds = 0, no_doors = 0;
std::array<Handle, 21> tSign{}, tCell{};
std::array<Handle, 5>  tBlock{};
Handle tFence = 0, tNet = 0;
std::array<std::array<int, 21>, 12> cellLocked{};

// ---------------------------------------------------------------------------
//  LoadSequences – load animation sequence indices for a player
//  (Blitz3D stored anim seq IDs; in the port these map to glTF/3DS clip IDs)
// ---------------------------------------------------------------------------
void LoadSequences(int cyc) {
    // Delegated to anims.cpp which has the full 200+ clip table
    LoadAnimSequences(cyc);
}

// ---------------------------------------------------------------------------
//  ApplyCostume  –  texture the player mesh according to charCostume/charHair etc.
// ---------------------------------------------------------------------------
// ApplyCostume is defined in costume.cpp

// ---------------------------------------------------------------------------
//  SeverLimbs  –  hide severed limbs that were chopped in a previous session
// ---------------------------------------------------------------------------
void SeverLimbs(int cyc) {
    for (int limb = 1; limb < LIMBS; ++limb) {
        if (pLimb[cyc][limb] > 0 && charScar[pChar[cyc]][limb] >= 5) {
            HideEntity(pLimb[cyc][limb]);
        }
    }
}

// ---------------------------------------------------------------------------
//  LoadPlayers  (converted from Players.bb)
// ---------------------------------------------------------------------------
void LoadPlayers() {
    const std::string BASE = "ux0:data/HardTime/";

    for (int cyc = 1; cyc <= no_plays; ++cyc) {
        // Load animated mesh
        BBString modelFile = BASE + "Characters/Models/Model" +
                             Dig(charModel[pChar[cyc]], 10) + ".3ds";
        Loader("Please Wait", "Loading Character " + Str(cyc) + " of " + Str(no_plays));

        p[cyc] = LoadAnimMesh(modelFile, 0);
        LoadAnimSequences(cyc);  // loads all 200+ animation clips

        // Appearance
        ApplyCostume(cyc);

        // Restore scars from save data
        for (int limb = 1; limb < LIMBS; ++limb) {
            pOldScar[cyc][limb] = -1;
            pScar[cyc][limb]    = charScar[pChar[cyc]][limb];
        }
        SeverLimbs(cyc);

        // Hide held weapons / phone by default
        Handle hPhone   = FindChild(p[cyc], "Phone");
        Handle hBarbell = FindChild(p[cyc], "Barbell");
        if (hPhone   > 0) HideEntity(hPhone);
        if (hBarbell > 0) HideEntity(hBarbell);
        for (int v = 1; v <= weapList; ++v) {
            Handle hw = FindChild(p[cyc], weapFile[v]);
            if (hw > 0) HideEntity(hw);
        }

        // Position
        pX[cyc]    = charX[pChar[cyc]];
        pZ[cyc]    = charZ[pChar[cyc]];
        pOldX[cyc] = pX[cyc]; pOldZ[cyc] = pZ[cyc];
        pTX[cyc]   = pX[cyc]; pTZ[cyc]   = pZ[cyc];
        pA[cyc]    = charA[pChar[cyc]];
        pTA[cyc]   = pA[cyc];
        pY[cyc]    = charY[pChar[cyc]] + 20.0f;
        pGravity[cyc] = 1.0f;

        PositionEntity(p[cyc], pX[cyc], pY[cyc], pZ[cyc]);
        RotateEntity(p[cyc], 0.0f, pA[cyc], 0.0f);

        float scaler = charHeight[pChar[cyc]] * 0.0025f;
        ScaleEntity(p[cyc], 0.34f + scaler, 0.34f + scaler, 0.34f + scaler);
        Animate(p[cyc], 1, 0.5f, pSeq[cyc][1], 0.0f);

        // Collision pivots
        pPivot[cyc] = CreatePivot();
        EntityType(pPivot[cyc], 1, false);
        EntityRadius(pPivot[cyc], 8.0f, 18.0f);
        PositionEntity(pPivot[cyc], pX[cyc], pY[cyc] + 18.0f, pZ[cyc]);

        pMovePivot[cyc] = CreatePivot();
        EntityType(pMovePivot[cyc], 4, false);
        EntityRadius(pMovePivot[cyc], 8.0f, 18.0f);
        PositionEntity(pMovePivot[cyc], pX[cyc], pY[cyc] + 18.0f, pZ[cyc]);

        // Shadows (if enabled)
        const std::string shadowPath = BASE + "World/Sprites/Shadow.png";
        for (int limb = 1; limb < LIMBS; ++limb) {
            pShadow[cyc][limb] = 0;
            bool doShadow = (limb == 30) ||
                            (optShadows == 2 &&
                             (limb == 1 ||
                              (limb >= 4  && limb <= 6)  ||
                              (limb >= 17 && limb <= 19) ||
                              limb == 32 || limb == 33 ||
                              limb == 35 || limb == 36));
            if (doShadow) {
                pShadow[cyc][limb] = CreateSprite();
                float sprScale = (limb == 30) ? 13.0f : 10.0f;
                if (limb == 6 || limb == 19 || limb == 33 || limb == 36) sprScale = 8.0f;
                ScaleSprite(pShadow[cyc][limb], sprScale, sprScale);
                RotateEntity(pShadow[cyc][limb], 90.0f, 0.0f, 0.0f);
                SpriteViewMode(pShadow[cyc][limb], 2);
                EntityColor(pShadow[cyc][limb], 10, 10, 10);
                PositionEntity(pShadow[cyc][limb], pX[cyc], pY[cyc], pZ[cyc]);
            }
        }

        // Reset runtime state
        pAnim[cyc] = 0; pAnimTim[cyc] = 0; pState[cyc] = 0;
        pAgenda[cyc] = Rnd(0, 2); pNowhere[cyc] = 99;
        pSubX[cyc] = 9999.0f; pSubZ[cyc] = 9999.0f;
        pEyes[cyc] = 2; pOldEyes[cyc] = -1;
        pHealth[cyc] = charHealth[pChar[cyc]];
        pHP[cyc]     = charHP[pChar[cyc]];
        pInjured[cyc]= charInjured[pChar[cyc]];
        pWeapon[cyc] = 0; pPhone[cyc] = 0;
        pSeat[cyc]   = 0; pBed[cyc]   = 0;
        pGrappling[cyc] = 0; pGrappler[cyc] = 0;

        for (int v = 1; v <= no_plays; ++v) pInteract[cyc][v] = 0;

        // Assign control
        charPlayer[pChar[cyc]] = cyc;
        pControl[cyc] = 0;
        if (pChar[cyc] == gamChar[slot]) {
            pControl[cyc] = 3;   // full human control (stick + buttons)
            camFoc = cyc;
            gamPlayer[slot] = cyc;
        }
    }
}

// ---------------------------------------------------------------------------
//  PlayerCycle  (converted from Players.bb)
// ---------------------------------------------------------------------------
void PlayerCycle() {
    for (int cyc = 1; cyc <= no_plays; ++cyc) {
        // Countdown timers
        if (--pNowhere[cyc]   < 0) pNowhere[cyc]   = 0;
        if (--pSatisfied[cyc] < 0) pSatisfied[cyc] = 0;
        if (--pRunTim[cyc]    < 0) pRunTim[cyc]    = 0;

        // Social timers
        if (gamPromo == 0) {
            charFollowTim[pChar[cyc]] -= gamSpeed[slot];
            if (charFollowTim[pChar[cyc]] < 0 ||
                charRelation[pChar[cyc]][gamChar[slot]] < 0)
                charFollowTim[pChar[cyc]] = 0;

            charBribeTim[pChar[cyc]] -= gamSpeed[slot];
            if (charBribeTim[pChar[cyc]] < 0 ||
                charRelation[pChar[cyc]][gamChar[slot]] < 0)
                charBribeTim[pChar[cyc]] = 0;
        }

        // Sync position from collision pivot
        if (pSeat[cyc] == 0 && pBed[cyc] == 0) {
            if (pShadow[cyc][30] > 0)
                pGround[cyc] = EntityY(pShadow[cyc][30]);
            pX[cyc] = EntityX(pPivot[cyc]);
            pY[cyc] = EntityY(pPivot[cyc]) - 18.0f;
            pZ[cyc] = EntityZ(pPivot[cyc]);
        }

        // Update health limit
        pHealthLimit[cyc] = std::max(1, charStrength[pChar[cyc]] * 10);

        // Clamping health and managing death
        if (pHealth[cyc] < 0) pHealth[cyc] = 0;
        if (pHealth[cyc] > pHealthLimit[cyc]) pHealth[cyc] = pHealthLimit[cyc];
        charHealth[pChar[cyc]] = pHealth[cyc];
        
        if (pHealth[cyc] <= 0 && pAnim[cyc] < 70) {
            ChangeAnim(cyc, Rnd(78, 79)); // die back or die front
        }
        
        // Manage status timers
        if (pDazed[cyc] > 0) pDazed[cyc]--;
        if (pDT[cyc] > 0) pDT[cyc]--;
        if (pInjured[cyc] > 0) pInjured[cyc] -= gamSpeed[slot];
        if (pInjured[cyc] < 0) pInjured[cyc] = 0;

        // Position the main mesh to match the pivot
        if (p[cyc] > 0) {
            PositionEntity(p[cyc], pX[cyc], pY[cyc], pZ[cyc]);
            RotateEntity(p[cyc], 0.0f, pA[cyc], 0.0f);
        }

        // Position move pivot
        if (pMovePivot[cyc] > 0)
            PositionEntity(pMovePivot[cyc], pX[cyc], pY[cyc] + 18.0f, pZ[cyc]);

        // Shadow positions (update to limb world positions)
        if (pShadow[cyc][30] > 0)
            PositionEntity(pShadow[cyc][30], pX[cyc], pGround[cyc], pZ[cyc]);
    }
}

// ---------------------------------------------------------------------------
//  LoadWeapons  (converted from Weapons.bb)
// ---------------------------------------------------------------------------
void LoadWeapons() {
    const std::string BASE = "ux0:data/HardTime/";

    for (int cyc = 1; cyc <= no_weaps; ++cyc) {
        if (weapLocation[cyc] != gamLocation[slot]) continue;

        BBString meshPath = BASE + "Weapons/" + weapFile[weapType[cyc]] + ".3ds";
        weap[cyc] = LoadAnimMesh(meshPath, 0);
        ScaleEntity(weap[cyc], 0.4f, 0.4f, 0.4f);

        // Apply texture
        if (weapTex[weapType[cyc]] > 0) {
            int numChildren = CountChildren(weap[cyc]);
            for (int c = 1; c <= numChildren; ++c)
                EntityTexture(GetChild(weap[cyc], c), weapTex[weapType[cyc]]);
        }
        // Apply shininess
        if (weapShiny[weapType[cyc]] > 0.0f) {
            int numChildren = CountChildren(weap[cyc]);
            for (int c = 1; c <= numChildren; ++c)
                EntityShininess(GetChild(weap[cyc], c), weapShiny[weapType[cyc]]);
        }
        if (weapName[weapType[cyc]] == "Bottle")
            EntityAlpha(weap[cyc], 0.9f);

        // Collision
        EntityType(weap[cyc], 3, false);
        EntityRadius(weap[cyc], 4.0f, 1.0f);

        weapGround[cyc] = CreatePivot();
        EntityType(weapGround[cyc], 4, false);
        EntityRadius(weapGround[cyc], 4.0f, 1.0f);

        weapWall[cyc] = CreatePivot();
        EntityType(weapWall[cyc], 4, false);
        EntityRadius(weapWall[cyc], 4.0f, 1.0f);

        // Position
        weapY[cyc] += 10.0f;
        PositionEntity(weap[cyc], weapX[cyc], weapY[cyc], weapZ[cyc]);
        RotateEntity(weap[cyc], 0.0f, weapA[cyc], 0.0f);
        if (weapState[cyc] == 0) HideEntity(weap[cyc]);

        // Reset physics
        weapGravity[cyc] = -1.0f;
        weapFlight[cyc]  = 0.0f;
        weapCarrier[cyc] = 0;
        weapOldScar[cyc] = -1;
    }
}

// ---------------------------------------------------------------------------
//  WeaponCycle  (converted from Weapons.bb)
// ---------------------------------------------------------------------------
void WeaponCycle() {
    for (int cyc = 1; cyc <= no_weaps; ++cyc) {
        if (weapLocation[cyc] != gamLocation[slot] || weapState[cyc] == 0) continue;

        // Store old positions
        weapOldX[cyc] = weapX[cyc];
        weapOldY[cyc] = weapY[cyc];
        weapOldZ[cyc] = weapZ[cyc];

        // Sync from entity
        weapX[cyc] = EntityX(weap[cyc]);
        weapY[cyc] = EntityY(weap[cyc]);
        weapZ[cyc] = EntityZ(weap[cyc]);

        if (weapCarrier[cyc] == 0) {
            // Gravity
            weapGravity[cyc] += 0.5f;
            if (weapGravity[cyc] > 10.0f) weapGravity[cyc] = 10.0f;
            TranslateEntity(weap[cyc], 0.0f, -weapGravity[cyc], 0.0f, true);

            // Floor clamp
            if (weapY[cyc] < 5.0f) {
                weapY[cyc] = 5.0f;
                weapGravity[cyc] = 0.0f;
                PositionEntity(weap[cyc], weapX[cyc], weapY[cyc], weapZ[cyc]);
            }

            // Sync ground/wall pivots
            PositionEntity(weapGround[cyc], weapX[cyc], weapY[cyc], weapZ[cyc]);
            PositionEntity(weapWall[cyc],   weapX[cyc], weapY[cyc], weapZ[cyc]);
        } else {
            // Weapon is carried – attach to carrier's hand bone
            int carrier = weapCarrier[cyc];
            if (carrier >= 1 && carrier <= no_plays) {
                PositionEntity(weap[cyc], pX[carrier], pY[carrier] + 30.0f, pZ[carrier]);
                RotateEntity(weap[cyc], 0.0f, pA[carrier], 0.0f);
            }
            weapGravity[cyc] = 0.0f;
        }
    }
}

// ---------------------------------------------------------------------------
//  LoadWorld  (converted from World.bb)
// ---------------------------------------------------------------------------
void LoadWorld() {
    const std::string BASE = "ux0:data/HardTime/";
    int loc = gamLocation[slot];

    if (GetBlock(loc) > 0) {
        world = LoadAnimMesh(BASE + "World/Block/Block.3ds", 0);
        EntityType(world, 10, true);
        Animate(world, 3, 10.0f);
        cellLocked[loc][0] = 0;
        for (int c = 1; c <= 20; ++c) {
            // Block textures
            Handle plate = FindChild(world, "Plate" + Dig(c, 10));
            if (loc == 1 && tBlock[1] > 0) EntityTexture(plate, tBlock[1], 0, 2);
            if (loc == 3 && tBlock[2] > 0) EntityTexture(plate, tBlock[2], 0, 2);
            if (loc == 5 && tBlock[3] > 0) EntityTexture(plate, tBlock[3], 0, 2);
            if (loc == 7 && tBlock[4] > 0) EntityTexture(plate, tBlock[4], 0, 2);
            if (tCell[c] > 0) EntityTexture(plate, tCell[c], 0, 3);

            EntityType(FindChild(world, "Door"  + Dig(c, 10)), 11, false);
            EntityType(FindChild(world, "Plate" + Dig(c, 10)), 11, false);
            EntityType(FindChild(world, "Bars"  + Dig(c, 10)), 11, false);
            cellLocked[loc][c] = 0;
        }
        no_chairs = 0; no_beds = 20; no_doors = 1;
    } else if (loc == 2) {
        // Yard
        world = LoadAnimMesh(BASE + "World/Yard/Yard.3ds", 0);
        EntityType(world, 10, true);
        if (tNet > 0)   EntityTexture(FindChild(world, "Net"),  tNet);
        if (tFence > 0) {
            for (int c = 1; c <= 2; ++c)
                EntityTexture(FindChild(world, "Fence0" + Str(c)), tFence);
        }
        no_chairs = 6; no_beds = 0; no_doors = 1;
    } else if (loc == 4) {
        // Study
        world = LoadAnimMesh(BASE + "World/Study/Study.3ds", 0);
        EntityType(world, 10, true);
        EntityFX(FindChild(world, "Screen"), 9);
        no_chairs = 5; no_beds = 0; no_doors = 2;
    } else if (loc == 6) {
        // Hospital
        world = LoadAnimMesh(BASE + "World/Hospital/Hospital.3ds", 0);
        EntityType(world, 10, true);
        EntityFX(FindChild(world, "Screen"), 9);
        for (int c = 1; c <= 4; ++c) {
            EntityAlpha(FindChild(world, "Beaker0" + Str(c)), 0.7f);
            EntityAlpha(FindChild(world, "Water0"  + Str(c)), 0.5f);
        }
        no_chairs = 9; no_beds = 3; no_doors = 2;
    } else if (loc == 8) {
        // Canteen / Kitchen
        world = LoadAnimMesh(BASE + "World/Kitchen/Kitchen.3ds", 0);
        EntityType(world, 10, true);
        no_chairs = 20; no_beds = 0; no_doors = 2;
        sAtmos = LoadSound(BASE + "Sound/Ambience/Hall.wav");
    } else if (loc == 9) {
        // Main Hall
        world = LoadAnimMesh(BASE + "World/Hall/Hall.3ds", 0);
        EntityType(world, 10, true);
        no_chairs = 8; no_beds = 0; no_doors = 3;
        sAtmos = LoadSound(BASE + "Sound/Ambience/Hall.wav");
    } else if (loc == 10) {
        // Workshop
        world = LoadAnimMesh(BASE + "World/Workshop/Workshop.3ds", 0);
        EntityType(world, 10, true);
        no_chairs = 12; no_beds = 0; no_doors = 2;
        sAtmos = LoadSound(BASE + "Sound/Ambience/Quiet.wav");
    } else if (loc == 11) {
        // Bathroom / Toilet
        world = LoadAnimMesh(BASE + "World/Toilet/Toilet.3ds", 0);
        EntityType(world, 10, true);
        no_chairs = 0; no_beds = 0; no_doors = 1;
        sAtmos = LoadSound(BASE + "Sound/Ambience/Quiet.wav");
    }
    // TODO: remaining locations (Kitchen, Hall, Workshop, Toilet, City, etc.)
}

// ---------------------------------------------------------------------------
//  SetCollisions  (from World.bb)
// ---------------------------------------------------------------------------
void SetCollisions() {
    // entity type IDs mirror Blitz3D convention
    for (int t = 10; t <= 11; ++t) {
        Collisions(1, t, 2, 3);   // humans  → scenery
        Collisions(2, t, 2, 3);   // shadows → scenery
        Collisions(3, t, 2, 1);   // weapon models → scenery
        Collisions(4, t, 2, 1);   // weapon pivots → scenery
    }
    Collisions(5, 10, 2, 3);      // camera → scenery
}

// ---------------------------------------------------------------------------
//  Extra global definitions
// ---------------------------------------------------------------------------
float lightR=100,lightG=100,lightB=100;
float ambR=100,ambG=100,ambB=100;
float skyR=255,skyG=255,skyB=255;
Handle fader=0;
float  fadeAlpha=0.0f, fadeTarget=0.0f;

std::array<Handle, NO_BULLETS+1> bullet      {};
std::array<float,  NO_BULLETS+1> bulletX     {};
std::array<float,  NO_BULLETS+1> bulletY     {};
std::array<float,  NO_BULLETS+1> bulletZ     {};
std::array<float,  NO_BULLETS+1> bulletXA    {};
std::array<float,  NO_BULLETS+1> bulletYA    {};
std::array<float,  NO_BULLETS+1> bulletZA    {};
std::array<int,    NO_BULLETS+1> bulletState {};
std::array<int,    NO_BULLETS+1> bulletTim   {};
std::array<int,    NO_BULLETS+1> bulletShooter{};

int wScreen=0, wOldScreen=0;
std::array<int, 41> limbPrecede{}, limbSource{};
GLuint gHealth=0, gHappiness=0, gMoney=0, gPhoto=0, gMap=0, gMarker=0;
int no_hairs=9, no_faces=66, no_bodies=10, no_arms=12, no_legs=6;
