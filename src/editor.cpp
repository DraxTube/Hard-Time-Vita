// =============================================================================
//  editor.cpp  –  Character editor  screen 51  (ported from Editor.bb)
//
//  Two pages:
//    Page 1: Profile  (height, strength, agility, intelligence, crime type)
//    Page 2: Appearance  (hair style, hair colour, face, specs, model, costume)
// =============================================================================
#include "gameplay.h"
#include "values.h"
#include "players.h"
#include "costume.h"
#include "texts.h"
#include "functions.h"
#include "anims.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <vitaGL.h>
#include <algorithm>

// ---------------------------------------------------------------------------
//  Static helpers
// ---------------------------------------------------------------------------
static void DrawBar(int x, int y, int val, int maxVal, int r, int g, int b,
                    const BBString& label) {
    int bw=120, bh=10;
    SetColor(30,30,40);   DrawRect(x, y, bw, bh);
    SetColor(r,g,b);      DrawRect(x, y, val*bw/maxVal, bh);
    SetColor(200,200,200);DrawText(label, x-60, y, false, false);
    SetColor(255,255,200);DrawText(Str(val)+"%", x+bw+6, y, false, false);
}

static void DrawSelector(int x, int y, const BBString& label,
                          const BBString& value, bool selected) {
    SetColor(selected?255:100, selected?220:100, selected?60:100);
    DrawText(label + ": ", x-80, y, false, false);
    if (selected) { SetColor(50,50,10); DrawRect(x-2, y-2, 160, 14); }
    SetColor(255,255,200);
    DrawText("< " + value + " >", x+20, y, false, false);
}

// ---------------------------------------------------------------------------
//  Editor  –  screen 51
// ---------------------------------------------------------------------------
void Editor() {
    Loader("Please Wait", "Loading Editor");

    const std::string BASE = "ux0:data/HardTime/";

    // Load prison setting as backdrop
    Handle edWorld = LoadAnimMesh(BASE+"World/Block/Block.3ds", 0);

    // Camera: fixed angle at character creation spot
    cam = CreateCamera();
    CameraViewport(cam, 0, 0, VITA_SCREEN_W, VITA_SCREEN_H);
    PositionEntity(cam, -232.0f, 36.0f, -89.0f);
    RotateEntity(cam, 3.0f, 119.0f, 0.0f);

    // Lighting
    light[1] = CreateLight(3);
    PositionEntity(light[1], -145.0f, 100.0f, -70.0f);
    RotateEntity(light[1], 90.0f, 0.0f, 0.0f);
    LightRange(light[1], 500.0f);
    LightColor(light[1], 200, 180, 160);

    // Load / init the player character model
    int cyc  = 1;
    int c    = gamChar[0];
    if (c <= 0) { c = 1; gamChar[0] = c; }
    pChar[cyc] = c;
    pX[cyc] = -257.0f; pY[cyc] = 11.5f; pZ[cyc] = -124.0f;
    pA[cyc] = 0.0f;
    ReloadModel(cyc);

    // Shadows
    for (int limb=1; limb<LIMBS; ++limb) {
        bool doShadow = (limb==30) ||
                        (optShadows>=2 &&
                         (limb==1||(limb>=4&&limb<=6)||(limb>=17&&limb<=19)||
                          limb==32||limb==33||limb==35||limb==36));
        if (doShadow) {
            pShadow[cyc][limb] = CreateSprite();
            float sc = (limb==30) ? 13.0f : ((limb==6||limb==19) ? 8.0f : 10.0f);
            ScaleSprite(pShadow[cyc][limb], sc, sc);
            RotateEntity(pShadow[cyc][limb], 90.0f, 0.0f, 0.0f);
            SpriteViewMode(pShadow[cyc][limb], 2);
            float alpha = (optShadows==2&&(limb==30||limb==6||limb==19)) ? 0.2f :
                          (optShadows==1) ? 0.5f : 0.1f;
            EntityAlpha(pShadow[cyc][limb], alpha);
            EntityColor(pShadow[cyc][limb], 10, 10, 10);
        }
    }

    // --- Main loop ---
    int focIdx = 8;   // selected row
    int page   = 1;   // 1=profile, 2=appearance
    int keytim = 20;
    int go     = 0;

    // Stat limits
    gamPointLimit = 200;

    while (go == 0 && screen == 51) {
        gInput.Update();
        if (keytim > 0) keytim--;

        // ---- Navigation ----
        if (keytim == 0) {
            int maxFoc = (page==1) ? 8 : 8;

            // D-pad up/down
            if (gInput.Pressed(SCE_CTRL_UP) || gInput.LeftY() < -0.5f)
                { focIdx--; ProduceSound(0,sMenuSelect,100); keytim=6; }
            if (gInput.Pressed(SCE_CTRL_DOWN) || gInput.LeftY() > 0.5f)
                { focIdx++; ProduceSound(0,sMenuSelect,100); keytim=6; }

            focIdx = std::max(1, std::min(focIdx, maxFoc));

            // Left/right to change values
            bool goLeft  = gInput.LeftX() < -0.5f || gInput.Held(SCE_CTRL_LEFT);
            bool goRight = gInput.LeftX() >  0.5f || gInput.Held(SCE_CTRL_RIGHT);

            if (page == 1) {
                // ---------- PROFILE PAGE ----------
                gamPoints = gamPointLimit - charStrength[c] - charAgility[c]
                            - charIntelligence[c];
                if (gamPoints < 0) gamPoints = 0;

                if (goLeft  && keytim==0) {
                    if (focIdx==2) charHeight[c]--;
                    if (focIdx==3) charStrength[c]--;
                    if (focIdx==4) charAgility[c]--;
                    if (focIdx==5) charIntelligence[c]--;
                    if (focIdx==6) charCrime[c]--;
                    ProduceSound(0,sMenuBrowse,100); keytim=3;
                }
                if (goRight && keytim==0) {
                    if (focIdx==2) charHeight[c]++;
                    if (focIdx==3 && gamPoints>0) charStrength[c]++;
                    if (focIdx==4 && gamPoints>0) charAgility[c]++;
                    if (focIdx==5 && gamPoints>0) charIntelligence[c]++;
                    if (focIdx==6) charCrime[c]++;
                    ProduceSound(0,sMenuBrowse,100); keytim=3;
                }
                // Clamp
                charHeight[c]       = std::max(5,  std::min(24,  charHeight[c]));
                charStrength[c]     = std::max(30, std::min(99,  charStrength[c]));
                charAgility[c]      = std::max(30, std::min(99,  charAgility[c]));
                charIntelligence[c] = std::max(30, std::min(99,  charIntelligence[c]));
                charCrime[c]        = std::max(1,  std::min(15,  charCrime[c]));
                charReputation[c]   = 50 + charCrime[c]*2;
                charSentence[c]     = 30 + charCrime[c]*2;

            } else {
                // ---------- APPEARANCE PAGE ----------
                int oldModel   = charModel[c];
                int oldCostume = charCostume[c];
                int oldHair    = charHairStyle[c];

                if (goLeft  && keytim==0) {
                    if (focIdx==1) charHairStyle[c]--;
                    if (focIdx==2) charHair[c]--;
                    if (focIdx==3) charFace[c]--;
                    if (focIdx==4) charSpecs[c]--;
                    if (focIdx==5) charModel[c]--;
                    if (focIdx==6) charCostume[c]--;
                    if (focIdx==7) charAccessory[c]--;
                    if (focIdx==8) charGang[c]--;
                    ProduceSound(0,sMenuBrowse,100); keytim=5;
                }
                if (goRight && keytim==0) {
                    if (focIdx==1) charHairStyle[c]++;
                    if (focIdx==2) charHair[c]++;
                    if (focIdx==3) charFace[c]++;
                    if (focIdx==4) charSpecs[c]++;
                    if (focIdx==5) charModel[c]++;
                    if (focIdx==6) charCostume[c]++;
                    if (focIdx==7) charAccessory[c]++;
                    if (focIdx==8) charGang[c]++;
                    ProduceSound(0,sMenuBrowse,100); keytim=5;
                }
                // Clamp appearance values
                charHairStyle[c] = std::max(0, std::min(31, charHairStyle[c]));
                charHair[c]      = std::max(1, std::min(20, charHair[c]));
                charFace[c]      = std::max(1, std::min(80, charFace[c]));
                charSpecs[c]     = std::max(0, std::min(4,  charSpecs[c]));
                charModel[c]     = std::max(1, std::min(20, charModel[c]));
                charCostume[c]   = std::max(0, std::min(8,  charCostume[c]));
                charAccessory[c] = std::max(0, std::min(7,  charAccessory[c]));
                charGang[c]      = std::max(0, std::min(5,  charGang[c]));

                // Reload model if body type changed
                if (charModel[c] != oldModel) ReloadModel(cyc);
                // Reapply appearance if style/costume changed
                if (charHairStyle[c]!=oldHair||charCostume[c]!=oldCostume)
                    ApplyCostume(cyc);
                else
                    ApplyCostume(cyc);  // always refresh
            }

            // Cross = confirm/next page
            if (gInput.Pressed(SCE_CTRL_CROSS) && keytim==0) {
                if (focIdx == 7 && page == 1) {
                    page = 2; focIdx = 1;
                    ProduceSound(0,sMenuGo,100); keytim=10;
                } else if (focIdx == 8) {
                    go = 1;  // done
                    ProduceSound(0,sMenuGo,100);
                }
            }
            // Circle = back
            if (gInput.Pressed(SCE_CTRL_CIRCLE)) {
                if (page == 2) { page = 1; focIdx = 7; keytim = 10; }
                else           { go = -1; }
            }
        }

        // ---- Animate character ----
        if (p[cyc]) {
            Animate(p[cyc], 1, RndF(0.1f,0.3f), pSeq[cyc][1], 10.0f);
            // Slow rotation for presentation
            pA[cyc] = CleanAngle(pA[cyc] + 0.3f);
            RotateEntity(p[cyc], 0.0f, pA[cyc], 0.0f);
        }

        // ---- Render 3D ----
        glClearColor(0.08f,0.06f,0.10f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        R3D::RenderWorld();

        // ---- 2D UI overlay ----
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrthof(0,VITA_SCREEN_W,VITA_SCREEN_H,0,-1,1);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        // Title
        SetColor(255,220,60);
        DrawText((page==1)?"CREATE CHARACTER – PROFILE":"CREATE CHARACTER – APPEARANCE",
                 VITA_SCREEN_W/2, 18, true, false);

        // Page indicator
        SetColor(140,140,140);
        DrawText("Page "+Str(page)+"/2  (X=next/confirm  O=back)", VITA_SCREEN_W/2, 40, true, false);

        int col1=100, col2=340, rowH=28, rowStart=80;

        if (page == 1) {
            // Row 1: Name
            SetColor(200,200,200);
            DrawText("Name:", col1, rowStart, false, false);
            bool nameSel=(focIdx==1);
            SetColor(nameSel?255:200, nameSel?220:200, nameSel?60:200);
            DrawText(charName[c], col2, rowStart, false, false);

            // Row 2–6: Stats
            static const char* pLabels[]={"Height","Strength","Agility","Intelligence","Crime"};
            for (int i=0;i<5;++i) {
                int row = rowStart + (i+1)*rowH;
                bool sel = (focIdx == i+2);
                int val=0;
                if (i==0) val=charHeight[c];
                if (i==1) val=charStrength[c];
                if (i==2) val=charAgility[c];
                if (i==3) val=charIntelligence[c];
                if (i==4) val=charCrime[c];
                DrawSelector(col2, row, pLabels[i], Str(val), sel);
            }

            // Points remaining
            SetColor(255,200,80);
            DrawText("Points remaining: "+Str(gamPoints), col1, rowStart+6*rowH, false, false);

            // Row 7: Next page
            DrawSelector(col2, rowStart+7*rowH, "Continue", "Appearance →", focIdx==7);
            // Row 8: Accept
            DrawSelector(col2, rowStart+8*rowH, "Accept", "Begin Game", focIdx==8);

        } else {
            // Appearance page
            static const char* aLabels[]={"Hair Style","Hair Colour","Face","Eyewear",
                                           "Build","Costume","Accessory","Gang"};
            int aVals[]={charHairStyle[c],charHair[c],charFace[c],charSpecs[c],
                         charModel[c],charCostume[c],charAccessory[c],charGang[c]};
            for (int i=0;i<8;++i) {
                int row = rowStart + i*rowH;
                bool sel = (focIdx == i+1);
                DrawSelector(col2, row, aLabels[i], Str(aVals[i]), sel);
            }
        }

        // Bottom hint
        SetColor(120,120,120);
        DrawText("Stick/D-pad: navigate   L/R: change value", VITA_SCREEN_W/2, VITA_SCREEN_H-20, true, false);

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        vglSwapBuffers(GL_FALSE);
    }

    // Cleanup
    FreeEntity(edWorld);
    FreeEntity(cam);    cam=0;
    FreeEntity(light[1]); light[1]=0;
    for (int limb=1;limb<LIMBS;++limb)
        if (pShadow[cyc][limb]) { FreeEntity(pShadow[cyc][limb]); pShadow[cyc][limb]=0; }
    if (p[cyc]) { FreeEntity(p[cyc]); p[cyc]=0; }

    if (go == 1) {
        // Start the game with the created character
        gamChar[slot] = c;
        charSentence[c] = 30 + charCrime[c]*2;
        charHealth[c]   = charStrength[c] * 10;
        charHP[c]       = charHealth[c];
        gamLocation[slot] = 1;  // start in North Block
        gamMoney[slot]  = 500;
        screen = 50;  // → Gameplay
    } else {
        screen = 1;   // → Main Menu
    }
}
