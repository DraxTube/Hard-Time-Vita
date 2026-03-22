#pragma once
// =============================================================================
//  functions.h  –  Universal helper functions  (converted from Functions.bb)
// =============================================================================

#include "blitz_compat.h"
#include "render3d.h"

// ---------------------------------------------------------------------------
//  Sound
// ---------------------------------------------------------------------------
void ProduceSound(Handle entity, BBSound* sound, int pitch, float vol = 0.0f);

// ---------------------------------------------------------------------------
//  2D Drawing helpers  (vitaGL immediate-mode 2D overlay)
//  These wrap vglBegin/vglEnd + glRasterPos / glBitmap equivalents.
//  In the Vita port we use an orthographic projection for all 2D rendering.
// ---------------------------------------------------------------------------
void Outline(const BBString& text, int x, int y,
             int r1, int g1, int b1,
             int r2, int g2, int b2);

void OutlineStraight(const BBString& text, int x, int y,
                     int r1, int g1, int b1,
                     int r2, int g2, int b2);

void DrawLine(int startX, int startY, int endX, int endY, int r, int g, int b);

// 2D text rendering using vitaGL / bitmap font
void DrawText(const BBString& text, int x, int y, bool centreH = false, bool centreV = false);
void SetColor(int r, int g, int b);
void DrawRect(int x, int y, int w, int h, bool filled = true);
void DrawOval(int x, int y, int w, int h, bool filled = true);

// ---------------------------------------------------------------------------
//  Math helpers (match original Function names)
// ---------------------------------------------------------------------------
BBString GetHeight(int value);
BBString GetFigure(int value);
BBString Dig(int value, int degree);
int      RoundOff(int value, int degree);
int      Reached(float curr, float dest, int range);
int      ReachedCord(float currX, float currZ, float destX, float destZ, int range);
float    CleanAngle(float angle);

// New helpers ported from Functions.bb
float    ReachAngle(float sA, float tA, float speed);
int      SatisfiedAngle(float sA, float tA, int range);
float    DiffAngle(float a1, float a2);
float    GetDiff(float source, float dest);
float    GetCentre(float source, float dest);
float    GetDistance(float sourceX, float sourceZ, float destX, float destZ);
float    HighestValue(float valueA, float valueB);
float    LowestValue(float valueA, float valueB);
void     GetSmoothSpeeds(float x, float tX, float y, float tY, float z, float tZ, int factor);
float    MakePositive(float value);
float    PercentOf(float valueA, float percent);
float    GetPercent(float valueA, float valueB);
int      InProximity(int cyc, int v, float range);

// ---------------------------------------------------------------------------
//  Loading screen
// ---------------------------------------------------------------------------
void Loader(const BBString& title, const BBString& message);
void Intro();

// ---------------------------------------------------------------------------
//  Image / texture loading
// ---------------------------------------------------------------------------
void LoadImages();
void LoadTextures();
