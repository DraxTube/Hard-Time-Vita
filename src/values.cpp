// =============================================================================
//  values.cpp  –  Global variable definitions  (converted from Values.bb)
// =============================================================================
#include "values.h"
#include "blitz_compat.h"
#include "texts.h"
#include "functions.h"
#include "players.h"
#include "debug_log.h"

// ---------------------------------------------------------------------------
//  Scalars
// ---------------------------------------------------------------------------
int version    = 4;
int screen     = 0, oldScreen = 0;
int screenSource = 0, screenAgenda = 0;
int screenCall = 0, callX = 0, callY = 0;
int go = 0, gotim = 0;
int foc = 0, subfoc = 0;
int timer = 0, keytim = 0;
int tester = 0;

// Progress
int slot = 1, oldLocation = 0;
int gamPoints = 0, gamPointLimit = 0;
int gamPause = 0, gamFile = 0, gamDoor = 0;
int gamEnded = 0;

std::array<std::string, MAX_SLOTS+1> gamName   {};
std::array<int, MAX_SLOTS+1> gamPhoto    {};
std::array<int, MAX_SLOTS+1> gamChar     {};
std::array<int, MAX_SLOTS+1> gamPlayer   {};
std::array<int, MAX_SLOTS+1> gamLocation {};
std::array<int, MAX_SLOTS+1> gamMoney    {};
std::array<int, MAX_SLOTS+1> gamSpeed    {};
std::array<int, MAX_SLOTS+1> gamSecs     {};
std::array<int, MAX_SLOTS+1> gamMins     {};
std::array<int, MAX_SLOTS+1> gamHours    {};
std::array<int, MAX_SLOTS+1> gamWarrant  {};
std::array<int, MAX_SLOTS+1> gamVictim   {};
std::array<int, MAX_SLOTS+1> gamItem     {};
std::array<int, MAX_SLOTS+1> gamArrival  {};
std::array<int, MAX_SLOTS+1> gamFatality {};
std::array<int, MAX_SLOTS+1> gamRelease  {};
std::array<int, MAX_SLOTS+1> gamEscape   {};
std::array<int, MAX_SLOTS+1> gamGrowth   {};
std::array<int, MAX_SLOTS+1> gamBlackout {};
std::array<int, MAX_SLOTS+1> gamBombThreat{};
std::array<int, MAX_SLOTS+1> gamMission  {};
std::array<int, MAX_SLOTS+1> gamClient   {};
std::array<int, MAX_SLOTS+1> gamTarget   {};
std::array<int, MAX_SLOTS+1> gamDeadline {};
std::array<int, MAX_SLOTS+1> gamReward   {};
std::array<int, 11> statTim             {};

// Promos
int gamPromo = 0;
int promoTim = 0, promoStage = 0;
int promoEffect = 0, promoVariable = 0;
int promoAccuser = 0, promoVerdict = 0, promoCash = 0;
std::string optionA, optionB;
std::array<int, 3>  promoActor  {};
std::array<int, 11> promoReact  {};
std::array<int, NO_PROMOS+1> promoUsed {};

// Phones
int phoneRing = 0, phoneTim = 0, phonePromo = 0;
std::array<float, 5> phoneX{}, phoneY{}, phoneZ{};

// Outro
std::array<int, 11> endChar{}, endFate{};

// ---------------------------------------------------------------------------
//  Options
// ---------------------------------------------------------------------------
int optPopulation = 60;
int optRes        = 0;   // Vita: always 0 (960×544 native)
int optFog        = 1;
int optFX         = 1;
int optShadows    = 2;
int optGore       = 3;

// Vita button mapping
int keyAttack  = SCE_CTRL_CROSS;
int keyDefend  = SCE_CTRL_CIRCLE;
int keyThrow   = SCE_CTRL_SQUARE;
int keyPickUp  = SCE_CTRL_TRIANGLE;
int buttAttack = SCE_CTRL_CROSS;
int buttDefend = SCE_CTRL_CIRCLE;
int buttThrow  = SCE_CTRL_SQUARE;
int buttPickUp = SCE_CTRL_TRIANGLE;

// ---------------------------------------------------------------------------
//  Sound handles
// ---------------------------------------------------------------------------
// Music & atmosphere
BBSound* sTheme      = nullptr;
int      chTheme     = -1;
float    musicVol    = 1.0f;
int      chAtmos     = -1;
BBSound* sAtmos      = nullptr;
int      chPhone = -1, chAlarm = -1;
int      chDeath = -1;

// Menu sfx
BBSound* sMenuBrowse = nullptr;
BBSound* sMenuSelect = nullptr;
BBSound* sMenuGo     = nullptr;
BBSound* sMenuBack   = nullptr;
BBSound* sVoid       = nullptr;
BBSound* sTrash      = nullptr;
BBSound* sCamera     = nullptr;
BBSound* sComputer   = nullptr;
BBSound* sCash       = nullptr;
BBSound* sPaper      = nullptr;
BBSound* sMurmur     = nullptr;
std::array<BBSound*, 3> sJury  {};
std::array<BBSound*, 4> sDoor  {};
BBSound* sBuzzer     = nullptr;
BBSound* sBell       = nullptr;
BBSound* sRing       = nullptr;
BBSound* sAlarm      = nullptr;
BBSound* sTanoy      = nullptr;
BBSound* sBasket     = nullptr;

// Movement
BBSound* sFall       = nullptr;
BBSound* sThud       = nullptr;
BBSound* sBreakdown  = nullptr;

// Pain
BBSound* sDeath      = nullptr;
BBSound* sChoke      = nullptr;
BBSound* sSnore      = nullptr;

// Impact / prop
BBSound* sBleed      = nullptr;
BBSound* sStab       = nullptr;
BBSound* sEat        = nullptr;
BBSound* sDrink      = nullptr;

// Weapon sounds
BBSound* sGeneric    = nullptr;
BBSound* sBlade      = nullptr;
BBSound* sMetal      = nullptr;
BBSound* sWood       = nullptr;
BBSound* sCane       = nullptr;
BBSound* sString     = nullptr;
BBSound* sRock       = nullptr;
BBSound* sAxe        = nullptr;
BBSound* sBall       = nullptr;
BBSound* sPhone      = nullptr;
BBSound* sCigar      = nullptr;
BBSound* sSyringe    = nullptr;
BBSound* sBottle     = nullptr;

// Technology
BBSound* sReload     = nullptr;
BBSound* sGun        = nullptr;
BBSound* sMine       = nullptr;
BBSound* sLaser      = nullptr;

// Weapon textures (defined in world_props.cpp)
// GLuint tMachine, tPistol;

// ---------------------------------------------------------------------------
//  Characters
// ---------------------------------------------------------------------------
int no_chars = 0, no_plays = 0, no_weaps = 0;
int no_costumes = 8, no_models = 5;
int no_hairstyles = 31, no_specs = 4;

std::array<int, MAX_CHARS> charStatus   {};
std::array<int, MAX_CHARS> charPromoRef {};

// Render handles
int cam    = 0;
int light1 = 0;

// ---------------------------------------------------------------------------
//  World
// ---------------------------------------------------------------------------
// trayState, trayOldState, tScreen, tTray, tEyes defined in world_props.cpp

// Camera
int   camType = 0, camTim = 0;
int   camRectify = 0;
float camTX = 0, camTY = 0, camTZ = 0;
float camPivX = 0, camPivY = 100, camPivZ = 0;
// camX, camY, camZ defined in moves.cpp
float camPivTX = 0, camPivTY = 0, camPivTZ = 0;
float speedX = 0, speedY = 0, speedZ = 0;
std::array<int, 11> camShortcut {};

// Lighting
int   no_lights = 0;
float lightTR = 0, lightTG = 0, lightTB = 0;
float ambTR = 0, ambTG = 0, ambTB = 0;
float atmosR = 100, atmosG = 100, atmosB = 100;
float atmosTR = 0, atmosTG = 0, atmosTB = 0;
float skyTR = 0, skyTG = 0, skyTB = 0;

// Doors
// Door, cell, and kit arrays defined in world_props.cpp

// ---------------------------------------------------------------------------
//  LoadSounds  –  loaded from ux0:data/HardTime/Sound/…
// ---------------------------------------------------------------------------
static void LoadSounds() {
    DebugLog("[SND] LoadSounds() start");
    const std::string S = "ux0:data/HardTime/Sound/";

    // Menu sounds
    sMenuBrowse = LoadSound(S + "Browse.wav");
    sMenuSelect = LoadSound(S + "Select.wav");
    sMenuGo     = LoadSound(S + "Confirm.wav");
    sMenuBack   = LoadSound(S + "Cancel.wav");
    sVoid       = LoadSound(S + "Void.wav");
    sTrash      = LoadSound(S + "Trash.wav");
    sCamera     = LoadSound(S + "Camera.wav");
    sComputer   = LoadSound(S + "Computer.wav");
    sCash       = LoadSound(S + "Cash.wav");
    sPaper      = LoadSound(S + "Paper.wav");
    DebugLog("[SND] Menu sounds loaded");

    // Court reactions
    sMurmur     = LoadSound(S + "Murmur.wav");
    sJury[1]    = LoadSound(S + "Cheer.wav");
    sJury[2]    = LoadSound(S + "Jeer.wav");

    // World sounds
    const std::string W = S + "World/";
    for (int i = 1; i <= 3; ++i)
        sDoor[i] = Load3DSound(W + "Door0" + std::to_string(i) + ".wav");
    sBuzzer = Load3DSound(W + "Buzzer.wav");
    sBell   = Load3DSound(W + "Bell.wav");
    sRing   = Load3DSound(W + "Ring.wav");
    sAlarm  = Load3DSound(W + "Alarm.wav");
    sTanoy  = Load3DSound(W + "Tanoy.wav");
    sBasket = Load3DSound(W + "Basket.wav");

    // Movement sounds
    const std::string M = S + "Movement/";
    sFall  = Load3DSound(M + "Fall.wav");
    sThud  = Load3DSound(M + "Thud.wav");
    // sSwing loaded in particles.cpp LoadParticles()
    // sStep loaded in moves.cpp LoadMoveSounds()

    // Pain sounds
    const std::string P = S + "Pain/";
    sDeath     = Load3DSound(P + "Death.wav");
    sChoke     = Load3DSound(P + "Choke.wav");
    sSnore     = Load3DSound(P + "Snoring.wav");
    sBreakdown = Load3DSound(P + "Breakdown.wav");
    // sPain loaded in particles.cpp LoadParticles()
    // sAgony loaded in moves.cpp LoadMoveSounds()

    // Impact / prop sounds
    const std::string PR = S + "Props/";
    sBleed   = Load3DSound(PR + "Bleed.wav");
    sStab    = Load3DSound(PR + "Stab.wav");
    sEat     = Load3DSound(PR + "Eat.wav");
    sDrink   = Load3DSound(PR + "Drink.wav");

    // Weapon sounds
    sGeneric = Load3DSound(PR + "Generic.wav");
    sBlade   = Load3DSound(PR + "Blade.wav");
    sMetal   = Load3DSound(PR + "Metal.wav");
    sWood    = Load3DSound(PR + "Wood.wav");
    sCane    = Load3DSound(PR + "Cane.wav");
    sString  = Load3DSound(PR + "String.wav");
    sRock    = Load3DSound(PR + "Rock.wav");
    sAxe     = Load3DSound(PR + "Axe.wav");
    sBall    = Load3DSound(PR + "Ball.wav");
    sPhone   = Load3DSound(PR + "Phone.wav");
    sCigar   = Load3DSound(PR + "Cigar.wav");
    sSyringe = Load3DSound(PR + "Syringe.wav");
    sBottle  = Load3DSound(PR + "Bottle.wav");
    // sSplash loaded in particles.cpp
    // sExplosion loaded in particles.cpp
    // sBlaze loaded in particles.cpp

    // Technology sounds
    sReload  = Load3DSound(PR + "Reload.wav");
    sGun     = Load3DSound(PR + "Gun.wav");
    sMine    = Load3DSound(PR + "Mine.wav");
    sLaser   = Load3DSound(PR + "Laser.wav");
    // sShot loaded in moves.cpp LoadMoveSounds()
    // sRicochet loaded in moves.cpp LoadMoveSounds()
    DebugLog("[SND] LoadSounds() done");
}

// ---------------------------------------------------------------------------
//  InitDoorData  –  from Values.bb .Doors section  (lines 940-1029)
// ---------------------------------------------------------------------------
void InitDoorData() {
    // Blocks: all 4 cell blocks share same door layout
    for (int b = 1; b <= 8; ++b) {
        if (b == 1 || b == 3 || b == 5 || b == 7) {
            int d = 1;
            doorA[b][d] = 180.0f;
            doorX1[b][d] = -25.0f;  doorX2[b][d] = 25.0f;
            doorY1[b][d] = 0.0f;    doorY2[b][d] = 60.0f;
            doorZ1[b][d] = -350.0f; doorZ2[b][d] = -330.0f;
        }
        if (b == 2 || b == 4 || b == 6 || b == 8) {
            int d = 1;
            doorA[b][d] = 180.0f;
            doorX1[b][d] = 65.0f;   doorX2[b][d] = 90.0f;
            doorY1[b][d] = 0.0f;    doorY2[b][d] = 60.0f;
            doorZ1[b][d] = 190.0f;  doorZ2[b][d] = 220.0f;
        }
    }
    // Study (loc 4)
    { int b=4, d=1;
      doorA[b][d]=180; doorX1[b][d]=-10; doorX2[b][d]=15;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=-160; doorZ2[b][d]=-130; }
    { int b=4, d=2;
      doorA[b][d]=270; doorX1[b][d]=135; doorX2[b][d]=165;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=-12;  doorZ2[b][d]=12; }
    // Hospital (loc 6)
    { int b=6, d=1;
      doorA[b][d]=180; doorX1[b][d]=-12; doorX2[b][d]=12;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=-160; doorZ2[b][d]=-130; }
    { int b=6, d=2;
      doorA[b][d]=0;   doorX1[b][d]=-12; doorX2[b][d]=12;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=135;  doorZ2[b][d]=165; }
    // Kitchen (loc 8)
    { int b=8, d=1;
      doorA[b][d]=180; doorX1[b][d]=-12; doorX2[b][d]=12;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=-360; doorZ2[b][d]=-330; }
    // Hall (loc 9) – 8 doors
    { int b=9;
      int d=1; doorA[b][d]=0;   doorX1[b][d]=-175; doorX2[b][d]=-120;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=285; doorZ2[b][d]=310;
      d=2; doorA[b][d]=0;   doorX1[b][d]=140;  doorX2[b][d]=165;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=285; doorZ2[b][d]=310;
      d=3; doorA[b][d]=270; doorX1[b][d]=285;  doorX2[b][d]=310;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=120; doorZ2[b][d]=180;
      d=4; doorA[b][d]=270; doorX1[b][d]=285;  doorX2[b][d]=310;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=-160; doorZ2[b][d]=-135;
      d=5; doorA[b][d]=180; doorX1[b][d]=125;  doorX2[b][d]=185;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=-310; doorZ2[b][d]=-285;
      d=6; doorA[b][d]=180; doorX1[b][d]=-165; doorX2[b][d]=-135;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=-310; doorZ2[b][d]=-285;
      d=7; doorA[b][d]=90;  doorX1[b][d]=-310; doorX2[b][d]=-285;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=-175; doorZ2[b][d]=-115;
      d=8; doorA[b][d]=90;  doorX1[b][d]=-310; doorX2[b][d]=-285;
      doorY1[b][d]=0; doorY2[b][d]=60; doorZ1[b][d]=140;  doorZ2[b][d]=165;
    }
    // Workshop (loc 10)
    { int b=10, d=1;
      doorA[b][d]=180; doorX1[b][d]=-12; doorX2[b][d]=12;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=-135; doorZ2[b][d]=-105; }
    // Toilets (loc 11)
    { int b=11, d=1;
      doorA[b][d]=180; doorX1[b][d]=78;  doorX2[b][d]=101;
      doorY1[b][d]=0;  doorY2[b][d]=60;  doorZ1[b][d]=-85;  doorZ2[b][d]=-55; }
}

// ---------------------------------------------------------------------------
//  InitCellData  –  from Values.bb .Cells section  (lines 1034-1104)
// ---------------------------------------------------------------------------
void InitCellData() {
    // Lower left side (1-4)
    cellX1[1]=-300; cellX2[1]=-205; cellY1[1]=0; cellY2[1]=85;
    cellZ1[1]=-140; cellZ2[1]=-55;  cellDoorX[1]=-195; cellDoorZ[1]=-95;

    cellX1[2]=-300; cellX2[2]=-205; cellY1[2]=0; cellY2[2]=85;
    cellZ1[2]=-35;  cellZ2[2]=48;   cellDoorX[2]=-195; cellDoorZ[2]=8;

    cellX1[3]=-300; cellX2[3]=-205; cellY1[3]=0; cellY2[3]=85;
    cellZ1[3]=70;   cellZ2[3]=150;  cellDoorX[3]=-195; cellDoorZ[3]=112;

    cellX1[4]=-300; cellX2[4]=-205; cellY1[4]=0; cellY2[4]=85;
    cellZ1[4]=170;  cellZ2[4]=255;  cellDoorX[4]=-195; cellDoorZ[4]=215;

    // Lower top side (5-6)
    cellX1[5]=-195; cellX2[5]=-110; cellY1[5]=0; cellY2[5]=85;
    cellZ1[5]=265;  cellZ2[5]=355;  cellDoorX[5]=-152; cellDoorZ[5]=248;

    cellX1[6]=113;  cellX2[6]=200;  cellY1[6]=0; cellY2[6]=85;
    cellZ1[6]=265;  cellZ2[6]=355;  cellDoorX[6]=160;  cellDoorZ[6]=248;

    // Lower right side (7-10)
    cellX1[7]=208; cellX2[7]=298; cellY1[7]=0; cellY2[7]=85;
    cellZ1[7]=172; cellZ2[7]=255; cellDoorX[7]=195;  cellDoorZ[7]=211;

    cellX1[8]=208; cellX2[8]=298; cellY1[8]=0; cellY2[8]=85;
    cellZ1[8]=68;  cellZ2[8]=150; cellDoorX[8]=195;  cellDoorZ[8]=107;

    cellX1[9]=208; cellX2[9]=298; cellY1[9]=0; cellY2[9]=85;
    cellZ1[9]=-35; cellZ2[9]=48;  cellDoorX[9]=195;  cellDoorZ[9]=3;

    cellX1[10]=208; cellX2[10]=298; cellY1[10]=0; cellY2[10]=85;
    cellZ1[10]=-140; cellZ2[10]=-55; cellDoorX[10]=195; cellDoorZ[10]=-100;

    // Upper translations (11-20 = mirror of 1-10 but on upper floor)
    for (int n = 11; n <= 20; ++n) {
        cellX1[n] = cellX1[n-10]; cellX2[n] = cellX2[n-10];
        cellY1[n] = 100;          cellY2[n] = 200;
        cellZ1[n] = cellZ1[n-10]; cellZ2[n] = cellZ2[n-10];
        cellDoorX[n] = cellDoorX[n-10];
        cellDoorZ[n] = cellDoorZ[n-10];
    }
}

// ---------------------------------------------------------------------------
//  InitLimbHierarchy  –  from Values.bb .Limbs section  (lines 308-344)
// ---------------------------------------------------------------------------
void InitLimbHierarchy() {
    // Left arm
    limbPrecede[4]=5;  limbSource[4]=3;   // left bicep
    limbPrecede[5]=6;  limbSource[5]=4;   // left arm
    limbPrecede[6]=0;  limbSource[6]=5;   // left hand
    limbPrecede[7]=8;  limbSource[7]=6;   // left thumb01
    limbPrecede[8]=0;  limbSource[8]=7;   // left thumb02
    limbPrecede[9]=10; limbSource[9]=6;   // left finger01
    limbPrecede[10]=0; limbSource[10]=9;  // left finger02
    limbPrecede[11]=12;limbSource[11]=6;  // left finger03
    limbPrecede[12]=0; limbSource[12]=11; // left finger04
    limbPrecede[13]=14;limbSource[13]=6;  // left finger05
    limbPrecede[14]=0; limbSource[14]=13; // left finger06
    limbPrecede[15]=16;limbSource[15]=6;  // left finger07
    limbPrecede[16]=0; limbSource[16]=15; // left finger08

    // Right arm
    limbPrecede[17]=18;limbSource[17]=3;  // right bicep
    limbPrecede[18]=19;limbSource[18]=17; // right arm
    limbPrecede[19]=0; limbSource[19]=18; // right hand
    limbPrecede[20]=21;limbSource[20]=19; // right thumb01
    limbPrecede[21]=0; limbSource[21]=20; // right thumb02
    limbPrecede[22]=23;limbSource[22]=19; // right finger01
    limbPrecede[23]=0; limbSource[23]=22; // right finger02
    limbPrecede[24]=25;limbSource[24]=19; // right finger03
    limbPrecede[25]=0; limbSource[25]=24; // right finger04
    limbPrecede[26]=27;limbSource[26]=19; // right finger05
    limbPrecede[27]=0; limbSource[27]=26; // right finger06
    limbPrecede[28]=29;limbSource[28]=19; // right finger07
    limbPrecede[29]=0; limbSource[29]=28; // right finger08

    // Legs
    limbPrecede[31]=32;limbSource[31]=30; // left thigh
    limbPrecede[32]=33;limbSource[32]=31; // left leg
    limbPrecede[33]=0; limbSource[33]=32; // left foot
    limbPrecede[34]=35;limbSource[34]=30; // right thigh
    limbPrecede[35]=36;limbSource[35]=34; // right leg
    limbPrecede[36]=0; limbSource[36]=35; // right foot

    // Additional
    limbPrecede[37]=0; limbSource[37]=1;  // left ear
    limbPrecede[38]=0; limbSource[38]=1;  // right ear
}

// ---------------------------------------------------------------------------
//  InitValues  –  replaces top-level code in Values.bb
// ---------------------------------------------------------------------------
void InitValues() {
    DebugLog("[INIT] InitValues() start");
    // Seed RNG
    SeedRnd(MilliSecs());

    // Populate all text strings
    DebugLog("[INIT] InitTexts()...");
    InitTexts();
    DebugLog("[INIT] InitTexts() done");

    // Initialise camera shortcuts
    for (int i = 1; i <= 9; ++i) camShortcut[i] = i;
    camShortcut[10] = 0;

    // Load sound assets
    DebugLog("[INIT] LoadSounds()...");
    LoadSounds();
    DebugLog("[INIT] LoadSounds() done");

    // Initialise door & cell data
    DebugLog("[INIT] InitDoorData()...");
    InitDoorData();
    DebugLog("[INIT] InitDoorData() done");
    DebugLog("[INIT] InitCellData()...");
    InitCellData();
    DebugLog("[INIT] InitCellData() done");
    DebugLog("[INIT] InitLimbHierarchy()...");
    InitLimbHierarchy();
    DebugLog("[INIT] InitLimbHierarchy() done");
    DebugLog("[INIT] InitValues() complete");
}
