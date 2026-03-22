// =============================================================================
//  menus.cpp  –  Menu screens + weapon data  (converted from Menus.bb + Weapons.bb)
// =============================================================================
#include "menus.h"
#include "players.h"
#include "values.h"
#include "functions.h"
#include "costume.h"
#include "render3d.h"
#include "blitz_compat.h"
#include "data.h"
#include "moves.h"
#include <vitaGL.h>
#include <algorithm>
#include <vector>

// ---------------------------------------------------------------------------
//  Helper: draw a simple menu list and return selected index
// ---------------------------------------------------------------------------
static int SimpleMenu(const BBString& title,
                      const std::vector<BBString>& items,
                      int current) {
    glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SetColor(220, 180, 60);
    DrawText(title, VITA_SCREEN_W / 2, 60, true, false);

    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        bool sel = (i == current);
        SetColor(sel ? 255 : 180, sel ? 255 : 180, sel ? 60 : 180);
        DrawText((sel ? "> " : "  ") + items[i],
                 VITA_SCREEN_W / 2,
                 140 + i * 36,
                 true, false);
    }

    vglSwapBuffers(GL_FALSE);
    return current;
}

// ---------------------------------------------------------------------------
//  Main Menu  –  screen 1
// ---------------------------------------------------------------------------
void MainMenu() {
    std::vector<BBString> items = {"New Game", "Continue", "Options", "Credits", "Quit"};
    int sel = 0;

    while (screen == 1) {
        gInput.Update();

        if (gInput.Pressed(SCE_CTRL_DOWN) || gInput.LeftY() > 0.5f)
            sel = (sel + 1) % static_cast<int>(items.size());
        if (gInput.Pressed(SCE_CTRL_UP)   || gInput.LeftY() < -0.5f)
            sel = (sel - 1 + items.size()) % items.size();

        SimpleMenu("HARD TIME", items, sel);

        if (gInput.Pressed(SCE_CTRL_CROSS)) {
            ProduceSound(0, sMenuSelect, 100);
            switch (sel) {
                case 0: screen = 5;  break;   // slot select → new game
                case 1: screen = 5;  break;   // slot select → continue
                case 2: screen = 2;  break;   // options
                case 3: screen = 6;  break;   // credits
                case 4: screen = 0;  break;   // quit
            }
        }
        if (gInput.Pressed(SCE_CTRL_CIRCLE)) {
            ProduceSound(0, sMenuBack, 100);
        }
    }
}

// ---------------------------------------------------------------------------
//  Options  –  screen 2
// ---------------------------------------------------------------------------
void Options() {
    std::vector<BBString> items = {
        "Population: " + Str(optPopulation),
        "Fog: "        + BBString(optFog     ? "On" : "Off"),
        "Effects: "    + BBString(optFX      ? "On" : "Off"),
        "Shadows: "    + Str(optShadows),
        "Gore: "       + Str(optGore),
        "Redefine Buttons",
        "Back"
    };
    int sel = 0;

    while (screen == 2) {
        gInput.Update();
        // rebuild labels each frame so values update immediately
        items[0] = "Population: " + Str(optPopulation);
        items[1] = "Fog: "        + BBString(optFog  ? "On" : "Off");
        items[2] = "Effects: "    + BBString(optFX   ? "On" : "Off");
        items[3] = "Shadows: "    + Str(optShadows);
        items[4] = "Gore: "       + Str(optGore);

        if (gInput.Pressed(SCE_CTRL_DOWN)) sel = (sel+1) % items.size();
        if (gInput.Pressed(SCE_CTRL_UP))   sel = (sel-1+items.size()) % items.size();

        SimpleMenu("OPTIONS", items, sel);

        if (gInput.Pressed(SCE_CTRL_CROSS) || gInput.Pressed(SCE_CTRL_RIGHT)) {
            ProduceSound(0, sMenuSelect, 100);
            switch(sel) {
                case 0: optPopulation = (optPopulation >= 100) ? 20 : optPopulation + 20; break;
                case 1: optFog      = 1 - optFog;      break;
                case 2: optFX       = 1 - optFX;       break;
                case 3: optShadows  = (optShadows+1) % 3; break;
                case 4: optGore     = (optGore+1)    % 4; break;
                case 5: screen = 4; break;   // redefine gamepad
                case 6: { SaveOptions(); screen = 1; } break;
            }
        }
        if (gInput.Pressed(SCE_CTRL_CIRCLE)) {
            ProduceSound(0, sMenuBack, 100);
            SaveOptions();
            screen = 1;
        }
    }
}

// ---------------------------------------------------------------------------
//  Redefine Keys  –  screen 3  (keyboard not applicable on Vita; redirect)
// ---------------------------------------------------------------------------
void RedefineKeys() { screen = 1; }

// ---------------------------------------------------------------------------
//  Redefine Gamepad  –  screen 4
// ---------------------------------------------------------------------------
void RedefineGamepad() {
    std::vector<BBString> labels = {"Attack", "Defend", "Throw", "Pick Up"};
    std::array<int*, 4> targets = {&buttAttack, &buttDefend, &buttThrow, &buttPickUp};
    static const std::vector<std::pair<BBString, int>> vitaButtons = {
        {"Cross",    SCE_CTRL_CROSS},
        {"Circle",   SCE_CTRL_CIRCLE},
        {"Square",   SCE_CTRL_SQUARE},
        {"Triangle", SCE_CTRL_TRIANGLE},
        {"L1",       SCE_CTRL_L1},
        {"R1",       SCE_CTRL_R1},
    };

    int sel = 0;
    while (screen == 4) {
        gInput.Update();
        if (gInput.Pressed(SCE_CTRL_DOWN)) sel = (sel+1) % 4;
        if (gInput.Pressed(SCE_CTRL_UP))   sel = (sel-1+4) % 4;

        std::vector<BBString> items;
        for (int i = 0; i < 4; ++i) {
            BBString btnName = "?";
            for (auto& b : vitaButtons)
                if (b.second == *targets[i]) { btnName = b.first; break; }
            items.push_back(labels[i] + ": " + btnName);
        }
        items.push_back("Back");
        SimpleMenu("REDEFINE BUTTONS", items, sel);

        if (gInput.Pressed(SCE_CTRL_CROSS) && sel < 4) {
            // Wait for next button press
            DrawText("Press a button...", VITA_SCREEN_W/2, VITA_SCREEN_H/2, true, true);
            vglSwapBuffers(GL_FALSE);
            // Poll until new button detected
            bool waiting = true;
            while (waiting) {
                gInput.Update();
                for (auto& b : vitaButtons) {
                    if (gInput.Pressed(static_cast<SceCtrlButtons>(b.second))) {
                        *targets[sel] = b.second;
                        waiting = false;
                        break;
                    }
                }
            }
        }
        if (gInput.Pressed(SCE_CTRL_CIRCLE) || sel == 4) {
            screen = 2;
        }
    }
}

// ---------------------------------------------------------------------------
//  Slot Select  –  screen 5
// ---------------------------------------------------------------------------
void SlotSelect() {
    std::vector<BBString> items;
    for (int i = 1; i <= 3; ++i)
        items.push_back("Slot " + Str(i) + ": " +
                        (gamName[i].empty() ? "(empty)" : gamName[i]));
    items.push_back("Back");
    int sel = 0;

    while (screen == 5) {
        gInput.Update();
        if (gInput.Pressed(SCE_CTRL_DOWN)) sel = (sel+1) % items.size();
        if (gInput.Pressed(SCE_CTRL_UP))   sel = (sel-1+items.size()) % items.size();

        SimpleMenu("SELECT SLOT", items, sel);

        if (gInput.Pressed(SCE_CTRL_CROSS)) {
            ProduceSound(0, sMenuSelect, 100);
            if (sel < 3) {
                slot = sel + 1;
                screen = 50;   // → Gameplay
            } else {
                screen = 1;
            }
        }
        if (gInput.Pressed(SCE_CTRL_CIRCLE)) { screen = 1; }
    }
}

// ---------------------------------------------------------------------------
//  Credits  –  screen 6
// ---------------------------------------------------------------------------
void Credits() {
    std::vector<BBString> lines = {
        "HARD TIME",
        "",
        "Original game by Mat Dickie",
        "Copyright 2007",
        "",
        "PS Vita Port",
        "Community Fan Project",
        "",
        "Press X to return"
    };

    while (screen == 6) {
        gInput.Update();
        glClearColor(0.0f, 0.0f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
            bool title = (i == 0);
            SetColor(title ? 220 : 180, title ? 180 : 180, title ? 60 : 180);
            DrawText(lines[i], VITA_SCREEN_W/2, 80 + i*38, true, false);
        }
        vglSwapBuffers(GL_FALSE);

        if (gInput.Pressed(SCE_CTRL_CROSS) || gInput.Pressed(SCE_CTRL_CIRCLE))
            screen = 1;
    }
}

// ---------------------------------------------------------------------------
//  Outro  –  screen 7  (game over / end credits scroll)
// ---------------------------------------------------------------------------
void Outro() {
    std::vector<BBString> lines = {
        "", "HARD TIME",
        "A game by Mat Dickie  (c) 2007",
        "", "PS Vita Port",
        "Fan Community Project",
        "", "Thank you for playing!",
        "", "", "Press X to return to menu"
    };

    int scroll = VITA_SCREEN_H;
    while (screen == 7) {
        gInput.Update();
        scroll -= 1;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        glOrthof(0, VITA_SCREEN_W, VITA_SCREEN_H, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glDisable(GL_DEPTH_TEST);

        for (int i = 0; i < (int)lines.size(); ++i) {
            int y = scroll + i * 40;
            if (y < -40 || y > VITA_SCREEN_H + 40) continue;
            bool title = (lines[i] == "HARD TIME");
            SetColor(title ? 255 : 180, title ? 220 : 180, title ? 60 : 180);
            DrawText(lines[i], VITA_SCREEN_W / 2, y, true, false);
        }

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
        vglSwapBuffers(GL_FALSE);

        if (gInput.Pressed(SCE_CTRL_CROSS) || scroll < -(int)(lines.size() * 40 + 100))
            screen = 1;
    }
}

// ---------------------------------------------------------------------------
//  EditSelect  –  screen 8 (redirect to character editor screen 51)
// ---------------------------------------------------------------------------
void EditSelect() {
    screen = 51;
}

// ---------------------------------------------------------------------------
//  LoadWeaponData  –  fills weapName, weapFile, weapDamage etc.
//  (Port of Values.bb LoadWeaponData, lines 711-888)
// ---------------------------------------------------------------------------
void LoadWeaponData() {
    weapList = 25;
    weapName[0] = "Thing";

    // Helper: set all weapon type properties at once
    auto W = [](int n, const char* name, const char* file,
                BBSound* snd, GLuint tex, float shiny,
                float size, float weight, float range, int damage,
                int style, int habitat, int create, int value) {
        weapName[n]   = name;
        weapFile[n]   = file;
        weapSound[n]  = snd;
        weapTex[n]    = tex;
        weapShiny[n]  = shiny;
        weapSize[n]   = size;
        weapWeight[n] = weight;
        weapRange[n]  = range;
        weapDamage[n] = damage;
        weapStyle[n]  = style;
        weapHabitat[n]= habitat;
        weapCreate[n] = create;
        weapValue[n]  = value;
    };

    //  n  name              file        sound      tex       shiny  size  wgt   range dmg  style hab  cre  val
    W( 1, "Rock",           "Rock",     sRock,     0,        0.0f,  5.0f, 0.4f, 6.0f, 3,   0,    2,   0,   10);
    W( 2, "Wooden Plank",   "Plank",    sWood,     0,        0.0f,  8.0f, 0.3f, 8.0f, 3,   1,   10,   1,   10);
    W( 3, "Steel Pipe",     "Pipe",     sMetal,    0,        1.0f,  8.0f, 0.3f, 8.0f, 3,   1,   10,   1,   10);
    W( 4, "Baseball Bat",   "Bat",      sWood,     0,        0.25f, 8.0f, 0.3f, 8.0f, 3,   1,    2,   1,   20);
    W( 5, "Pool Cue",       "Cue",      sCane,     0,        0.25f,12.0f, 0.25f,10.0f,2,   1,    9,   1,   20);
    W( 6, "Knife",          "Dagger",   sBlade,    0,        1.0f,  6.0f, 0.25f, 6.0f,4,   7,    8,   1,   20);
    W( 7, "Pistol",         "Pistol",   sGun,      tPistol,  0.5f,  5.0f, 0.3f, 6.0f, 3,   3,    0,   1,  100);
    W( 8, "Machine Gun",    "Machine",  sGun,      tMachine, 0.5f,  8.0f, 0.4f, 8.0f, 3,   4,    0,   1,  100);
    W( 9, "Explosive",      "TNT",      sGeneric,  0,        0.0f,  6.0f, 0.3f, 5.0f, 2,   6,    0,   1,  100);
    W(10, "Brick",          "Brick",    sRock,     0,        0.0f,  6.0f, 0.4f, 6.0f, 3,   0,    2,   0,   10);
    W(11, "Dumbbell",       "Dumbell",  sAxe,      0,        0.25f, 8.0f, 0.5f, 6.0f, 5,   0,    2,   1,   20);
    W(12, "Nightstick",     "Baton",    sWood,     0,        0.25f, 6.0f, 0.3f, 7.0f, 3,   1,    0,   1,   20);
    W(13, "Hammer",         "Hammer",   sRock,     0,        0.25f, 5.0f, 0.4f, 6.0f, 4,   1,   10,   1,   20);
    W(14, "Ball",           "Ball",     sBall,     0,        0.0f,  7.0f, 0.3f, 5.0f, 1,   0,   99,   1,   10);
    W(15, "Broom",          "Broom",    sCane,     0,        0.0f, 13.0f, 0.25f,11.0f,2,   1,   99,   1,   20);
    W(16, "Cigarette",      "Cigar",    sCigar,    0,        0.0f,  4.0f, 0.15f, 6.0f,1,   0,    2,   0,   10);
    W(17, "Syringe",        "Syringe",  sSyringe,  0,        0.5f,  5.0f, 0.2f, 6.0f, 2,   7,    6,   0,   20);
    W(18, "Bottle",         "Bottle",   sBottle,   0,        0.25f, 5.0f, 0.25f, 5.0f,2,   6,    8,   0,   10);
    W(19, "Extinguisher",   "Exting",   sMetal,    0,        0.25f, 6.0f, 0.3f, 5.0f, 3,   6,   99,   1,   50);
    W(20, "Screwdriver",    "Screw",    sBlade,    0,        1.0f,  5.0f, 0.25f, 6.0f,3,   7,   10,   1,   10);
    W(21, "Scissor",        "Scissors", sBlade,    0,        1.0f,  5.0f, 0.25f, 6.0f,4,   7,    4,   1,   10);
    W(22, "Meat Cleaver",   "Cleaver",  sBlade,    0,        1.0f,  8.0f, 0.3f, 8.0f, 5,   1,    8,   1,   20);
    W(23, "Sword",          "Samurai",  sBlade,    0,        1.0f,  8.0f, 0.3f,10.0f, 5,   1,    0,   1,   50);
    W(24, "Comb",           "Comb",     sCigar,    0,        0.25f, 5.0f, 0.2f, 6.0f, 1,   0,   99,   0,   10);
    W(25, "Mirror",         "Mirror",   sGeneric,  0,        0.5f,  8.0f, 0.25f, 7.0f,2,   1,   99,   1,   20);
}
