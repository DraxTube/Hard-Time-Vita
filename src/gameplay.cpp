// =============================================================================
//  gameplay.cpp  –  Main 3D gameplay loop  (converted from Gameplay.bb)
//
//  Replaces Blitz3D:
//    RenderWorld()  → R3D::RenderWorld() via vitaGL
//    Flip()         → vglSwapBuffers()
//    Timer/ms       → MilliSecs()
// =============================================================================
#include "gameplay.h"
#include "values.h"
#include "players.h"
#include "functions.h"
#include "ai.h"
#include "anims.h"
#include "moves.h"
#include "particles.h"
#include "crimes.h"
#include "promos.h"
#include "missions.h"
#include "bullets.h"
#include "world_props.h"
#include "screen_fade.h"
#include "costume.h"
#include "texts.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <vitaGL.h>
#include <algorithm>

// Forward declarations
void RenderHUD();
void ManageCamera();

// ---------------------------------------------------------------------------
//  Gameplay  –  50. main 3D scene
// ---------------------------------------------------------------------------
void Gameplay() {
    // Adjust to native Vita resolution (no resolution switching needed)
    // Original: ChangeResolution(optRes, 1) – on Vita we're always 960×544

    // Load the current location world mesh
    Loader("Please Wait", "Loading " + textLocation[gamLocation[slot]]);
    LoadWorld();

    // Load atmosphere / ambient sound
    Loader("Please Wait", "Loading Atmosphere");
    if (sAtmos) PlaySound(sAtmos);

    // Set up 3D scene
    SetCollisions();
    LoadMoveSounds();
    LoadParticles();
    LoadPools();
    InitBullets();
    InitWorldProps();
    InitFader();
    Loader("Please Wait", "Loading Players");
    LoadPlayers();
    Loader("Please Wait", "Loading Weapons");
    LoadWeapons();

    // Camera setup (mirrors original Blitz3D camera init)
    cam = CreateCamera();
    CameraRange(cam, 1.0f, 1500.0f);
    CameraFOV(cam, 60.0f);
    CameraClsColor(cam, 0, 0, 0);

    // Main directional light
    light1 = CreateLight(2);
    RotateEntity(light1, 45.0f, 45.0f, 0.0f);
    LightColor(light1, 255, 255, 245);

    // Fog (mirrors optFog setting)
    if (optFog) {
        FogMode(1);
        FogColor(180, 180, 200);
        FogRange(200.0f, 900.0f);
    } else {
        FogMode(0);
    }

    // -----------------------------------------------------------------------
    //  MAIN GAMEPLAY LOOP
    // -----------------------------------------------------------------------
    gotim = 0;
    gamPause = 0;

    int prevMs = MilliSecs();

    while (screen == 50) {
        // --- Timing ---
        int nowMs  = MilliSecs();
        int deltaMs = nowMs - prevMs;
        prevMs = nowMs;
        gotim += deltaMs;

        // --- Input ---
        gInput.Update();

        // Pause: Start button (replaces Escape key)
        if (gInput.Pressed(SCE_CTRL_START)) {
            gamPause = 1 - gamPause;
        }

        if (!gamPause) {
            // --- Update all players (input + AI) ---
            for (int cyc = 1; cyc <= no_plays; ++cyc) {
                GetInput(cyc);
            }

            // --- Move / physics ---
            MovePlayers();
            WeaponCycle();
            PlayerCycle();

            // --- Animation ---
            AnimatePlayers();

            // --- Game logic ---
            CrimeCycle();
            PromoCycle();
            MissionCycle();
            ParticleCycle();
            PoolCycle();
            ExplosionCycle();
            BulletCycle();
            DoorCycle();
            PropCycle();
            FaderCycle();

            // --- Camera ---
            ManageCamera();

            // --- In-game time ---
            if (gotim >= gamSpeed[slot]) {
                gotim -= gamSpeed[slot];
                gamSecs[slot]++;
                
                // Track minute change
                bool minuteTick = false;
                if (gamSecs[slot] >= 60) { gamSecs[slot] = 0; gamMins[slot]++; minuteTick = true; }
                
                // Track hour change
                bool hourTick = false;
                if (gamMins[slot] >= 60) { gamMins[slot] = 0; gamHours[slot]++; hourTick = true; }
                
                // Day rollover
                if (gamHours[slot] >= 24) {
                    gamHours[slot] = 0;
                    for (int charId = 1; charId <= no_chars; ++charId) {
                        if (charSentence[charId] > 0) charSentence[charId]--;
                    }
                }
                
                // Top of the hour events
                if (gamMins[slot] == 0 && gamSecs[slot] == 0) {
                    // Morning bell
                    if (gamHours[slot] == 7) {
                        ProduceSound(cam, sBuzzer, 22050, 1.0f);
                        for (int v = 1; v <= no_plays; ++v) {
                            if (pChar[v] != gamChar[slot]) {
                                pHealth[v] += Rnd(10, 100);
                                if (pHealth[v] > 100) pHealth[v] = 100;
                            }
                        }
                    }
                    // Evening bell
                    if (gamHours[slot] == 22) {
                        ProduceSound(cam, sBuzzer, 22050, 1.0f);
                    }
                    // Dinner bell
                    if (gamHours[slot] == 13) {
                        ProduceSound(cam, sBell, 22050, 1.0f);
                        for (int tray = 1; tray <= 50; ++tray) trayState[tray] = Rnd(1, 7);
                    }
                }
                
                // Eat from trays tracking
                if (gamHours[slot] != 13 && gamMins[slot] == 0 && gamSecs[slot] == 0) {
                    for (int tray = 1; tray <= 50; ++tray) {
                        if (tray != pSeat[gamPlayer[slot]] || gamLocation[slot] != 8) {
                            if (trayState[tray] > 0) trayState[tray]--;
                        }
                    }
                }
            }
            
            // --- Lighting properties based on time of day ---
            float ambTR = 200, ambTG = 200, ambTB = 200;
            if (gamHours[slot] >= 10 && gamHours[slot] <= 16) {
                ambTR = 200; ambTG = 200; ambTB = 200; // Day
            } else if ((gamHours[slot] >= 7 && gamHours[slot] <= 9) || (gamHours[slot] >= 17 && gamHours[slot] <= 19)) {
                ambTR = 200; ambTG = 190; ambTB = 170; // Dusk/Dawn
            } else if ((gamHours[slot] >= 20 && gamHours[slot] <= 22) || (gamHours[slot] >= 4 && gamHours[slot] <= 6)) {
                ambTR = 150; ambTG = 150; ambTB = 150; // Twilight
            } else if (gamHours[slot] >= 23 || gamHours[slot] <= 3) {
                ambTR = 80;  ambTG = 80;  ambTB = 80;  // Night
            }
            
            // Apply ambient color to directional light roughly to simulate environment changing
            LightColor(light1, ambTR, ambTG, ambTB);
            
            // --- Location Transition ---
            if (go == 1) {
                gamLocation[slot] = charLocation[gamChar[slot]];
                go = 0;
                gamPause = 1;
                break;
            }
        }

        // --- 3D Render ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        R3D::RenderWorld();

        // --- 2D HUD overlay ---
        // Switch to orthographic projection for 2D
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrthof(0, VITA_SCREEN_W, VITA_SCREEN_H, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        RenderHUD();
        DrawFade();

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        // --- Flip ---
        vglSwapBuffers(GL_FALSE);

        // --- Check for exit to menu ---
        if (gInput.Held(SCE_CTRL_SELECT)) {
            screen = 1;   // return to main menu
        }
    }

    // Cleanup 3D scene entities
    if (world)  FreeEntity(world);
    if (cam)    FreeEntity(cam);
    if (light1) FreeEntity(light1);
    // Players and weapons freed in their own cleanup functions
}

// ---------------------------------------------------------------------------
//  Camera management  (simplified – mirrors original camera follow code)
// ---------------------------------------------------------------------------
void ManageCamera() {
    if (camFoc < 1 || camFoc > no_plays) return;

    float targetX = pX[camFoc];
    float targetY = pY[camFoc] + 60.0f;
    float targetZ = pZ[camFoc] - 180.0f;

    // Smooth follow with linear interpolation (Blitz3D used direct positioning)
    float cx = EntityX(cam);
    float cy = EntityY(cam);
    float cz = EntityZ(cam);

    float tween = 0.12f;
    PositionEntity(cam,
        cx + (targetX - cx) * tween,
        cy + (targetY - cy) * tween,
        cz + (targetZ - cz) * tween);

    PointEntity(cam, pPivot[camFoc], 0.0f);
}

// ---------------------------------------------------------------------------
//  HUD rendering  (ported from Gameplay.bb 2D overlay section)
// ---------------------------------------------------------------------------
void RenderHUD() {
    int plr = gamPlayer[slot];

    // --- Pause overlay ---
    if (gamPause) {
        SetColor(0,0,0);
        DrawRect(VITA_SCREEN_W/2-120, VITA_SCREEN_H/2-80, 240, 160);
        SetColor(255,220,80);
        DrawText("PAUSED", VITA_SCREEN_W/2, VITA_SCREEN_H/2-30, true, true);
        SetColor(200,200,200);
        DrawText("X  Resume", VITA_SCREEN_W/2, VITA_SCREEN_H/2+10, true, true);
        DrawText("O  Menu",   VITA_SCREEN_W/2, VITA_SCREEN_H/2+40, true, true);
        if (gInput.Pressed(SCE_CTRL_CIRCLE)) screen = 1;
        return;
    }

    if (plr < 1 || plr > no_plays) return;

    int pChar_ = pChar[plr];

    // --- Health bar ---
    {
        int hp    = pHealth[plr];
        int maxhp = pHealthLimit[plr];
        if (maxhp < 1) maxhp = 1;
        int bx=20, by=VITA_SCREEN_H-50, bw=160, bh=12;
        SetColor(60,0,0);   DrawRect(bx, by, bw, bh);
        int fill = (hp * bw) / maxhp;
        fill = std::max(0, std::min(fill, bw));
        // Colour: green→yellow→red
        int r = 255 - (fill*255/bw);
        int g = (fill*255/bw);
        SetColor(r, g, 0);  DrawRect(bx, by, fill, bh);
        SetColor(255,255,255); DrawText("HP", bx-22, by+2, false, false);
    }

    // --- Stat bars (Strength / Agility / Intelligence / Reputation) ---
    const char* statLabels[] = {"STR","AGI","INT","REP"};
    int statVals[] = {
        charStrength[pChar_], charAgility[pChar_],
        charIntelligence[pChar_], charReputation[pChar_]
    };
    for (int i=0;i<4;++i) {
        int bx=20, by=VITA_SCREEN_H-90-(i*18), bw=100, bh=8;
        SetColor(30,30,60); DrawRect(bx, by, bw, bh);
        SetColor(60+i*30, 100, 200-i*20);
        DrawRect(bx, by, statVals[i]*bw/100, bh);
        SetColor(200,200,200);
        DrawText(BBString(statLabels[i]), bx-28, by, false, false);
    }

    // --- Money ---
    {
        SetColor(255,220,50);
        DrawText("$"+GetFigure(gamMoney[slot]), 20, 20, false, false);
    }

    // --- Clock ---
    {
        SetColor(220,220,180);
        BBString clock = Dig(gamHours[slot],10) + ":" + Dig(gamMins[slot],10);
        DrawText(clock, VITA_SCREEN_W-70, 20, false, false);
    }

    // --- Sentence ---
    if (charSentence[pChar_] > 0) {
        SetColor(255,100,100);
        DrawText("Sentence: "+Str(charSentence[pChar_])+" days",
                 VITA_SCREEN_W/2, 20, true, false);
    } else {
        SetColor(80,255,80);
        DrawText("FREE", VITA_SCREEN_W/2, 20, true, false);
    }

    // --- Warrant level ---
    if (gamWarrant[slot] > 0) {
        SetColor(255, 60, 60);
        DrawText("WANTED x"+Str(gamWarrant[slot]),
                 VITA_SCREEN_W-20, 50, false, false);
    }

    // --- Active mission ---
    if (gamMission[slot] > 0) {
        SetColor(255,200,80);
        DrawText("MISSION: deadline "+Dig(gamDeadline[slot],10)+":00",
                 VITA_SCREEN_W/2, VITA_SCREEN_H-30, true, false);
    }

    // --- Active promo subtitles ---
    if (gamPromo > 0 && promoActor[1] > 0 && promoActor[2] > 0) {
        // Draw dialogue box at bottom of screen
        SetColor(0,0,0); DrawRect(60, VITA_SCREEN_H-130, VITA_SCREEN_W-120, 90);
        SetColor(255,255,200);
        DrawText(charName[pChar[promoActor[1]]] + ":", 80, VITA_SCREEN_H-120, false, false);
        SetColor(200,200,200);

        // Option buttons when in choice stage
        if (promoStage == 1 && !optionA.empty()) {
            SetColor(100,255,100);
            DrawText("[X] " + optionA, 80, VITA_SCREEN_H-90, false, false);
            SetColor(255,100,100);
            DrawText("[O] " + optionB, 80, VITA_SCREEN_H-65, false, false);
        }
    }
}


// ---------------------------------------------------------------------------
//  Ending  –  screen 53  (ported from Credits.bb Ending())
// ---------------------------------------------------------------------------
void Ending() {
    Loader("Please Wait", "Leaving Prison");

    const std::string BASE = "ux0:data/HardTime/";
    Handle outroWorld = LoadAnimMesh(BASE + "World/Yard/Outro.3ds", 0);
    if (tSign[9] > 0) EntityTexture(FindChild(outroWorld, "Sign01"), tSign[9], 0, 2);
    if (tNet > 0)     EntityTexture(FindChild(outroWorld, "Net"), tNet);

    cam    = CreateCamera();
    light[1] = CreateLight(2);
    RotateEntity(light[1], 45.0f, 45.0f, 0.0f);
    LightColor(light[1], 240, 240, 220);

    // Load player model for the walk-out animation
    int plr = gamPlayer[slot];
    if (plr > 0 && plr <= no_plays) {
        PositionEntity(p[plr], 0.0f, 0.0f, -200.0f);
        RotateEntity(p[plr], 0.0f, 0.0f, 0.0f);
        Animate(p[plr], 1, 1.5f, pSeq[plr][13], 0.0f);  // run anim
    }

    // Camera: fixed angle looking at gate
    PositionEntity(cam, 0.0f, 80.0f, -350.0f);
    RotateEntity(cam, 15.0f, 0.0f, 0.0f);

    float walkZ = -200.0f;
    int endTim = 0;
    while (screen == 53 && endTim < 500) {
        gInput.Update();
        endTim++;

        // Walk character out through the gate
        walkZ += 1.2f;
        if (plr > 0 && plr <= no_plays)
            PositionEntity(p[plr], 0.0f, 0.0f, walkZ);

        glClearColor(0.5f,0.7f,0.9f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        R3D::RenderWorld();

        // 2D overlay
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrthof(0,VITA_SCREEN_W,VITA_SCREEN_H,0,-1,1);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        if (endTim > 200) {
            // Fade in "CONGRATULATIONS"
            float alpha = std::min(1.0f,(endTim-200)/100.0f);
            SetColor(255, (int)(220*alpha), (int)(80*alpha));
            DrawText("CONGRATULATIONS", VITA_SCREEN_W/2, VITA_SCREEN_H/2-40, true, true);
            SetColor(200,200,200);
            DrawText(charName[gamChar[slot]] + " is free!", VITA_SCREEN_W/2, VITA_SCREEN_H/2+20, true, true);
        }
        if (endTim > 400) {
            SetColor(180,180,180);
            DrawText("Press X to continue", VITA_SCREEN_W/2, VITA_SCREEN_H-60, true, false);
            if (gInput.Pressed(SCE_CTRL_CROSS)) endTim = 500;
        }

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        vglSwapBuffers(GL_FALSE);
    }

    gamEnded = 1;
    FreeEntity(outroWorld);
    FreeEntity(cam);    cam    = 0;
    FreeEntity(light[1]); light[1] = 0;
    screen = 1;  // back to main menu
}
