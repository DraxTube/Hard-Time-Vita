// =============================================================================
//  functions.cpp  –  Universal helper functions  (converted from Functions.bb)
// =============================================================================
#include "functions.h"
#include "debug_log.h"
#include "values.h"
#include "players.h"
#include "costume.h"
#include "world_props.h"
#include "moves.h"
#include "render3d.h"
#include "../third_party/vita_font.h"
#include <vitaGL.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <string>

// ---------------------------------------------------------------------------
//  2D rendering state
// ---------------------------------------------------------------------------
static int   s_drawR = 255, s_drawG = 255, s_drawB = 255;

// ---------------------------------------------------------------------------
//  Sound
// ---------------------------------------------------------------------------
void ProduceSound(Handle entity, BBSound* sound, int pitch, float vol) {
    if (!sound) return;
    // Fluctuate pitch (mirror original Blitz3D code)
    int range1 = pitch - (pitch / 8);
    int range2 = pitch + (pitch / 16);
    int pitcher = Rnd(range1, range2);

    if (vol == 0.0f) vol = RndF(0.4f, 1.2f);

    if (pitch > 0) SoundPitch(sound, pitcher);
    SoundVolume(sound, vol);
    EmitSound(sound, entity);
    if (pitch > 0) SoundPitch(sound, pitch);
    SoundVolume(sound, 1.0f);
}

// ---------------------------------------------------------------------------
//  2D drawing
// ---------------------------------------------------------------------------
void SetColor(int r, int g, int b) {
    s_drawR = r; s_drawG = g; s_drawB = b;
    VitaFont::set_color(r/255.f, g/255.f, b/255.f);
    glColor3ub((GLubyte)r, (GLubyte)g, (GLubyte)b);
}

void DrawText(const BBString& text, int x, int y, bool centreH, bool centreV) {
    VitaFont::set_color(s_drawR/255.f, s_drawG/255.f, s_drawB/255.f);
    VitaFont::draw_text(text, x, y, centreH, centreV);
}

void Outline(const BBString& text, int x, int y,
             int r1, int g1, int b1, int r2, int g2, int b2) {
    if (r1 != r2 || g1 != g2 || b1 != b2) {
        SetColor(r1, g1, b1);
        DrawText(text, x+2, y+2, true, true);
        DrawText(text, x+1, y,   true, true);
        DrawText(text, x-1, y,   true, true);
        DrawText(text, x,   y+1, true, true);
        DrawText(text, x,   y-1, true, true);
    }
    SetColor(r2, g2, b2);
    DrawText(text, x, y, true, true);
}

void OutlineStraight(const BBString& text, int x, int y,
                     int r1, int g1, int b1, int r2, int g2, int b2) {
    if (r1 != r2 || g1 != g2 || b1 != b2) {
        SetColor(r1, g1, b1);
        DrawText(text, x+2, y+2, false, true);
        DrawText(text, x+1, y,   false, true);
        DrawText(text, x-1, y,   false, true);
        DrawText(text, x,   y+1, false, true);
        DrawText(text, x,   y-1, false, true);
    }
    SetColor(r2, g2, b2);
    DrawText(text, x, y, false, true);
}

void DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b) {
    // Draw bold outline in black, then coloured line
    glLineWidth(3.0f);
    glColor3ub(0, 0, 0);
    glBegin(GL_LINES);
      glVertex2f(x1-1, y1); glVertex2f(x2-1, y2);
      glVertex2f(x1+1, y1); glVertex2f(x2+1, y2);
      glVertex2f(x1, y1-1); glVertex2f(x2, y2-1);
      glVertex2f(x1, y1+1); glVertex2f(x2, y2+1);
    glEnd();
    glColor3ub(r, g, b);
    glBegin(GL_LINES);
      glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
    glLineWidth(1.0f);
}

void DrawRect(int x, int y, int w, int h, bool filled) {
    if (filled) {
        glBegin(GL_QUADS);
          glVertex2f(x,   y);
          glVertex2f(x+w, y);
          glVertex2f(x+w, y+h);
          glVertex2f(x,   y+h);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
          glVertex2f(x,   y);
          glVertex2f(x+w, y);
          glVertex2f(x+w, y+h);
          glVertex2f(x,   y+h);
        glEnd();
    }
}

void DrawOval(int x, int y, int w, int h, bool filled) {
    float cx = x + w * 0.5f, cy = y + h * 0.5f;
    float rx = w * 0.5f, ry = h * 0.5f;
    int   segs = 32;
    glBegin(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
    if (filled) glVertex2f(cx, cy);
    for (int i = 0; i <= segs; ++i) {
        float a = (float)i / segs * 2.0f * M_PI;
        glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a));
    }
    glEnd();
}

// ---------------------------------------------------------------------------
//  Math helpers
// ---------------------------------------------------------------------------
BBString GetHeight(int value) {
    int feet   = value / 12;
    int inches = value - (feet * 12);
    return std::to_string(feet + 5) + "'" + std::to_string(inches) + "''";
}

BBString GetFigure(int value) {
    bool minus = false;
    if (value < 0) { minus = true; value = -value; }
    int millions  = value / 1000000;
    int thousands = (value - millions * 1000000) / 1000;
    int hundreds  = value - (millions * 1000000 + thousands * 1000);
    // piece together with apostrophe separators
    char buf[64];
    if (millions > 0)
        snprintf(buf, sizeof(buf), "%d'%03d'%03d", millions, thousands, hundreds);
    else if (thousands > 0)
        snprintf(buf, sizeof(buf), "%d'%03d", thousands, hundreds);
    else
        snprintf(buf, sizeof(buf), "%d", hundreds);
    return minus ? (BBString("-") + buf) : BBString(buf);
}

BBString Dig(int value, int degree) {
    char buf[16];
    if (degree == 100) snprintf(buf, sizeof(buf), "%03d", value);
    else if (degree == 10) snprintf(buf, sizeof(buf), "%02d", value);
    else snprintf(buf, sizeof(buf), "%d", value);
    return BBString(buf);
}

int RoundOff(int value, int degree) {
    return (value / degree) * degree;
}

int Reached(float curr, float dest, int range) {
    return (curr > dest - range && curr < dest + range) ? 1 : 0;
}

int ReachedCord(float currX, float currZ, float destX, float destZ, int range) {
    return (Reached(currX, destX, range) && Reached(currZ, destZ, range)) ? 1 : 0;
}

float CleanAngle(float angle) {
    int its = 0;
    while ((angle < 0.0f || angle > 360.0f) && its < 100) {
        if (angle < 0.0f)   angle += 360.0f;
        if (angle > 360.0f) angle -= 360.0f;
        its++;
    }
    return angle;
}

float DiffAngle(float a1, float a2) {
    float diff = a2 - a1;
    return CleanAngle(diff);
}

// ---------------------------------------------------------------------------
//  New helpers ported from Functions.bb
// ---------------------------------------------------------------------------

float ReachAngle(float sA, float tA, float speed) {
    sA = CleanAngle(sA);
    tA = CleanAngle(tA);
    
    // get negative route
    int neg = 0;
    float checkA = sA;
    do {
        neg++;
        checkA -= 1.0f;
        if (checkA < 0.0f) checkA = 360.0f;
    } while (!(checkA >= tA - 1.0f && checkA <= tA + 1.0f) && neg < 360);
    
    // get positive route
    int pos = 0;
    checkA = sA;
    do {
        pos++;
        checkA += 1.0f;
        if (checkA > 360.0f) checkA = 0.0f;
    } while (!(checkA >= tA - 1.0f && checkA <= tA + 1.0f) && pos < 360);
    
    // return shortest route
    return (neg < pos) ? -speed : speed;
}

int SatisfiedAngle(float sA, float tA, int range) {
    int value = 0;
    // scan clockwise
    float angler = sA;
    for (int count = 1; count <= range; ++count) {
        if (angler >= tA - 1.0f && angler <= tA + 1.0f) value = 1;
        angler += 1.0f;
        if (angler > 360.0f) angler = 0.0f;
    }
    // scan counter-clockwise
    angler = sA;
    for (int count = 1; count <= range; ++count) {
        if (angler >= tA - 1.0f && angler <= tA + 1.0f) value = 1;
        angler -= 1.0f;
        if (angler < 0.0f) angler = 360.0f;
    }
    return value;
}

float MakePositive(float value) {
    return (value < 0.0f) ? (value - (value * 2.0f)) : value;
}

float GetDiff(float source, float dest) {
    float diff = dest - source;
    if (diff < 0.0f) diff = MakePositive(diff);
    return diff;
}

float GetCentre(float source, float dest) {
    return source + (GetDiff(source, dest) / 2.0f);
}

float GetDistance(float sourceX, float sourceZ, float destX, float destZ) {
    float diffX = GetDiff(sourceX, destX);
    float diffZ = GetDiff(sourceZ, destZ);
    return (diffX > diffZ) ? diffX : diffZ;
}

float HighestValue(float valueA, float valueB) {
    return (valueB > valueA) ? valueB : valueA;
}

float LowestValue(float valueA, float valueB) {
    return (valueB < valueA) ? valueB : valueA;
}

void GetSmoothSpeeds(float x, float tX, float y, float tY, float z, float tZ, int factor) {
    extern float speedX, speedY, speedZ; // Need to set globals
    float diffX = GetDiff(x, tX);
    float lead  = diffX;
    int leader  = 1;
    
    float diffY = GetDiff(y, tY);
    if (diffY > lead) { lead = diffY; leader = 2; }
    
    float diffZ = GetDiff(z, tZ);
    if (diffZ > lead) { lead = diffZ; leader = 3; }
    
    float anchor = lead / (float)factor;
    
    if (leader == 1) speedX = anchor; else speedX = anchor * (diffX / lead);
    if (leader == 2) speedY = anchor; else speedY = anchor * (diffY / lead);
    if (leader == 3) speedZ = anchor; else speedZ = anchor * (diffZ / lead);
}

float PercentOf(float valueA, float percent) {
    return (valueA / 100.0f) * percent;
}

float GetPercent(float valueA, float valueB) {
    if (valueB == 0.0f) return 0.0f;
    return (valueA / valueB) * 100.0f;
}

// InProximity depends on player arrays, so must be implemented locally or include players.h
// Since we are in functions.cpp, we include players.h
#include "players.h"
int InProximity(int cyc, int v, float range) {
    int value = 0;
    if (pX[v] > pX[cyc] - range && pX[v] < pX[cyc] + range &&
        pZ[v] > pZ[cyc] - range && pZ[v] < pZ[cyc] + range) {
        value = 1;
    }
    return value;
}

// ---------------------------------------------------------------------------
//  Loader screen
// ---------------------------------------------------------------------------
void Loader(const BBString& title, const BBString& message) {
    DebugLog("[LOADER] start: %s | %s", title.c_str(), message.c_str());

    // Set up 2D orthographic projection (required before any GL draw calls)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, VITA_SCREEN_W, VITA_SCREEN_H, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DebugLog("[LOADER] glClear done");

    SetColor(255, 255, 255);
    DebugLog("[LOADER] SetColor done");
    DrawText(title,   VITA_SCREEN_W / 2, VITA_SCREEN_H / 2 - 20, true, true);
    DebugLog("[LOADER] DrawText title done");
    DrawText(message, VITA_SCREEN_W / 2, VITA_SCREEN_H / 2 + 20, true, true);
    DebugLog("[LOADER] DrawText message done");

    vglSwapBuffers(GL_FALSE);
    DebugLog("[LOADER] vglSwapBuffers done");
}

// ---------------------------------------------------------------------------
//  Intro
// ---------------------------------------------------------------------------
void Intro() {
    DebugLog("[INTRO] start");
    Loader("HARD TIME", "© Mat Dickie 2007  |  PS Vita Port");
    DebugLog("[INTRO] Loader done, waiting 2s");
    int start = MilliSecs();
    while (MilliSecs() - start < 2000) {
        gInput.Update();
        if (gInput.Pressed(SCE_CTRL_CROSS) ||
            gInput.Pressed(SCE_CTRL_START)) break;
    }
    DebugLog("[INTRO] done");
}

// ---------------------------------------------------------------------------
//  Asset loading – real implementations
// ---------------------------------------------------------------------------
void LoadImages() {
    // 2D HUD images – loaded as GL textures
    const std::string BASE = "ux0:data/HardTime/Graphics/";
    // gHealth  = LoadTexture(BASE + "Health.png");    // add when assets confirmed
    // gMoney   = LoadTexture(BASE + "Money.png");
}

void LoadTextures() {
    const std::string BASE = "ux0:data/HardTime/";

    // Signs (World/Signs/Sign01-11.PNG)
    for (int i = 1; i <= 11; ++i)
        tSign[i] = (GLuint)LoadTexture(BASE + "World/Signs/Sign" + Dig(i,10) + ".PNG");

    // Block number plates (Characters/Numbers/Block01-04.PNG)
    for (int i = 1; i <= 4; ++i)
        tBlock[i] = (GLuint)LoadTexture(BASE + "Characters/Numbers/Block" + Dig(i,10) + ".PNG");

    // Cell number plates (Characters/Numbers/Cell01-20.PNG)
    for (int i = 1; i <= 20; ++i)
        tCell[i] = (GLuint)LoadTexture(BASE + "Characters/Numbers/Cell" + Dig(i,10) + ".PNG");

    // Video screens (World/Screens/Screen00-10.JPG)
    for (int i = 0; i <= 10; ++i)
        tScreen[i] = (GLuint)LoadTexture(BASE + "World/Screens/Screen" + Dig(i,10) + ".JPG");

    // Food trays (World/Sprites/Tray00-07.JPG)
    for (int i = 0; i <= 7; ++i)
        tTray[i] = (GLuint)LoadTexture(BASE + "World/Sprites/Tray" + Dig(i,10) + ".JPG");

    // World sprites
    tFence  = (GLuint)LoadTexture(BASE + "World/Sprites/Fence.PNG");
    tNet    = (GLuint)LoadTexture(BASE + "World/Sprites/Net.PNG");
    tShower = (GLuint)LoadTexture(BASE + "World/Sprites/Shower.PNG");

    // Weapon textures (Weapons/Textures/Machine.PNG, Pistol.PNG)
    tMachine = (GLuint)LoadTexture(BASE + "Weapons/Textures/Machine.PNG");
    tPistol  = (GLuint)LoadTexture(BASE + "Weapons/Textures/Pistol.PNG");

    // Facial expressions (Characters/Expressions/)
    tEars = (GLuint)LoadTexture(BASE + "Characters/Expressions/Ears.JPG");
    for (int i = 1; i <= 3; ++i)
        tEyes[i] = (GLuint)LoadTexture(BASE + "Characters/Expressions/Eyes0" + std::to_string(i) + ".JPG");
    for (int i = 0; i <= 5; ++i)
        tMouth[i] = (GLuint)LoadTexture(BASE + "Characters/Expressions/Mouth0" + std::to_string(i) + ".JPG");

    // Costume variations
    tShaved = (GLuint)LoadTexture(BASE + "Characters/Hair/Shaved.JPG");
    for (int i = 1; i <= 3; ++i)
        tSpecs[i] = (GLuint)LoadTexture(BASE + "Characters/Specs/Specs" + Dig(i,10) + ".JPG");

    // Hair (Characters/Hair/Hair01-09.PNG — uses dynamic count)
    for (int i = 1; i <= no_hairs; ++i)
        tHair[i] = (GLuint)LoadTexture(BASE + "Characters/Hair/Hair" + Dig(i,10) + ".PNG");

    // Faces (Characters/Faces/Face01-66.JPG)
    for (int i = 1; i <= no_faces; ++i)
        tFace[i] = (GLuint)LoadTexture(BASE + "Characters/Faces/Face" + Dig(i,10) + ".JPG");

    // Bodies (Characters/Bodies/Body01-10.JPG)
    for (int i = 1; i <= no_bodies; ++i)
        tBody[i] = (GLuint)LoadTexture(BASE + "Characters/Bodies/Body" + Dig(i,10) + ".JPG");

    // Arms (Characters/Arms/Arm01-12.JPG)
    for (int i = 1; i <= no_arms; ++i)
        tArm[i] = (GLuint)LoadTexture(BASE + "Characters/Arms/Arm" + Dig(i,10) + ".JPG");

    // Legs (Characters/Legs/Legs01-06.JPG)
    for (int i = 1; i <= no_legs; ++i)
        tLegs[i] = (GLuint)LoadTexture(BASE + "Characters/Legs/Legs" + Dig(i,10) + ".JPG");

    // Racial shading (Characters/Shading/Body01-04.PNG, Arm01-08.PNG)
    for (int i = 1; i <= 4; ++i)
        tBodyShade[i] = (GLuint)LoadTexture(BASE + "Characters/Shading/Body" + Dig(i,10) + ".PNG");
    for (int i = 1; i <= 8; ++i)
        tArmShade[i] = (GLuint)LoadTexture(BASE + "Characters/Shading/Arm" + Dig(i,10) + ".PNG");

    // Scarring (Characters/Scarring/)
    for (int i = 0; i <= 4; ++i)
        tFaceScar[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Face" + Dig(i,10) + ".JPG");
    for (int i = 0; i <= 4; ++i)
        tBodyScar[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Body" + Dig(i,10) + ".JPG");
    for (int i = 0; i <= 4; ++i)
        tArmScar[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Arm" + Dig(i,10) + ".JPG");
    for (int i = 0; i <= 4; ++i)
        tLegScar[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Legs" + Dig(i,10) + ".JPG");

    // Wounds (Characters/Scarring/Wounds/)
    tSeverEars = (GLuint)LoadTexture(BASE + "Characters/Scarring/Wounds/Ears.JPG");
    for (int i = 1; i <= 3; ++i)
        tSeverBody[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Wounds/Body" + Dig(i,10) + ".JPG");
    for (int i = 1; i <= 3; ++i)
        tSeverArm[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Wounds/Arm" + Dig(i,10) + ".JPG");
    for (int i = 1; i <= 3; ++i)
        tSeverLegs[i] = (GLuint)LoadTexture(BASE + "Characters/Scarring/Wounds/Legs" + Dig(i,10) + ".JPG");

    // Tattoos (Characters/Tattoos/)
    for (int i = 1; i <= 6; ++i) {
        tTattooBody[i]   = (GLuint)LoadTexture(BASE + "Characters/Tattoos/Body" + Dig(i,10) + ".JPG");
        tTattooVest[i]   = (GLuint)LoadTexture(BASE + "Characters/Tattoos/Vest" + Dig(i,10) + ".JPG");
        tTattooArm[i]    = (GLuint)LoadTexture(BASE + "Characters/Tattoos/Arm" + Dig(i,10) + ".JPG");
        tTattooTee[i]    = (GLuint)LoadTexture(BASE + "Characters/Tattoos/Tee" + Dig(i,10) + ".JPG");
        tTattooSleeve[i] = (GLuint)LoadTexture(BASE + "Characters/Tattoos/Sleeve" + Dig(i,10) + ".JPG");
    }
}
