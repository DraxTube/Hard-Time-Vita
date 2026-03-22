// =============================================================================
//  main.cpp  –  PS Vita entry point for Hard Time
//
//  Replaces the Blitz3D runtime:
//    - Initialises vitaGL (OpenGL ES wrapper for GXM)
//    - Sets up SceCtrl input
//    - Runs the main screen loop (mirrors original Gameplay.bb)
// =============================================================================

#include "blitz_compat.h"
#include "render3d.h"
#include "values.h"
#include "functions.h"
#include "menus.h"
#include "gameplay.h"
#include "editor.h"
#include "texts.h"
#include "data.h"
#include "costume.h"

#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <vitaGL.h>

// ---------------------------------------------------------------------------
//  Global definitions (declared extern in headers)
// ---------------------------------------------------------------------------
BB::Input BB::gInput;

// ---------------------------------------------------------------------------
//  Vita system initialisation
// ---------------------------------------------------------------------------
static void InitVita() {
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    // Init audio engine (real SceAudio playback)
    InitAudio();
    // Init vitaGL
    vglInitExtended(0, VITA_SCREEN_W, VITA_SCREEN_H, 8 * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X);
}

// ---------------------------------------------------------------------------
//  Screen router  (mirrors original LoadScreen() in Gameplay.bb)
// ---------------------------------------------------------------------------
static void LoadScreen(int request) {
    switch (request) {
        case 1:  MainMenu();      break;
        case 2:  Options();       break;
        case 3:  RedefineKeys();  break;
        case 4:  RedefineGamepad(); break;
        case 5:  SlotSelect();    break;
        case 6:  Credits();       break;
        case 7:  Outro();         break;
        case 8:  EditSelect();    break;
        // 3D scenes
        case 50: Gameplay();      break;
        case 51: Editor();        break;
        case 52: CourtCase();     break;
        case 53: Ending();        break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
//  Program entry
// ---------------------------------------------------------------------------
int main() {
    InitVita();

    // Mirror original Blitz3D startup sequence
    SeedRnd(MilliSecs());

    // Initialise game globals
    InitValues();

    // Load user options from ux0:data/HardTime/Options.dat
    LoadOptions();

    // Intro sequence (logo screen)
    Intro();

    // Load all shared media
    LoadImages();
    LoadTextures();
    LoadWeaponData();

    // --- Main screen loop (mirrors original Repeat/Until loop) ---
    SeedRnd(MilliSecs());
    screen = 1;

    while (screen != 0) {
        // Update controller state
        gInput.Update();

        // Quick-exit: Start + Select (replaces KB56+KB45 in original)
        if (gInput.Held(SCE_CTRL_START) && gInput.Held(SCE_CTRL_SELECT)) {
            break;
        }

        LoadScreen(screen);
    }

    // Shutdown
    ShutdownAudio();
    sceKernelExitProcess(0);
    return 0;
}
