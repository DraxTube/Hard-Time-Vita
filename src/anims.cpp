// =============================================================================
//  anims.cpp  –  Animation state machine  (ported from Anims.bb + Moves.bb)
//
//  Blitz3D anim system:
//    LoadAnimSeq(mesh, file)  → loads an extra animation file into the mesh
//    ExtractAnimSeq(mesh, from, to, srcSeq)  → extracts a sub-clip
//    Animate(mesh, mode, speed, seq, transition)
//       mode 1=loop, 3=once
//
//  PS Vita mapping:
//    We keep integer sequence IDs in pSeq[cyc][id].
//    The vitaGL/assimp backend plays them by clip index.
//    LoadAnimSeq / ExtractAnimSeq are thin wrappers until the
//    assimp-based skeleton system is fully wired up.
// =============================================================================
#include "anims.h"
#include "players.h"
#include "values.h"
#include "particles.h"
#include "functions.h"
#include "costume.h"
#include "crimes.h"
#include "moves.h"
#include "render3d.h"
#include "blitz_compat.h"
#include "world_props.h"

// ---------------------------------------------------------------------------
//  Stub functions (not yet fully ported from Blitz3D)
// ---------------------------------------------------------------------------
static void WeaponImpact(int /*attacker*/, int /*victim*/, int /*blocked*/) {
    // TODO: port weapon impact effects from Blitz3D
}
static int LimbProximity(Handle limb, float x, float z, float range) {
    if (limb == 0) return 0;
    float dx = EntityX(limb, 1) - x;
    float dz = EntityZ(limb, 1) - z;
    return (dx*dx + dz*dz < range*range) ? 1 : 0;
}
static void ScarWeapon(int /*weaponIndex*/, int /*severity*/) {
    // TODO: port weapon scarring from Blitz3D
}

// ---------------------------------------------------------------------------
//  Extra per-player state arrays
// ---------------------------------------------------------------------------
std::array<int,   51> pDazed    {};
std::array<int,   51> pFoc      {};
std::array<int,   51> pPromoState{};
// pMultiSting defined in moves.cpp
std::array<int,   51> pSpeaking {};
std::array<int,   51> pStepTim  {};
std::array<float, 51> pOldMoveX {};
std::array<float, 51> pOldMoveZ {};
std::array<int,   51> pDT       {};
std::array<Handle, 11> light    {};

// ---------------------------------------------------------------------------
//  LoadAnimSeq / ExtractAnimSeq stubs
//  In a full assimp port these load/slice .3ds animation clips.
// ---------------------------------------------------------------------------
static int s_nextSeqId = 1000;   // IDs above the slot table

int LoadAnimSeq(Handle /*mesh*/, const BBString& /*path*/) {
    // TODO: load .3ds animation clip and register with skeleton system
    return s_nextSeqId++;
}

int ExtractAnimSeq(Handle /*mesh*/, int startFrame, int endFrame, int srcSeq) {
    // TODO: slice srcSeq[startFrame..endFrame] and register as new clip
    (void)startFrame; (void)endFrame; (void)srcSeq;
    return s_nextSeqId++;
}

// ---------------------------------------------------------------------------
//  LoadAnimSequences  (from Anims.bb LoadSequences)
// ---------------------------------------------------------------------------
void LoadAnimSequences(int cyc) {
    const std::string BASE = "ux0:data/HardTime/Characters/Sequences/";
    // Load 4 source animation files
    pSeq[cyc][601] = LoadAnimSeq(p[cyc], BASE + "Standard01.3ds");
    pSeq[cyc][602] = LoadAnimSeq(p[cyc], BASE + "Standard02.3ds");
    pSeq[cyc][603] = LoadAnimSeq(p[cyc], BASE + "Standard03.3ds");
    pSeq[cyc][604] = LoadAnimSeq(p[cyc], BASE + "Standard04.3ds");
    pSeq[cyc][610] = LoadAnimSeq(p[cyc], BASE + "Move_Execute.3ds");
    pSeq[cyc][611] = LoadAnimSeq(p[cyc], BASE + "Move_Receive.3ds");

    // Stances  0-10
    pSeq[cyc][1]  = ExtractAnimSeq(p[cyc],0,40,    pSeq[cyc][601]); // standing
    pSeq[cyc][2]  = ExtractAnimSeq(p[cyc],1145,1185,pSeq[cyc][601]); // kneeling
    pSeq[cyc][3]  = ExtractAnimSeq(p[cyc],925,965,  pSeq[cyc][602]); // injured
    pSeq[cyc][4]  = ExtractAnimSeq(p[cyc],1065,1105,pSeq[cyc][602]); // dazed
    pSeq[cyc][5]  = ExtractAnimSeq(p[cyc],1980,2020,pSeq[cyc][603]); // ball stance
    // Movement 10-20
    pSeq[cyc][10] = ExtractAnimSeq(p[cyc],140,220,  pSeq[cyc][601]); // standing turn
    pSeq[cyc][11] = ExtractAnimSeq(p[cyc],1195,1275,pSeq[cyc][601]); // kneeling turn
    pSeq[cyc][12] = ExtractAnimSeq(p[cyc],915,995,  pSeq[cyc][603]); // walking
    pSeq[cyc][13] = ExtractAnimSeq(p[cyc],50,130,   pSeq[cyc][601]); // running
    pSeq[cyc][14] = ExtractAnimSeq(p[cyc],975,1055, pSeq[cyc][602]); // injured movement
    pSeq[cyc][15] = ExtractAnimSeq(p[cyc],1115,1195,pSeq[cyc][602]); // dazed movement
    pSeq[cyc][16] = ExtractAnimSeq(p[cyc],75,155,   pSeq[cyc][603]); // weapon movement
    pSeq[cyc][17] = ExtractAnimSeq(p[cyc],30,110,   pSeq[cyc][604]); // ball walk
    pSeq[cyc][18] = ExtractAnimSeq(p[cyc],120,200,  pSeq[cyc][604]); // ball run
    // Weapon interaction 20-30
    pSeq[cyc][20] = ExtractAnimSeq(p[cyc],1985,2045,pSeq[cyc][601]); // pick-up weapon
    pSeq[cyc][21] = ExtractAnimSeq(p[cyc],65,75,    pSeq[cyc][602]); // drop weapon
    pSeq[cyc][22] = ExtractAnimSeq(p[cyc],205,275,  pSeq[cyc][602]); // throw weapon
    pSeq[cyc][23] = ExtractAnimSeq(p[cyc],255,325,  pSeq[cyc][603]); // snatch weapon
    pSeq[cyc][24] = ExtractAnimSeq(p[cyc],165,245,  pSeq[cyc][603]); // examine weapon
    pSeq[cyc][25] = ExtractAnimSeq(p[cyc],1860,1900,pSeq[cyc][603]); // handover
    pSeq[cyc][26] = ExtractAnimSeq(p[cyc],1910,1970,pSeq[cyc][603]); // basketball throw
    pSeq[cyc][27] = ExtractAnimSeq(p[cyc],400,440,  pSeq[cyc][604]); // phone pick-up
    // H2H attacks 30-40
    pSeq[cyc][30] = ExtractAnimSeq(p[cyc],250,320,  pSeq[cyc][601]); // upper punch
    pSeq[cyc][31] = ExtractAnimSeq(p[cyc],1060,1135,pSeq[cyc][601]); // lower kick
    pSeq[cyc][32] = ExtractAnimSeq(p[cyc],1405,1455,pSeq[cyc][601]); // stomp
    pSeq[cyc][33] = ExtractAnimSeq(p[cyc],1675,1745,pSeq[cyc][601]); // big attack
    pSeq[cyc][34] = ExtractAnimSeq(p[cyc],1775,1875,pSeq[cyc][601]); // rear attack
    pSeq[cyc][35] = ExtractAnimSeq(p[cyc],105,175,  pSeq[cyc][602]); // rising attack
    // Weapon swings 40-50
    pSeq[cyc][40] = ExtractAnimSeq(p[cyc],305,375,  pSeq[cyc][602]); // upper swing
    pSeq[cyc][41] = ExtractAnimSeq(p[cyc],405,485,  pSeq[cyc][602]); // lower swing
    pSeq[cyc][42] = ExtractAnimSeq(p[cyc],745,815,  pSeq[cyc][602]); // ground swing
    pSeq[cyc][43] = ExtractAnimSeq(p[cyc],515,585,  pSeq[cyc][602]); // big swing
    pSeq[cyc][44] = ExtractAnimSeq(p[cyc],615,715,  pSeq[cyc][602]); // rear swing
    // Weapon stabs 50-60
    pSeq[cyc][51] = ExtractAnimSeq(p[cyc],335,425,  pSeq[cyc][603]); // quick stab
    pSeq[cyc][52] = ExtractAnimSeq(p[cyc],555,625,  pSeq[cyc][603]); // ground stab
    pSeq[cyc][53] = ExtractAnimSeq(p[cyc],455,525,  pSeq[cyc][603]); // big stab
    // Guns 60-70
    pSeq[cyc][60] = ExtractAnimSeq(p[cyc],1405,1445,pSeq[cyc][602]); // rifle stance
    pSeq[cyc][61] = ExtractAnimSeq(p[cyc],1005,1085,pSeq[cyc][603]); // rifle walk
    pSeq[cyc][62] = ExtractAnimSeq(p[cyc],1455,1535,pSeq[cyc][602]); // rifle running
    pSeq[cyc][63] = ExtractAnimSeq(p[cyc],1545,1585,pSeq[cyc][602]); // rifle fire
    pSeq[cyc][64] = ExtractAnimSeq(p[cyc],1735,1795,pSeq[cyc][602]); // pistol fire
    pSeq[cyc][65] = ExtractAnimSeq(p[cyc],1645,1725,pSeq[cyc][602]); // reload
    // Hurt & block 70-90
    pSeq[cyc][70] = ExtractAnimSeq(p[cyc],340,390,  pSeq[cyc][601]); // upper hurt
    pSeq[cyc][71] = ExtractAnimSeq(p[cyc],1320,1380,pSeq[cyc][601]); // lower hurt
    pSeq[cyc][72] = ExtractAnimSeq(p[cyc],1490,1550,pSeq[cyc][601]); // ground hurt back
    pSeq[cyc][73] = ExtractAnimSeq(p[cyc],1575,1645,pSeq[cyc][601]); // ground hurt front
    pSeq[cyc][74] = ExtractAnimSeq(p[cyc],1885,1925,pSeq[cyc][601]); // upper block
    pSeq[cyc][75] = ExtractAnimSeq(p[cyc],1935,1975,pSeq[cyc][601]); // lower block
    pSeq[cyc][76] = ExtractAnimSeq(p[cyc],825,865,  pSeq[cyc][602]); // upper weapon block
    pSeq[cyc][77] = ExtractAnimSeq(p[cyc],875,915,  pSeq[cyc][602]); // lower weapon block
    pSeq[cyc][78] = ExtractAnimSeq(p[cyc],450,595,  pSeq[cyc][604]); // die back
    pSeq[cyc][79] = ExtractAnimSeq(p[cyc],610,760,  pSeq[cyc][604]); // die front
    // Fall & rise 80-90
    pSeq[cyc][80] = ExtractAnimSeq(p[cyc],415,500,  pSeq[cyc][601]); // fall onto back
    pSeq[cyc][81] = ExtractAnimSeq(p[cyc],510,550,  pSeq[cyc][601]); // lying on back
    pSeq[cyc][82] = ExtractAnimSeq(p[cyc],560,670,  pSeq[cyc][601]); // get up off back
    pSeq[cyc][83] = ExtractAnimSeq(p[cyc],695,780,  pSeq[cyc][601]); // fall onto front (turn)
    pSeq[cyc][84] = ExtractAnimSeq(p[cyc],790,830,  pSeq[cyc][601]); // lying on front
    pSeq[cyc][85] = ExtractAnimSeq(p[cyc],840,910,  pSeq[cyc][601]); // get up off front
    pSeq[cyc][86] = ExtractAnimSeq(p[cyc],935,1030, pSeq[cyc][601]); // fall onto front (direct)
    pSeq[cyc][87] = ExtractAnimSeq(p[cyc],635,675,  pSeq[cyc][603]); // falling from height
    pSeq[cyc][88] = ExtractAnimSeq(p[cyc],685,745,  pSeq[cyc][603]); // landing from fall
    // Standing gestures 90-100
    pSeq[cyc][90] = ExtractAnimSeq(p[cyc],755,835,  pSeq[cyc][603]); // open door
    pSeq[cyc][91] = ExtractAnimSeq(p[cyc],1095,1135,pSeq[cyc][603]); // friendly wave
    pSeq[cyc][92] = ExtractAnimSeq(p[cyc],260,340,  pSeq[cyc][604]); // sweeping
    pSeq[cyc][93] = ExtractAnimSeq(p[cyc],950,990,  pSeq[cyc][604]); // smoking
    pSeq[cyc][94] = ExtractAnimSeq(p[cyc],1000,1040,pSeq[cyc][604]); // injecting
    pSeq[cyc][95] = ExtractAnimSeq(p[cyc],1050,1090,pSeq[cyc][604]); // drinking
    pSeq[cyc][96] = ExtractAnimSeq(p[cyc],1120,1370,pSeq[cyc][604]); // breakdown
    pSeq[cyc][97] = ExtractAnimSeq(p[cyc],1380,1440,pSeq[cyc][604]); // comb hair
    pSeq[cyc][98] = ExtractAnimSeq(p[cyc],1450,1510,pSeq[cyc][604]); // admire reflection
    // Seated gestures 100-120
    pSeq[cyc][100] = ExtractAnimSeq(p[cyc],1145,1155,pSeq[cyc][603]); // static
    pSeq[cyc][101] = ExtractAnimSeq(p[cyc],1385,1425,pSeq[cyc][603]); // slouching
    pSeq[cyc][102] = ExtractAnimSeq(p[cyc],1215,1255,pSeq[cyc][603]); // reading
    pSeq[cyc][103] = ExtractAnimSeq(p[cyc],1265,1375,pSeq[cyc][603]); // eating
    pSeq[cyc][104] = ExtractAnimSeq(p[cyc],1435,1475,pSeq[cyc][603]); // building
    pSeq[cyc][105] = ExtractAnimSeq(p[cyc],1485,1515,pSeq[cyc][603]); // lie down
    pSeq[cyc][106] = ExtractAnimSeq(p[cyc],1515,1555,pSeq[cyc][603]); // sleeping
    pSeq[cyc][107] = ExtractAnimSeq(p[cyc],1565,1620,pSeq[cyc][603]); // get off bed
    pSeq[cyc][108] = ExtractAnimSeq(p[cyc],1810,1850,pSeq[cyc][603]); // weight-lifting
    pSeq[cyc][109] = ExtractAnimSeq(p[cyc],210,250,  pSeq[cyc][604]); // typing
    // Speaking stances 120-130
    pSeq[cyc][120] = ExtractAnimSeq(p[cyc],350,390,  pSeq[cyc][604]); // holding phone
    pSeq[cyc][121] = ExtractAnimSeq(p[cyc],755,835,  pSeq[cyc][603]); // hand gestures
    pSeq[cyc][122] = ExtractAnimSeq(p[cyc],770,850,  pSeq[cyc][604]); // hands on hips
    pSeq[cyc][123] = ExtractAnimSeq(p[cyc],860,940,  pSeq[cyc][604]); // folded arms
    // Additional 130+
    pSeq[cyc][130] = ExtractAnimSeq(p[cyc],1520,1600,pSeq[cyc][604]); // body changed
    pSeq[cyc][131] = ExtractAnimSeq(p[cyc],1610,1650,pSeq[cyc][604]); // mourning
    pSeq[cyc][132] = ExtractAnimSeq(p[cyc],1660,1720,pSeq[cyc][604]); // dumbbell curl

    LoadMoveSequences(cyc);
}

// ---------------------------------------------------------------------------
//  LoadMoveSequences  (from Moves.bb)
// ---------------------------------------------------------------------------
void LoadMoveSequences(int cyc) {
    pSeq[cyc][200] = ExtractAnimSeq(p[cyc],1650,1720,pSeq[cyc][603]); // standing grapple lunge
    pSeq[cyc][201] = ExtractAnimSeq(p[cyc],1730,1800,pSeq[cyc][603]); // ground grapple lunge
    pSeq[cyc][202] = ExtractAnimSeq(p[cyc],60,100,   pSeq[cyc][610]); // apply headlock [execute]
    pSeq[cyc][203] = ExtractAnimSeq(p[cyc],60,100,   pSeq[cyc][611]); // apply headlock [receive]
    pSeq[cyc][204] = ExtractAnimSeq(p[cyc],320,400,  pSeq[cyc][610]); // pick up from floor [execute]
    pSeq[cyc][205] = ExtractAnimSeq(p[cyc],320,400,  pSeq[cyc][611]); // pick up from floor [receive]
    pSeq[cyc][206] = ExtractAnimSeq(p[cyc],110,150,  pSeq[cyc][610]); // hold headlock [execute]
    pSeq[cyc][207] = ExtractAnimSeq(p[cyc],110,150,  pSeq[cyc][611]); // hold headlock [receive]
    pSeq[cyc][208] = ExtractAnimSeq(p[cyc],250,310,  pSeq[cyc][610]); // headlock movement [execute]
    pSeq[cyc][209] = ExtractAnimSeq(p[cyc],250,310,  pSeq[cyc][611]); // headlock movement [receive]
    pSeq[cyc][210] = ExtractAnimSeq(p[cyc],160,240,  pSeq[cyc][610]); // release headlock [execute]
    pSeq[cyc][211] = ExtractAnimSeq(p[cyc],160,240,  pSeq[cyc][611]); // release headlock [receive]
    pSeq[cyc][212] = ExtractAnimSeq(p[cyc],410,450,  pSeq[cyc][610]); // headlock punch [execute]
    pSeq[cyc][213] = ExtractAnimSeq(p[cyc],410,450,  pSeq[cyc][611]); // headlock punch [receive]
    pSeq[cyc][214] = ExtractAnimSeq(p[cyc],460,500,  pSeq[cyc][610]); // knee to face [execute]
    pSeq[cyc][215] = ExtractAnimSeq(p[cyc],460,500,  pSeq[cyc][611]); // knee to face [receive]
    pSeq[cyc][216] = ExtractAnimSeq(p[cyc],510,650,  pSeq[cyc][610]); // headlock takedown [execute]
    pSeq[cyc][217] = ExtractAnimSeq(p[cyc],510,650,  pSeq[cyc][611]); // headlock takedown [receive]
    pSeq[cyc][218] = ExtractAnimSeq(p[cyc],660,840,  pSeq[cyc][610]); // bodyslam [execute]
    pSeq[cyc][219] = ExtractAnimSeq(p[cyc],660,840,  pSeq[cyc][611]); // bodyslam [receive]
    pSeq[cyc][220] = ExtractAnimSeq(p[cyc],850,1000, pSeq[cyc][610]); // chokeslam [execute]
    pSeq[cyc][221] = ExtractAnimSeq(p[cyc],850,1000, pSeq[cyc][611]); // chokeslam [receive]
    pSeq[cyc][222] = ExtractAnimSeq(p[cyc],1010,1185,pSeq[cyc][610]); // bulldog [execute]
    pSeq[cyc][223] = ExtractAnimSeq(p[cyc],1010,1185,pSeq[cyc][611]); // bulldog [receive]
    pSeq[cyc][224] = ExtractAnimSeq(p[cyc],1195,1350,pSeq[cyc][610]); // push off [execute]
    pSeq[cyc][225] = ExtractAnimSeq(p[cyc],1195,1350,pSeq[cyc][611]); // push off [receive]
    pSeq[cyc][226] = ExtractAnimSeq(p[cyc],1360,1520,pSeq[cyc][610]); // kick throw [execute]
    pSeq[cyc][227] = ExtractAnimSeq(p[cyc],1360,1520,pSeq[cyc][611]); // kick throw [receive]
}

// ---------------------------------------------------------------------------
//  Animation helpers
// ---------------------------------------------------------------------------
void ChangeAnim(int cyc, int anim) {
    pAnim[cyc]    = anim;
    pAnimTim[cyc] = 0;
}

void SharpTransition(int cyc, int anim, float angle) {
    Animate(p[cyc], 1, RndF(0.1f,0.3f), pSeq[cyc][anim], 0.0f);
    pState[cyc] = anim;
    if (angle >= 0) pA[cyc] = angle;
}

int DirPressed(int cyc) {
    return (cUp[cyc] || cDown[cyc] || cLeft[cyc] || cRight[cyc]) ? 1 : 0;
}
int HorizontalPressed(int cyc) { return (cLeft[cyc] || cRight[cyc]) ? 1 : 0; }
int VerticalPressed(int cyc)   { return (cUp[cyc]   || cDown[cyc])  ? 1 : 0; }
int ActionPressed(int cyc)     { return (cAttack[cyc] || cDefend[cyc] || cThrow[cyc] || cPickUp[cyc]) ? 1 : 0; }

void FaceEntity(int cyc, Handle target, float /*speed*/) {
    float dx = EntityX(target) - pX[cyc];
    float dz = EntityZ(target) - pZ[cyc];
    pA[cyc] = ATan2(dx, dz);
}

void FixGrapple(int cyc, int victim) {
    // Keep victim aligned at executor's position
    PositionEntity(p[victim], pX[cyc], pY[cyc], pZ[cyc]);
    RotateEntity(p[victim], 0.0f, pA[cyc]+180.0f, 0.0f);
}

void FindMoveCommands(int cyc) {
    // Translate attack/defend inputs to grapple move animations
    if (pGrappling[cyc] == 0) return;
    int v = pGrappling[cyc];
    if (cAttack[cyc] && pAnimTim[cyc] > 3) {
        int r = Rnd(0,3);
        if (r==0) ChangeAnim(cyc, 212);  // headlock punch
        if (r==1) ChangeAnim(cyc, 214);  // knee to face
        if (r==2) ChangeAnim(cyc, 216);  // takedown
        if (r==3) ChangeAnim(cyc, 218);  // bodyslam
    }
    if (cDefend[cyc] && pAnimTim[cyc] > 3)
        ChangeAnim(cyc, 210);  // release
    (void)v;
}

void FacialExpressions(int cyc) {
    if (p[cyc] == 0) return;
    // Eyes: 0=closed, 1=half, 2=open  — cycle blink every ~120 frames
    int targetEye = 2;
    if (pAnimTim[cyc] % 120 < 4)       targetEye = 0;  // blink closed
    else if (pAnimTim[cyc] % 120 < 8)  targetEye = 1;  // half-open
    // Dazed/injured = droopy eyes
    if (pDazed[cyc] > 0 || pHealth[cyc] < 20) targetEye = 1;
    if (pHealth[cyc] <= 0)                      targetEye = 0;

    if (targetEye == pEyes[cyc]) return;
    pOldEyes[cyc] = pEyes[cyc];
    pEyes[cyc]    = targetEye;

    // Apply eye texture to face limbs (limbs 1-2)
    GLuint eyeTex = (targetEye < (int)tEyes.size() && tEyes[targetEye]) ? tEyes[targetEye] : 0;
    for (int limb = 1; limb <= 2; ++limb) {
        if (pLimb[cyc][limb] && eyeTex)
            EntityTexture(pLimb[cyc][limb], eyeTex, 0, 1);
    }
}

// Forward declaration for CombatAnims (defined below Animations)
static void CombatAnims(int cyc);

// ---------------------------------------------------------------------------
//  Animations  –  main per-player update (from Anims.bb)
// ---------------------------------------------------------------------------
void Animations(int cyc) {
    // Only process grapple moves (200+) separately
    if (pAnim[cyc] >= 200) {
        MoveAnims(cyc);
        pAnimTim[cyc]++;
        return;
    }

    // Combat attacks (30-69)
    if (pAnim[cyc] >= 30 && pAnim[cyc] <= 69) {
        CombatAnims(cyc);
        pAnimTim[cyc]++;
        return;
    }

    int anim = 1;
    float speeder = RndF(0.1f, 0.3f);

    // ---- 0: Standing ----
    if (pAnim[cyc] == 0) {
        anim = 1;
        if (pDazed[cyc] > 0)          { anim = 4; speeder = RndF(0.3f, 0.6f); }
        if (pInjured[cyc] > 0 || pHealth[cyc] < 10) anim = 3;
        if (pPhone[cyc] > 0)           anim = 120;
        if (pWeapon[cyc] > 0 && weapStyle[weapType[pWeapon[cyc]]] == 4)
            anim = 60;
        if (pWeapon[cyc] > 0 && weapName[weapType[pWeapon[cyc]]] == "Ball")
            anim = 5;

        bool relaxed = (anim==1||anim==122||anim==123) &&
                       (pState[cyc]==1||pState[cyc]==122||pState[cyc]==123);
        if (pAnimTim[cyc] == 0 || (anim != pState[cyc] && !relaxed)) {
            Animate(p[cyc], 1, speeder, pSeq[cyc][anim], 10.0f);
            pState[cyc] = anim;
        }

        // Idle weapon actions
        if (gotim > 50 && pAnimTim[cyc] > 30 && pWeapon[cyc] > 0 && pPhone[cyc] == 0) {
            BBString wn = weapName[weapType[pWeapon[cyc]]];
            if (wn=="Broom")    { ChangeAnim(cyc,92); pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
            if (wn=="Cigarette" && charRole[pChar[cyc]]==0) { ChangeAnim(cyc,93); pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
            if (wn=="Syringe"   && charRole[pChar[cyc]]==0) { ChangeAnim(cyc,94); pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
            if (wn=="Bottle"    && charRole[pChar[cyc]]==0) { ChangeAnim(cyc,95); pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
            if (wn=="Comb")     { ChangeAnim(cyc,97); pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
            if (wn=="Mirror")   { ChangeAnim(cyc,98); pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
            if (wn=="Dumbbell") { ChangeAnim(cyc,132);pAgenda[cyc]=0; pTA[cyc]=pA[cyc]; }
        }
    }

    // ---- 1: Kneeling ----
    else if (pAnim[cyc] == 1) {
        anim = 2;
        if (pAnimTim[cyc] == 0 || anim != pState[cyc]) {
            Animate(p[cyc], 1, RndF(0.1f,0.3f), pSeq[cyc][anim], 10.0f);
            pState[cyc] = anim;
        }
        if (pAnimTim[cyc] > 5)
            if (DirPressed(cyc) || pDazed[cyc] > 0) ChangeAnim(cyc, 0);
    }

    // ---- 10: Standing turn ----
    else if (pAnim[cyc] == 10) {
        anim = 10;
        if (pWeapon[cyc] > 0 && weapStyle[weapType[pWeapon[cyc]]]==4) anim=61;
        if (pInjured[cyc]>0 || pHealth[cyc]<10) anim=14;
        if (pDazed[cyc]>0) anim=15;
        if (pAnimTim[cyc]==0 || anim!=pState[cyc]) {
            float spd = (Rnd(0,1)==0) ? -3.0f : 3.0f;
            Animate(p[cyc], 1, spd, pSeq[cyc][anim], 5.0f);
            pState[cyc] = anim;
        }
        if (pDazed[cyc]>0) {
            if (cLeft[cyc])  pA[cyc] -= 5.0f;
            if (cRight[cyc]) pA[cyc] += 5.0f;
        } else {
            if (cLeft[cyc])  pA[cyc] += 10.0f;
            if (cRight[cyc]) pA[cyc] -= 10.0f;
        }
        pA[cyc] = CleanAngle(pA[cyc]);
        if (pAnimTim[cyc] > 5)
            if (!HorizontalPressed(cyc) || VerticalPressed(cyc)) ChangeAnim(cyc, 0);
        pStepTim[cyc]++;
    }

    // ---- 11: Kneeling turn ----
    else if (pAnim[cyc] == 11) {
        anim = 11;
        if (pAnimTim[cyc]==0 || anim!=pState[cyc]) {
            float spd = (Rnd(0,1)==0) ? -3.0f : 3.0f;
            Animate(p[cyc], 1, spd, pSeq[cyc][anim], 5.0f);
            pState[cyc] = anim;
        }
        if (cLeft[cyc])  pA[cyc] += 5.0f;
        if (cRight[cyc]) pA[cyc] -= 5.0f;
        pA[cyc] = CleanAngle(pA[cyc]);
    }

    // ---- 12: Walking ----
    else if (pAnim[cyc] == 12) {
        anim = 12;
        if (pWeapon[cyc]>0) anim=16;
        if (pInjured[cyc]>0 || pHealth[cyc]<10) anim=14;
        if (pDazed[cyc]>0) anim=15;
        if (pAnimTim[cyc]==0 || anim!=pState[cyc]) {
            Animate(p[cyc], 1, RndF(1.0f,1.5f), pSeq[cyc][anim], 5.0f);
            pState[cyc] = anim;
        }
        if (cLeft[cyc])  pA[cyc] += (pDazed[cyc]>0 ? 2.0f : 5.0f);
        if (cRight[cyc]) pA[cyc] -= (pDazed[cyc]>0 ? 2.0f : 5.0f);
        pA[cyc] = CleanAngle(pA[cyc]);
        RotateEntity(pPivot[cyc], 0.0f, pA[cyc], 0.0f);
        MoveEntity(pPivot[cyc], 0.0f, 0.0f, -1.5f);
        pStepTim[cyc]++;
        if (pAnimTim[cyc] > 3 && !VerticalPressed(cyc) && !HorizontalPressed(cyc))
            ChangeAnim(cyc, 0);
        if (pAnimTim[cyc] > 3 && cUp[cyc] && !cDown[cyc])
            ChangeAnim(cyc, 13); // → run
    }

    // ---- 13: Running ----
    else if (pAnim[cyc] == 13) {
        anim = 13;
        if (pInjured[cyc]>0 || pHealth[cyc]<10) anim=14;
        if (pDazed[cyc]>0) anim=15;
        if (pAnimTim[cyc]==0 || anim!=pState[cyc]) {
            Animate(p[cyc], 1, RndF(2.5f,3.5f), pSeq[cyc][anim], 5.0f);
            pState[cyc] = anim;
        }
        if (cLeft[cyc])  pA[cyc] += 4.0f;
        if (cRight[cyc]) pA[cyc] -= 4.0f;
        pA[cyc] = CleanAngle(pA[cyc]);
        RotateEntity(pPivot[cyc], 0.0f, pA[cyc], 0.0f);
        MoveEntity(pPivot[cyc], 0.0f, 0.0f, -3.0f);
        pStepTim[cyc]++;
        if (!DirPressed(cyc)) ChangeAnim(cyc, 0);
    }

    // ---- Phone Interactions (28-29) ----
    else if (pAnim[cyc] >= 28 && pAnim[cyc] <= 29) {
        anim = pAnim[cyc];
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 3, 3.0f, pSeq[cyc][27], 5.0f);
        }
        if (pAnimTim[cyc] == 4) ProduceSound(p[cyc], sSwing, 22050, RndF(0.1f, 0.3f));
        
        if (anim == 28) {
            if (pAnimTim[cyc] == 8 && pScar[cyc][6] <= 4 && pPhone[cyc] == 0) {
                int v = PhoneProximity(cyc);
                if (v > 0 && PhoneTaken(v) == 0) {
                    ProduceSound(p[cyc], sPhone, 22050, 0.0f);
                    Handle phoneInWorld = FindChild(world, "Phone" + Dig(v, 10));
                    if (phoneInWorld) HideEntity(phoneInWorld);
                    Handle phoneInHand = FindChild(p[cyc], "Phone");
                    if (phoneInHand) ShowEntity(phoneInHand);
                    // Warning: phoneRing and phoneTim features from gameplay are ignored here as they are not defined
                    pPhone[cyc] = v;
                }
            }
            if (pAnimTim[cyc] > 8 && pPhone[cyc] > 0) {
                ChangeAnim(cyc, 0); pAgenda[cyc] = 0;
            }
            if (pAnimTim[cyc] > 15) ChangeAnim(cyc, 0);
        } else if (anim == 29) {
            if (pAnimTim[cyc] == 8 && pPhone[cyc] > 0) {
                ProduceSound(p[cyc], sPhone, 22050, 0.0f);
                Handle phoneInHand = FindChild(p[cyc], "Phone");
                if (phoneInHand) HideEntity(phoneInHand);
                Handle phoneInWorld = FindChild(world, "Phone" + Dig(pPhone[cyc], 10));
                if (phoneInWorld) ShowEntity(phoneInWorld);
                pPhone[cyc] = 0;
            }
            if (pAnimTim[cyc] > 15) ChangeAnim(cyc, 0);
        }
    }

    // ---- 70-79: Hurt reactions ----
    else if (pAnim[cyc] >= 70 && pAnim[cyc] <= 79) {
        anim = pAnim[cyc];
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 3, 3.0f, pSeq[cyc][anim], 0.0f);
            pState[cyc] = anim;
        }
        if (pAnimTim[cyc] > 20) ChangeAnim(cyc, 0);
    }

    // ---- 80-88: Fall / rise ----
    else if (pAnim[cyc] >= 80 && pAnim[cyc] <= 88) {
        anim = pAnim[cyc];
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 3, 2.0f, pSeq[cyc][anim], 0.0f);
            pState[cyc] = anim;
        }
        int endTim = (anim==81||anim==84) ? 999 : 40;
        if (pAnimTim[cyc] > endTim) {
            if (anim==80) ChangeAnim(cyc, 81);
            else if (anim==81) { if (DirPressed(cyc)||ActionPressed(cyc)) ChangeAnim(cyc, 82); }
            else if (anim==82) ChangeAnim(cyc, 0);
            else if (anim==83) ChangeAnim(cyc, 84);
            else if (anim==84) { if (DirPressed(cyc)||ActionPressed(cyc)) ChangeAnim(cyc, 85); }
            else if (anim==85) ChangeAnim(cyc, 0);
            else if (anim==86) ChangeAnim(cyc, 84);
            else ChangeAnim(cyc, 0);
        }
    }

    // ---- Seated gestures (100-109) ----
    else if (pAnim[cyc] >= 100 && pAnim[cyc] <= 109) {
        anim = pAnim[cyc];
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 1, RndF(0.1f,0.3f), pSeq[cyc][anim], 5.0f);
            pState[cyc] = anim;
        }
    }

    // ---- Standing gestures (90-98) ----
    else if (pAnim[cyc] >= 90 && pAnim[cyc] <= 98) {
        anim = pAnim[cyc];
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], (anim==92||anim==96) ? 1 : 3, RndF(0.5f,1.5f), pSeq[cyc][anim], 5.0f);
            pState[cyc] = anim;
        }
        
        if (anim == 90 && pChar[cyc] == gamChar[slot] && gamDoor > 0) {
            int loc = gamLocation[slot];
            if (loc >= 0 && loc < MAX_LOCS_D && gamDoor < MAX_DOORS) {
                if (!SatisfiedAngle(pA[cyc], doorA[loc][gamDoor], 10.0f)) {
                    pA[cyc] = pA[cyc] + ReachAngle(pA[cyc], doorA[loc][gamDoor], 5.0f);
                    pA[cyc] = CleanAngle(pA[cyc]);
                }
            }
            if (pAnimTim[cyc] > 10) EnterDoor(cyc, gamDoor);
        }
        
        if (pAnimTim[cyc] > 60 && anim != 96) ChangeAnim(cyc, 0);
    }

    // ---- Speaking (120-123) ----
    else if (pAnim[cyc] >= 120 && pAnim[cyc] <= 123) {
        anim = pAnim[cyc];
        if (pAnimTim[cyc] == 0 || anim != pState[cyc]) {
            Animate(p[cyc], 1, RndF(0.25f,0.5f), pSeq[cyc][anim], 10.0f);
            pState[cyc] = anim;
        }
    }

    pAnimTim[cyc]++;
}

// (CombatAnims forward-declared above Animations)

// ---------------------------------------------------------------------------
//  CombatAnims  –  Missing combat attacks 30-65 from Anims.bb
// ---------------------------------------------------------------------------
static void CombatAnims(int cyc) {
    int anim = pAnim[cyc];

    // ---- 30: upper punch ----
    if (anim == 30) {
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 4.0f, pSeq[cyc][30], 5.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 3) ProduceSound(p[cyc], sSwing, 22050, RndF(0.1f, 0.3f));
        if (pAnimTim[cyc] <= 15) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f);
            RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.5f);
            pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 4 && pAnimTim[cyc] <= 10 && pScar[cyc][18] <= 4 && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = pAnimTim[cyc] - 3;
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 15 + range) && pY[cyc] > pY[v] - 30.0f && pY[cyc] < pY[v] + 5.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 5 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], sImpact[Rnd(1, 2)], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 22.0f, pZ[v], 2, 10, 99); ScarLimb(v, 1, 10);
                            ChangeAnim(v, 70); pDT[v] = (110 - pHealth[v]) * 2;
                            pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f); CreateSpurt(pX[v], pY[cyc] + 22.0f, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                            pHP[v] -= Rnd(0, 1);
                        }
                        WeaponImpact(cyc, v, blocked);
                        pHurtA[v] = pA[cyc]; pStagger[v] = (8.0f - contact) * 0.2f; if (pStagger[v] < 0.2f) pStagger[v] = 0.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 50); DamageRep(cyc, v, 0); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 18) ChangeAnim(cyc, 0);
        if (pWeapon[cyc] > 0 && (weapStyle[weapType[pWeapon[cyc]]] == 1 || weapStyle[weapType[pWeapon[cyc]]] == 7)) ChangeAnim(cyc, 40);
    }
    // ---- 31: lower kick ----
    else if (anim == 31) {
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 3.5f, pSeq[cyc][31], 8.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 2) ProduceSound(p[cyc], sSwing, 22050, RndF(0.1f, 0.3f));
        if (pAnimTim[cyc] <= 18) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f); RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.5f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 4 && pAnimTim[cyc] <= 10 && pScar[cyc][32] <= 4 && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = pAnimTim[cyc]; if (range > 7) range = 7;
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 15 + range) && pY[cyc] > pY[v] - 15.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 5 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 15.0f, pZ[v], 2, 10, 99); ScarArea(v, pX[v], pY[cyc] + 15.0f, pZ[v], 10);
                            ChangeAnim(v, 71); pDT[v] = (110 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f); CreateSpurt(pX[v], pY[cyc] + 15.0f, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); } pHP[v] -= Rnd(0, 1);
                        }
                        pHurtA[v] = pA[cyc]; pStagger[v] = (8.0f - contact) * 0.3f; if (pStagger[v] < 0.3f) pStagger[v] = 0.3f;
                        RiskAnger(cyc, v); GainStrength(cyc, 50); DamageRep(cyc, v, 0); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 22) ChangeAnim(cyc, 1);
        if (pWeapon[cyc] > 0 && (weapStyle[weapType[pWeapon[cyc]]] == 1 || weapStyle[weapType[pWeapon[cyc]]] == 7)) ChangeAnim(cyc, 41);
    }
    // ---- 32: stomp ----
    else if (anim == 32) {
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 3.0f, pSeq[cyc][32], 5.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 3) ProduceSound(p[cyc], sSwing, 22050, RndF(0.1f, 0.3f));
        if (pAnimTim[cyc] <= 12) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f);
            if (!InProximity(cyc, pFoc[cyc], 10)) { RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.5f); }
        }
        if (pAnimTim[cyc] >= 7 && pAnimTim[cyc] <= 11 && pScar[cyc][35] <= 4 && pSting[cyc] == 1) {
            int v = pFoc[cyc];
            if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 25) && pY[cyc] > pY[v] - 15.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) == 3 && pSting[cyc] == 1) {
                int contact = InRange(cyc, v, 6);
                if (contact > 0) {
                    charAttacker[pChar[v]] = pChar[cyc];
                    ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f);
                    if (pHealth[v] > 0) ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                    Handle limb = pLimb[cyc][36];
                    CreateSpurt(EntityX(limb, 1), pY[v], EntityZ(limb, 1), 2, 10, 99); ScarArea(v, EntityX(limb, 1), pY[v], EntityZ(limb, 1), 10);
                    GroundReaction(v); pDT[v] -= 10; pHealth[v] -= GetPower(cyc);
                    RiskAnger(cyc, v); GainStrength(cyc, 100); DamageRep(cyc, v, 0); pSting[cyc] = 0;
                }
            }
        }
        if (pAnimTim[cyc] > 18) ChangeAnim(cyc, 0);
        if (pWeapon[cyc] > 0 && (weapStyle[weapType[pWeapon[cyc]]] <= 1 || weapStyle[weapType[pWeapon[cyc]]] == 7)) ChangeAnim(cyc, 42); 
    }
    // ---- 33: big attack ----
    else if (anim == 33) {
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 3.0f, pSeq[cyc][33], 10.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 5) ProduceSound(p[cyc], sSwing, 22050, RndF(0.3f, 0.5f));
        if (pAnimTim[cyc] <= 16) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f); RotateEntity(pPivot[cyc], 0, pA[cyc], 0);
            MoveEntity(pPivot[cyc], 0, 0, 0.5f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 10 && pAnimTim[cyc] <= 14 && (pScar[cyc][18] <= 4 || pScar[cyc][5] <= 4) && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = pAnimTim[cyc] - 8; if (range > 5) range = 5;
                if (pWeapon[cyc] > 0) range++;
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 15 + range) && pY[cyc] > pY[v] - 30.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 3 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], sImpact[3], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 1.0f);
                            float impactY = pY[cyc] + 20.0f; if (impactY > EntityY(pLimb[v][1], 1)) impactY = EntityY(pLimb[v][1], 1);
                            CreateSpurt(pX[v], impactY, pZ[v], 2, 10, 99); ScarLimb(v, 1, 10);
                            ChangeAnim(v, 70); pDT[v] = (150 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc) * 2; pHP[v] -= GetPower(cyc) * 2;
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], sImpact[6], 22050, 0.0f);
                            float impactY = pY[cyc] + 20.0f; if (impactY > EntityY(pLimb[v][1], 1)) impactY = EntityY(pLimb[v][1], 1);
                            CreateSpurt(pX[v], impactY, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                            if (pWeapon[v] == 0) pHealth[v] -= 1; pHP[v] -= 1;
                        }
                        WeaponImpact(cyc, v, blocked);
                        pHurtA[v] = pA[cyc]; pStagger[v] = (8.0f - contact) * 0.2f; if (pStagger[v] < 0.2f) pStagger[v] = 0.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 25); DamageRep(cyc, v, 1); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 23) ChangeAnim(cyc, 0);
        if (pWeapon[cyc] > 0 && (weapStyle[weapType[pWeapon[cyc]]] == 1 || weapStyle[weapType[pWeapon[cyc]]] == 7)) ChangeAnim(cyc, 43);
    }
    // ---- 34: rear attack ----
    else if (anim == 34) {
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 3, 4.0f, pSeq[cyc][34], 10.0f);
            for (int v = 1; v <= no_plays; v++) pMultiSting[cyc][v] = 1;
        }
        if (pAnimTim[cyc] == 5) ProduceSound(p[cyc], sSwing, 22050, RndF(0.3f, 0.5f));
        if (pAnimTim[cyc] >= 5 && pAnimTim[cyc] <= 16) {
            RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, -1.0f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 7 && pAnimTim[cyc] <= 17 && pScar[cyc][18] <= 4) {
            for (int v = 1; v <= no_plays; v++) {
                int range = pAnimTim[cyc] - 8; if (range > 5) range = 5;
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && LimbProximity(pLimb[cyc][18], pX[v], pZ[v], 8) && pY[cyc] > pY[v] - 30.0f && pY[cyc] < pY[v] + 5.0f && AttackViable(v) == 1 && pMultiSting[cyc][v] == 1) {
                    charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                    if (Rnd(0, 10) <= 3 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                    if (blocked == 0) {
                        ProduceSound(p[v], sImpact[3], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 1.0f);
                        Handle limb = pLimb[cyc][18]; CreateSpurt(EntityX(limb, 1), pY[cyc] + 20.0f, EntityZ(limb, 1), 2, 10, 99);
                        ScarLimb(v, 1, 10); ChangeAnim(v, 70); pDT[v] = (150 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc) * 2; pHP[v] -= GetPower(cyc) * 2;
                    } else {
                        if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                        ProduceSound(p[v], sImpact[6], 22050, 0.0f); CreateSpurt(pX[v], pY[cyc] + 20.0f, pZ[v], 2, 10, 4);
                        if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                        if (pWeapon[v] == 0) pHealth[v] -= 1; pHP[v] -= 1;
                    }
                    WeaponImpact(cyc, v, blocked);
                    pHurtA[v] = pA[v] + 180.0f; pStagger[v] = 1.2f;
                    RiskAnger(cyc, v); GainStrength(cyc, 25); DamageRep(cyc, v, 1); pMultiSting[cyc][v] = 0;
                }
            }
        }
        if (pAnimTim[cyc] > 30) { SharpTransition(cyc, 1, 180.0f); ChangeAnim(cyc, 0); }
        if (pWeapon[cyc] > 0 && weapStyle[weapType[pWeapon[cyc]]] == 1) ChangeAnim(cyc, 44);
    }
    // ---- 35: rising punch ----
    else if (anim == 35) {
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 3.0f, pSeq[cyc][35], 5.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 3) ProduceSound(p[cyc], sSwing, 22050, RndF(0.1f, 0.3f));
        if (pAnimTim[cyc] <= 15) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f); RotateEntity(pPivot[cyc], 0, pA[cyc], 0);
            MoveEntity(pPivot[cyc], 0, 0, 0.2f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 6 && pAnimTim[cyc] <= 11 && pScar[cyc][18] <= 4 && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = pAnimTim[cyc] - 3; if (range > 6) range = 6;
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 15 + range) && pY[cyc] > pY[v] - 15.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 5 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 15.0f, pZ[v], 2, 10, 99); ScarArea(v, pX[v], pY[cyc] + 15.0f, pZ[v], 10);
                            ChangeAnim(v, 71); pDT[v] = (110 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f); CreateSpurt(pX[v], pY[cyc] + 15.0f, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); } pHP[v] -= Rnd(0, 1);
                        }
                        WeaponImpact(cyc, v, blocked);
                        pHurtA[v] = pA[cyc]; pStagger[v] = (8.0f - contact) * 0.2f; if (pStagger[v] < 0.2f) pStagger[v] = 0.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 50); DamageRep(cyc, v, 0); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 25) ChangeAnim(cyc, 1);
        if (pWeapon[cyc] > 0 && (weapStyle[weapType[pWeapon[cyc]]] == 1 || weapStyle[weapType[pWeapon[cyc]]] == 7)) ChangeAnim(cyc, 41);
    }
    // ---- 40: upper swing ----
    else if (anim == 40) {
        int actAnim = 40; if (weapStyle[weapType[pWeapon[cyc]]] == 7) actAnim = 51;
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 4.0f, pSeq[cyc][actAnim], 5.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 3) ProduceSound(p[cyc], sSwing, 22050, 0.0f);
        if (pAnimTim[cyc] <= 11) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f); RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.4f); pStepTim[cyc] += Rnd(0, 1);
        }
        int impactTim = (weapStyle[weapType[pWeapon[cyc]]] == 7) ? 9 : 5;
        if (pAnimTim[cyc] >= impactTim && pAnimTim[cyc] <= impactTim + 4 && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = weapRange[weapType[pWeapon[cyc]]];
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 20 + range) && pY[cyc] > pY[v] - 30.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 5 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 1.0f);
                            if (weapStyle[weapType[pWeapon[cyc]]] == 7) ProduceSound(p[v], sStab, 22050, 1.0f);
                            ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 20.0f, pZ[v], 2, 10, 99); ScarArea(v, pX[v], pY[cyc] + 20.0f, pZ[v], 2);
                            if (CountScars(v) >= 2) { ScarWeapon(pWeapon[cyc], 5); CreatePool(pX[v], pGround[v], pZ[v], RndF(2.0f, 8.0f), 1, 1); }
                            ChangeAnim(v, 70); pDT[v] = (110 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                            pDT[v] += weapDamage[weapType[pWeapon[cyc]]] * 10;
                            pHealth[v] -= Rnd(1, weapDamage[weapType[pWeapon[cyc]]]); pHP[v] -= Rnd(1, weapDamage[weapType[pWeapon[cyc]]]);
                            if (weapName[weapType[pWeapon[cyc]]] == "Syringe" && pInjured[v] < 100) pInjured[v] = Rnd(100, 500);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 0.0f); ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 20.0f, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                            if (pWeapon[v] == 0) pHealth[v] -= 1; pHP[v] -= Rnd(0, 1);
                        }
                        pHurtA[v] = pA[cyc]; pStagger[v] = (range - contact) * 0.2f; if (pStagger[v] < 0.2f) pStagger[v] = 0.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 50); DamageRep(cyc, v, 1); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 18) ChangeAnim(cyc, 0);
    }
    // ---- 41: lower swing ----
    else if (anim == 41) {
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 4.0f, pSeq[cyc][41], 5.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 3) ProduceSound(p[cyc], sSwing, 22050, 0.0f);
        if (pAnimTim[cyc] <= 11) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f); RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.4f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 5 && pAnimTim[cyc] <= 9 && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = weapRange[weapType[pWeapon[cyc]]] - 1;
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 20 + range) && pY[cyc] > pY[v] - 15.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 5 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 1.0f);
                            if (weapStyle[weapType[pWeapon[cyc]]] == 7) ProduceSound(p[v], sStab, 22050, 1.0f);
                            ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 10.0f, pZ[v], 2, 10, 99); ScarArea(v, pX[v], pY[cyc] + 10.0f, pZ[v], 2);
                            if (CountScars(v) >= 2) { ScarWeapon(pWeapon[cyc], 5); CreatePool(pX[v], pGround[v], pZ[v], RndF(2.0f, 8.0f), 1, 1); }
                            ChangeAnim(v, 71); pDT[v] = (110 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                            pDT[v] += weapDamage[weapType[pWeapon[cyc]]] * 10;
                            pHealth[v] -= Rnd(1, weapDamage[weapType[pWeapon[cyc]]]); pHP[v] -= Rnd(1, weapDamage[weapType[pWeapon[cyc]]]);
                            if (weapName[weapType[pWeapon[cyc]]] == "Syringe" && pInjured[v] < 100) pInjured[v] = Rnd(100, 500);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 0.0f); ProduceSound(p[v], sImpact[Rnd(4, 5)], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 10.0f, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                            if (pWeapon[v] == 0) pHealth[v] -= 1; pHP[v] -= Rnd(0, 1);
                        }
                        pHurtA[v] = pA[cyc]; pStagger[v] = (range - contact) * 0.2f; if (pStagger[v] < 0.2f) pStagger[v] = 0.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 50); DamageRep(cyc, v, 1); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 20) ChangeAnim(cyc, 1);
    }
    // ---- 42: ground swing ----
    else if (anim == 42) {
        int actAnim = 42; if (weapStyle[weapType[pWeapon[cyc]]] == 7) actAnim = 52;
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 3.5f, pSeq[cyc][actAnim], 10.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 4) ProduceSound(p[cyc], sSwing, 22050, 0.0f);
        if (pAnimTim[cyc] <= 14) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f);
            if (!InProximity(cyc, pFoc[cyc], 15)) { RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.3f); }
        }
        if (pAnimTim[cyc] >= 10 && pAnimTim[cyc] <= 15 && pSting[cyc] == 1) {
            int v = pFoc[cyc];
            if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && pY[cyc] > pY[v] - 15.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) == 3 && pSting[cyc] == 1) {
                int range = weapRange[weapType[pWeapon[cyc]]] + (weapRange[weapType[pWeapon[cyc]]] / 3);
                if (LimbProximity(FindChild(p[cyc], weapFile[weapType[pWeapon[cyc]]]), pX[v], pZ[v], range) || LimbProximity(pLimb[cyc][19], pX[v], pZ[v], range / 2.0f)) {
                    charAttacker[pChar[v]] = pChar[cyc];
                    ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 1.0f);
                    if (weapStyle[weapType[pWeapon[cyc]]] == 7) ProduceSound(p[v], sStab, 22050, 1.0f);
                    if (pHealth[v] > 0) ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 0.0f);
                    Handle limb = FindChild(p[cyc], weapFile[weapType[pWeapon[cyc]]]);
                    CreateSpurt(pX[v], pY[v], pZ[v], 3, 10, 99); ScarArea(v, pX[v], pY[v], pZ[v], 2);
                    if (CountScars(v) >= 2) { ScarWeapon(pWeapon[cyc], 5); CreatePool(pX[v], pGround[v], pZ[v], RndF(2.0f, 8.0f), 1, 1); }
                    GroundReaction(v); pDT[v] -= 10; pHealth[v] -= GetPower(cyc);
                    pHealth[v] -= Rnd(1, weapDamage[weapType[pWeapon[cyc]]]);
                    if (weapName[weapType[pWeapon[cyc]]] == "Syringe" && pInjured[v] < 100) pInjured[v] = Rnd(100, 500);
                    RiskAnger(cyc, v); GainStrength(cyc, 50); DamageRep(cyc, v, 1); pSting[cyc] = 0;
                }
            }
        }
        if (pAnimTim[cyc] > 22) ChangeAnim(cyc, 0);
    }
    // ---- 43: big swing ----
    else if (anim == 43) {
        int actAnim = 43; if (weapStyle[weapType[pWeapon[cyc]]] == 7) actAnim = 53;
        if (pAnimTim[cyc] == 0) { Animate(p[cyc], 3, 3.0f, pSeq[cyc][actAnim], 10.0f); pSting[cyc] = 1; }
        if (pAnimTim[cyc] == 5) ProduceSound(p[cyc], sSwing, 22050, 0.0f);
        if (pAnimTim[cyc] <= 16) {
            FaceEntity(cyc, p[pFoc[cyc]], 5.0f); RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, 0.4f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 10 && pAnimTim[cyc] <= 15 && pSting[cyc] == 1) {
            for (int v = 1; v <= no_plays; v++) {
                int range = weapRange[weapType[pWeapon[cyc]]];
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && InProximity(cyc, v, 20 + range) && pY[cyc] > pY[v] - 30.0f && pY[cyc] < pY[v] + 15.0f && AttackViable(v) >= 1 && AttackViable(v) <= 2 && pSting[cyc] == 1) {
                    int contact = InRange(cyc, v, range);
                    if (contact > 0) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 3 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 1.0f);
                            if (weapStyle[weapType[pWeapon[cyc]]] == 7) ProduceSound(p[v], sStab, 22050, 1.0f);
                            ProduceSound(p[v], sImpact[3], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 1.0f);
                            float impactY = pY[cyc] + 20.0f; if (impactY > EntityY(pLimb[v][1], 1)) impactY = EntityY(pLimb[v][1], 1);
                            CreateSpurt(pX[v], impactY, pZ[v], 2, 10, 99); ScarArea(v, pX[v], impactY, pZ[v], 2);
                            if (CountScars(v) >= 2) { ScarWeapon(pWeapon[cyc], 5); CreatePool(pX[v], pGround[v], pZ[v], RndF(2.0f, 8.0f), 1, 1); }
                            ChangeAnim(v, 70); pDT[v] = (150 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                            pDT[v] += weapDamage[weapType[pWeapon[cyc]]] * 10;
                            pHealth[v] -= weapDamage[weapType[pWeapon[cyc]]]; pHP[v] -= weapDamage[weapType[pWeapon[cyc]]];
                            if (weapName[weapType[pWeapon[cyc]]] == "Syringe" && pInjured[v] < 100) pInjured[v] = Rnd(100, 500);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 0.0f); ProduceSound(p[v], sImpact[6], 22050, 0.0f);
                            float impactY = pY[cyc] + 20.0f; if (impactY > EntityY(pLimb[v][1], 1)) impactY = EntityY(pLimb[v][1], 1);
                            CreateSpurt(pX[v], impactY, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                            if (pWeapon[v] == 0) pHealth[v] -= 1; pHP[v] -= 1;
                        }
                        pHurtA[v] = pA[cyc]; pStagger[v] = (range - contact) * 0.2f; if (pStagger[v] < 0.2f) pStagger[v] = 0.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 25); DamageRep(cyc, v, 2); pSting[cyc] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 26) ChangeAnim(cyc, 0);
    }
    // ---- 44: rear swing ----
    else if (anim == 44) {
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 3, 4.0f, pSeq[cyc][44], 10.0f);
            for (int v = 1; v <= no_plays; v++) pMultiSting[cyc][v] = 1;
        }
        if (pAnimTim[cyc] == 5) ProduceSound(p[cyc], sSwing, 22050, 0.0f);
        if (pAnimTim[cyc] >= 5 && pAnimTim[cyc] <= 16) {
            RotateEntity(pPivot[cyc], 0, pA[cyc], 0); MoveEntity(pPivot[cyc], 0, 0, -1.0f); pStepTim[cyc] += Rnd(0, 1);
        }
        if (pAnimTim[cyc] >= 7 && pAnimTim[cyc] <= 20) {
            for (int v = 1; v <= no_plays; v++) {
                if (cyc != v && (!Friendly(cyc, v) || v == pFoc[cyc]) && pY[cyc] > pY[v] - 30.0f && pY[cyc] < pY[v] + 5.0f && AttackViable(v) == 1 && pMultiSting[cyc][v] == 1) {
                    if (LimbProximity(FindChild(p[cyc], weapFile[weapType[pWeapon[cyc]]]), pX[v], pZ[v], 8) || LimbProximity(pLimb[cyc][18], pX[v], pZ[v], 8) || LimbProximity(pLimb[cyc][19], pX[v], pZ[v], 8)) {
                        charAttacker[pChar[v]] = pChar[cyc]; int blocked = 0;
                        if (Rnd(0, 10) <= 3 + BlockPower(v) && pAnim[v] >= 74 && pAnim[v] <= 75 && InLine(v, p[cyc], 90.0f)) blocked = 1;
                        if (blocked == 0) {
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 1.0f);
                            if (weapStyle[weapType[pWeapon[cyc]]] == 7) ProduceSound(p[v], sStab, 22050, 1.0f);
                            ProduceSound(p[v], sImpact[3], 22050, 0.0f); ProduceSound(p[v], sPain[Rnd(1, 8)], 22050, 1.0f);
                            CreateSpurt(pX[v], pY[cyc] + 20.0f, pZ[v], 2, 10, 99); ScarArea(v, pX[v], pY[cyc] + 20.0f, pZ[v], 2);
                            if (CountScars(v) >= 2) { ScarWeapon(pWeapon[cyc], 5); CreatePool(pX[v], pGround[v], pZ[v], RndF(2.0f, 8.0f), 1, 1); }
                            ChangeAnim(v, 70); pDT[v] = (150 - pHealth[v]) * 2; pHealth[v] -= GetPower(cyc); pHP[v] -= GetPower(cyc);
                            pDT[v] += weapDamage[weapType[pWeapon[cyc]]] * 10;
                            pHealth[v] -= weapDamage[weapType[pWeapon[cyc]]]; pHP[v] -= weapDamage[weapType[pWeapon[cyc]]];
                            if (weapName[weapType[pWeapon[cyc]]] == "Syringe" && pInjured[v] < 100) pInjured[v] = Rnd(100, 500);
                        } else {
                            if (pWeapon[v] > 0) { ProduceSound(p[v], weapSound[weapType[pWeapon[v]]], 22050, 0.0f); DropWeapon(v, 10); }
                            ProduceSound(p[v], weapSound[weapType[pWeapon[cyc]]], 22050, 1.0f); ProduceSound(p[v], sImpact[6], 22050, 0.0f);
                            CreateSpurt(pX[v], pY[cyc] + 20.0f, pZ[v], 2, 10, 4);
                            if (pWeapon[v] == 0) { for (int limb = 4; limb <= 29; limb++) ScarLimb(v, limb, 10); }
                            if (pWeapon[v] == 0) pHealth[v] -= 1; pHP[v] -= 1;
                        }
                        pHurtA[v] = pA[v] + 180.0f; pStagger[v] = 1.2f;
                        RiskAnger(cyc, v); GainStrength(cyc, 25); DamageRep(cyc, v, 2); pMultiSting[cyc][v] = 0;
                    }
                }
            }
        }
        if (pAnimTim[cyc] > 30) { SharpTransition(cyc, 1, 180.0f); ChangeAnim(cyc, 0); }
    }
}

// ---------------------------------------------------------------------------
//  MoveAnims  –  grapple move animations (200+)
// ---------------------------------------------------------------------------
void MoveAnims(int cyc) {
    // Standing grapple lunge (200)
    if (pAnim[cyc] == 200 || pAnim[cyc] == 201) {
        if (pAnimTim[cyc] == 0) {
            Animate(p[cyc], 3, 3.0f, pSeq[cyc][pAnim[cyc]], 5.0f);
        }
        if (pAnimTim[cyc] <= 12) {
            FaceEntity(cyc, p[pFoc[cyc]], 10.0f);
            RotateEntity(pPivot[cyc], 0.0f, pA[cyc], 0.0f);
            MoveEntity(pPivot[cyc], 0.0f, 0.0f, 0.5f);
        }
        // Attempt grapple on contact window
        if (pAnimTim[cyc] >= 8 && pAnimTim[cyc] <= 13 && pGrappling[cyc] == 0) {
            for (int v = 1; v <= no_plays; ++v) {
                if (cyc == v) continue;
                float dx = pX[v]-pX[cyc], dz = pZ[v]-pZ[cyc];
                float dist = std::sqrt(dx*dx+dz*dz);
                if (dist < 20.0f && pGrappling[v]==0 && pGrappler[v]==0) {
                    charAttacker[pChar[v]] = pChar[cyc];
                    ProduceSound(p[v], sImpact[Rnd(4,5)], 22050, 0.0f);
                    if (pHealth[v]>0) ProduceSound(p[v], sPain[Rnd(1,8)], 22050, 0.0f);
                    ChangeAnim(cyc, 203); ChangeAnim(v, 202);
                    pGrappling[cyc] = v; pGrappler[v] = cyc;
                    FixGrapple(cyc, v);
                    break;
                }
            }
        }
        if (pAnimTim[cyc] > 26) ChangeAnim(cyc, 0);
    }

    // Hold headlock (205)
    else if (pAnim[cyc] == 205) {
        int v = pGrappling[cyc];
        if (pAnimTim[cyc]==0) {
            Animate(p[cyc], 1, 0.5f, pSeq[cyc][206], 5.0f);
            Animate(p[v],   1, 0.5f, pSeq[v][207], 5.0f);
        }
        FixGrapple(cyc, v);
        if (DirPressed(cyc)) ChangeAnim(cyc, 206);
        FindMoveCommands(cyc);
    }

    // Headlock movement (206)
    else if (pAnim[cyc] == 206) {
        int v = pGrappling[cyc];
        if (pAnimTim[cyc]==0) {
            Animate(p[cyc], 1, 1.5f, pSeq[cyc][208], 5.0f);
            Animate(p[v],   1, 1.5f, pSeq[v][209], 5.0f);
        }
        if (cLeft[cyc])  pA[cyc] = CleanAngle(pA[cyc]+1.0f);
        if (cRight[cyc]) pA[cyc] = CleanAngle(pA[cyc]-1.0f);
        if (VerticalPressed(cyc)) {
            RotateEntity(pPivot[cyc], 0.0f, pA[cyc], 0.0f);
            if (cUp[cyc])   MoveEntity(pPivot[cyc], 0.0f, 0.0f, -0.3f);
            if (cDown[cyc]) MoveEntity(pPivot[cyc], 0.0f, 0.0f,  0.15f);
        }
        FixGrapple(cyc, v);
        if (pAnimTim[cyc] > 5 && !DirPressed(cyc)) ChangeAnim(cyc, 205);
        FindMoveCommands(cyc);
        pStepTim[cyc]++;
    }

    // Release headlock (210)
    else if (pAnim[cyc] == 210) {
        int v = pGrappling[cyc];
        if (pAnimTim[cyc]==0) {
            Animate(p[cyc], 3, 3.0f, pSeq[cyc][210], 0.0f);
            Animate(p[v],   3, 3.0f, pSeq[v][211], 0.0f);
        }
        if (pAnimTim[cyc] > 26) {
            charReputation[pChar[cyc]]--;
            pGrappling[cyc]=0; pGrappler[v]=0;
            ChangeAnim(cyc, 0); SharpTransition(cyc, 1, -1.0f);
            ChangeAnim(v, 0);   SharpTransition(v, 1, -1.0f);
        }
    }

    // Bodyslam (218)
    else if (pAnim[cyc] == 218) {
        int v = pGrappling[cyc];
        if (pAnimTim[cyc]==0) {
            Animate(p[cyc], 3, 2.0f, pSeq[cyc][218], 0.0f);
            Animate(p[v],   3, 2.0f, pSeq[v][219], 0.0f);
        }
        if (pAnimTim[cyc] == 80) {
            pHealth[v] -= 25;
            pHP[v] = 0;
            ProduceSound(p[v], sImpact[Rnd(1,3)], 22050, 0.0f);
            if (pHealth[v]>0) ProduceSound(p[v], sPain[Rnd(1,8)], 22050, 0.0f);
            CreateSpurt(pX[v], pY[v]+20.0f, pZ[v], 5, 5, 3);
        }
        if (pAnimTim[cyc] > 90) {
            pGrappling[cyc]=0; pGrappler[v]=0;
            ChangeAnim(cyc, 0);
            ChangeAnim(v, 84);
        }
    }

    // Chokeslam (220)
    else if (pAnim[cyc] == 220) {
        int v = pGrappling[cyc];
        if (pAnimTim[cyc]==0) {
            Animate(p[cyc], 3, 2.0f, pSeq[cyc][220], 0.0f);
            Animate(p[v],   3, 2.0f, pSeq[v][221], 0.0f);
        }
        if (pAnimTim[cyc] == 100) {
            pHealth[v] -= 30;
            pHP[v] = 0;
            ProduceSound(p[v], sFall, 22050, 0.0f);
            CreateSpurt(pX[v], pY[v]+10.0f, pZ[v], 5, 8, 4);
        }
        if (pAnimTim[cyc] > 120) {
            pGrappling[cyc]=0; pGrappler[v]=0;
            ChangeAnim(cyc, 0); ChangeAnim(v, 80);
        }
    }

    // Default: any unhandled move anim
    else {
        if (pAnimTim[cyc] > 40) ChangeAnim(cyc, 0);
    }
}

// ---------------------------------------------------------------------------
//  AnimatePlayers  –  called from gameplay loop
// ---------------------------------------------------------------------------
void AnimatePlayers() {
    for (int cyc = 1; cyc <= no_plays; ++cyc) {
        if (p[cyc] == 0) continue;
        Animations(cyc);
        FacialExpressions(cyc);
    }
}
