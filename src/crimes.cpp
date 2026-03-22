#include "ai.h"
// =============================================================================
//  crimes.cpp  –  Crime system, court case, combat helpers  (from Crimes.bb)
// =============================================================================
#include "crimes.h"
#include "players.h"
#include "values.h"
#include "anims.h"
#include "particles.h"
#include "functions.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <cmath>

// ---------------------------------------------------------------------------
//  AttackViable  –  returns combat state of player v
//  1=standing  2=kneeling  3=on ground  0=dead/immune
// ---------------------------------------------------------------------------
int AttackViable(int cyc) {
    if (pHealth[cyc] <= 0) return 0;
    if (pAnim[cyc] >= 80 && pAnim[cyc] <= 86)  return 3;  // on ground
    if (pAnim[cyc] == 1  || pAnim[cyc] == 11)  return 2;  // kneeling
    if (pGrappler[cyc] > 0)                     return 2;
    return 1;
}

// ---------------------------------------------------------------------------
//  Friendly  –  1 if cyc and v are on the same side
// ---------------------------------------------------------------------------
int Friendly(int cyc, int v) {
    if (charRole[pChar[cyc]] == 1 && charRole[pChar[v]] == 1) return 1;
    if (charGang[pChar[cyc]] > 0 &&
        charGang[pChar[cyc]] == charGang[pChar[v]]) return 1;
    if (charRelation[pChar[cyc]][pChar[v]] > 0) return 1;
    return 0;
}

// (InProximity defined in functions.cpp)

// ---------------------------------------------------------------------------
//  InRange  –  3D distance check (more precise)
// ---------------------------------------------------------------------------
int InRange(int cyc, int v, float range) {
    float dx=pX[cyc]-pX[v], dy=pY[cyc]-pY[v], dz=pZ[cyc]-pZ[v];
    return (dx*dx+dy*dy+dz*dz < range*range) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  InLine  –  entity within FOV cone of cyc
// ---------------------------------------------------------------------------
int InLine(int cyc, Handle entity, float fov) {
    float tx = EntityX(entity) - pX[cyc];
    float tz = EntityZ(entity) - pZ[cyc];
    float angle = ATan2(tx, tz);
    float diff  = std::fabs(CleanAngle(angle - pA[cyc]));
    return (diff < fov) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  RiskAnger  –  chance to turn a character hostile toward attacker
// ---------------------------------------------------------------------------
void RiskAnger(int cyc, int victim) {
    int vChar = pChar[victim];
    int aChar = pChar[cyc];
    if (charRelation[vChar][aChar] > 0) {
        charRelation[vChar][aChar] = 0;
    }
    charAngerTim[vChar][aChar] = Rnd(200, 600);
}

// ---------------------------------------------------------------------------
//  DamageRep  –  reduce reputation of attacker in bystanders' eyes
// ---------------------------------------------------------------------------
void DamageRep(int cyc, int /*victim*/, int severe) {
    int delta = severe ? -3 : -1;
    charReputation[pChar[cyc]] = std::max(0, charReputation[pChar[cyc]] + delta);
}

// ---------------------------------------------------------------------------
//  ChangeRelationship
// ---------------------------------------------------------------------------
void ChangeRelationship(int charA, int charB, int delta) {
    charRelation[charA][charB] = std::max(-1, std::min(1, charRelation[charA][charB] + delta));
    charRelation[charB][charA] = charRelation[charA][charB];
}

// ---------------------------------------------------------------------------
//  GainStrength  –  XP → stat gain
// ---------------------------------------------------------------------------
void GainStrength(int cyc, int amount) {
    charExperience[pChar[cyc]] += amount;
    if (charExperience[pChar[cyc]] >= 1000) {
        charExperience[pChar[cyc]] -= 1000;
        charStrength[pChar[cyc]] = std::min(100, charStrength[pChar[cyc]] + 1);
    }
}

// ---------------------------------------------------------------------------
//  RiskInjury  –  chance of permanent injury (broken bone)
// ---------------------------------------------------------------------------
void RiskInjury(int v, int chance) {
    if (Rnd(0, 100) < chance)
        charInjured[pChar[v]] = std::min(3, charInjured[pChar[v]] + 1);
}

// ---------------------------------------------------------------------------
//  ScarArea  –  apply scar decal to a limb group
// ---------------------------------------------------------------------------
void ScarArea(int v, int zone, int /*x*/, int /*y*/, int style) {
    // zone 0 = whole body, otherwise specific limb
    int start = (zone == 0) ? 1 : zone;
    int end   = (zone == 0) ? 40 : zone;
    for (int limb = start; limb <= end; ++limb) {
        if (pScar[v][limb] < 5) {
            pScar[v][limb] = std::max(pScar[v][limb], style);
            charScar[pChar[v]][limb] = pScar[v][limb];
        }
    }
}

// ---------------------------------------------------------------------------
//  CountScars
// ---------------------------------------------------------------------------
int CountScars(int cyc) {
    int count = 0;
    for (int limb = 1; limb <= 40; ++limb)
        if (pScar[cyc][limb] > 0) count++;
    return count;
}

// ---------------------------------------------------------------------------
//  ScarLimb  –  apply scar to specific limb
// ---------------------------------------------------------------------------
void ScarLimb(int cyc, int limb, int chance) {
    if (limb == 0) chance *= 2;
    int randy = Rnd(0, chance);
    if (randy == 0 && pScar[cyc][limb] <= 4) {
        pScar[cyc][limb]++;
        if (pScar[cyc][limb] > 4) pScar[cyc][limb] = 4;
        charScar[pChar[cyc]][limb] = pScar[cyc][limb];
        
        if (pScar[cyc][limb] >= 2 && limb > 0 && pLimb[cyc][limb] > 0) {
            float vol = pScar[cyc][limb] * 0.1f;
            ProduceSound(p[cyc], sBleed, 22050, vol);
            float limbX = EntityX(pLimb[cyc][limb], 1);
            float limbZ = EntityZ(pLimb[cyc][limb], 1);
            CreatePool(limbX, pGround[cyc], limbZ, RndF(1.0f, 5.0f), 1, 1);
        }
        if (limb == 1) { // Head shots cause daze
            pDazed[cyc] = Rnd(50, 200);
        }
    }
}

// ---------------------------------------------------------------------------
//  GetPower / BlockPower
// ---------------------------------------------------------------------------
int GetPower(int cyc) {
    int power = 1;
    int str = charStrength[pChar[cyc]];
    if (str >= 60) power = Rnd(1, 2);
    if (str >= 70) power = 2;
    if (str >= 80) power = Rnd(2, 3);
    if (str >= 90) power = 3;
    return power;
}

int BlockPower(int cyc) {
    int block = GetPower(cyc);
    if (pWeapon[cyc] > 0 && weapStyle[weapType[pWeapon[cyc]]] == 1) {
        block += 1;
    }
    return block;
}

// ---------------------------------------------------------------------------
//  GroundReaction  –  play appropriate ground-hit animation
// ---------------------------------------------------------------------------
void GroundReaction(int v) {
    int r = Rnd(1, 3);
    if (r==1) ChangeAnim(v, 80);
    if (r==2) ChangeAnim(v, 83);
    if (r==3) ChangeAnim(v, 86);
}

// ---------------------------------------------------------------------------
//  DropWeapon  –  force character to lose held weapon
// ---------------------------------------------------------------------------
void DropWeapon(int cyc, int /*range*/) {
    if (pWeapon[cyc] == 0) return;
    int w = pWeapon[cyc];
    weapCarrier[w] = 0;
    pWeapon[cyc]   = 0;
    // Eject weapon entity at character's feet
    PositionEntity(weap[w], pX[cyc], pY[cyc]+5.0f, pZ[cyc]);
    ShowEntity(weap[w]);
    weapGravity[w] = -1.0f;
}

// ---------------------------------------------------------------------------
//  CrimeCycle  –  per-frame warrant/witness processing
// ---------------------------------------------------------------------------
void CrimeCycle() {
    int plr = gamPlayer[slot];
    if (plr <= 0 || plr > no_plays) return;

    // Decrement anger timers
    for (int c = 1; c <= no_chars; ++c) {
        for (int d = 1; d <= no_chars; ++d) {
            if (charAngerTim[c][d] > 0)
                charAngerTim[c][d] -= gamSpeed[slot];
        }
    }

    // Witness detection: guards see the player commit violence
    if (charAttacker[pChar[plr]] != 0 || pWeapon[plr] != 0) {
        for (int c = 1; c <= no_plays; ++c) {
            if (c == plr) continue;
            if (charRole[pChar[c]] != 1) continue;  // only wardens
            if (AttackViable(c) < 1)    continue;
            if (!InLine(c, p[plr], 60)) continue;

            // Escalate warrant
            if (charAttacker[pChar[plr]] != 0 &&
                gamWarrant[slot] < 10)
                gamWarrant[slot]++;

            // Carrying illegal weapon
            if (pWeapon[plr] > 0 &&
                weapValue[weapType[pWeapon[plr]]] > 0 &&
                gamWarrant[slot] < 4)
                gamWarrant[slot] = 4;
        }
        charAttacker[pChar[plr]] = 0;
    }

    // If warrant ≥ 20 → trigger court case
    if (gamWarrant[slot] >= 20) {
        gamWarrant[slot] = 0;
        screen = 52;  // Court Case screen
    }
}

// ---------------------------------------------------------------------------
//  CourtCase  –  screen 52  (ported from Crimes.bb)
// ---------------------------------------------------------------------------
void CourtCase() {
    Loader("Please Wait", "Preparing Court Case");

    // Load courtroom mesh
    Handle courtWorld = LoadAnimMesh("ux0:data/HardTime/World/Courtroom/Courtroom.3ds", 0);
    EntityType(courtWorld, 10, true);

    // Camera setup
    cam = CreateCamera();
    CameraViewport(cam, 0, 0, VITA_SCREEN_W, VITA_SCREEN_H);
    PositionEntity(cam, 50.0f, 30.0f, -80.0f);

    // Ambient lighting
    light[1] = CreateLight(2);
    RotateEntity(light[1], 30.0f, 45.0f, 0.0f);
    LightColor(light[1], 200, 200, 200);

    // Load 5 characters: player, accuser, 2 lawyers, judge
    no_plays = 5;
    pChar[1] = gamChar[slot];
    pChar[2] = promoAccuser;
    pChar[3] = 1; pChar[4] = 2; pChar[5] = 3;

    // Fixed positions
    float posX[] = {0.0f,-45.0f,55.0f,-65.0f,75.0f,7.0f};
    float posY[] = {0.0f,2.0f,2.0f,2.0f,2.0f,19.0f};
    float posZ[] = {0.0f,10.0f,10.0f,7.0f,7.0f,127.0f};
    float posA[] = {0.0f,-15.0f,15.0f,-40.0f,40.0f,180.0f};

    const std::string BASE = "ux0:data/HardTime/";
    for (int cyc = 1; cyc <= 5; ++cyc) {
        BBString model = BASE + "Characters/Models/Model" +
                         Dig(charModel[pChar[cyc]], 10) + ".3ds";
        p[cyc] = LoadAnimMesh(model, 0);
        LoadAnimSequences(cyc);
        ApplyCostume(cyc);

        pX[cyc]=posX[cyc]; pY[cyc]=posY[cyc];
        pZ[cyc]=posZ[cyc]; pA[cyc]=posA[cyc];
        PositionEntity(p[cyc], pX[cyc], pY[cyc], pZ[cyc]);
        RotateEntity(p[cyc], 0.0f, pA[cyc], 0.0f);

        float sc = charHeight[pChar[cyc]] * 0.0025f;
        if (cyc == 5) sc = 12 * 0.0025f;
        ScaleEntity(p[cyc], 0.34f+sc, 0.34f+sc, 0.34f+sc);

        // Idle anims
        if (cyc <= 4) Animate(p[cyc], 1, RndF(0.1f,0.3f), pSeq[cyc][1], 0.0f);
        else          Animate(p[cyc], 1, RndF(0.1f,0.3f), pSeq[cyc][100], 0.0f);
        pState[cyc] = 1;
    }

    // Court verdict loop
    promoTim   = -100;
    promoStage = (gamWarrant[slot] == 0) ? 2 : 0;
    promoVerdict = 0;

    // Fine calculation
    promoCash = gamMoney[slot] / 5;
    if (promoCash < 100)   promoCash = 100;
    if (promoCash > 10000) promoCash = 10000;
    promoCash = RoundOff(promoCash, 100);

    int go = 0;
    while (go == 0 && screen == 52) {
        gInput.Update();

        promoTim++;

        // Skip / speed up
        if (promoTim > 50 &&
            (gInput.Pressed(SCE_CTRL_CROSS) || gInput.Pressed(SCE_CTRL_START)))
            promoTim += 100;

        // Stage: verdict delivered
        if (promoStage == 2 && promoVerdict == 0) {
            if (gamWarrant[slot] > 0) {
                promoVerdict = 1;  // guilty
                int extra = gamWarrant[slot] / 2;
                charSentence[pChar[gamPlayer[slot]]] += extra;
                gamMoney[slot] -= promoCash;
            } else {
                promoVerdict = 2;  // not guilty
            }
        }

        if (promoStage == 3 && promoTim > 200) go = 1;

        // Advance stage
        if (promoTim > 500 && promoStage < 3) promoStage++;

        // Render
        glClearColor(0.1f, 0.08f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        R3D::RenderWorld();

        // 2D text overlay
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrthof(0,VITA_SCREEN_W,VITA_SCREEN_H,0,-1,1);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        SetColor(255,220,120);
        DrawText("THE COURT", VITA_SCREEN_W/2, 30, true, false);

        if (promoVerdict == 1) {
            SetColor(255,80,80);
            DrawText("GUILTY - Sentence extended!", VITA_SCREEN_W/2, VITA_SCREEN_H/2, true, true);
            DrawText("Fine: $" + GetFigure(promoCash), VITA_SCREEN_W/2, VITA_SCREEN_H/2+40, true, true);
        } else if (promoVerdict == 2) {
            SetColor(80,255,80);
            DrawText("NOT GUILTY - You are free to go.", VITA_SCREEN_W/2, VITA_SCREEN_H/2, true, true);
        }

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();

        vglSwapBuffers(GL_FALSE);
    }

    gamWarrant[slot] = 0;
    FreeEntity(courtWorld);
    FreeEntity(cam); cam = 0;
    FreeEntity(light[1]); light[1] = 0;
    screen = 50;  // back to gameplay
}
