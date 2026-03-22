// =============================================================================
//  screen_fade.cpp  –  Screen fade system
// =============================================================================
#include "screen_fade.h"
#include "players.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <vitaGL.h>
#include <algorithm>

static float s_fadeSpeed = 0.01f;

void InitFader() {
    fader     = CreateSprite();
    fadeAlpha = 0.0f;
    fadeTarget = 0.0f;
    EntityAlpha(fader, 0.0f);
    EntityColor(fader, 0, 0, 0);
    HideEntity(fader);
}

void SetFadeTarget(float target, float speed) {
    fadeTarget  = target;
    s_fadeSpeed = speed;
    if (fadeAlpha > 0.0f || target > 0.0f) ShowEntity(fader);
}

void FaderCycle() {
    if (fadeAlpha < fadeTarget) {
        fadeAlpha = std::min(fadeTarget, fadeAlpha + s_fadeSpeed);
    } else if (fadeAlpha > fadeTarget) {
        fadeAlpha = std::max(fadeTarget, fadeAlpha - s_fadeSpeed);
    }
    if (fadeAlpha <= 0.0f) HideEntity(fader);
}

void DrawFade() {
    if (fadeAlpha <= 0.001f) return;
    // Draw full-screen black quad at current alpha (2D overlay, no depth)
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
    glBegin(GL_QUADS);
        glVertex2f(0,                0);
        glVertex2f(VITA_SCREEN_W,    0);
        glVertex2f(VITA_SCREEN_W,    VITA_SCREEN_H);
        glVertex2f(0,                VITA_SCREEN_H);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1,1,1,1);
}
