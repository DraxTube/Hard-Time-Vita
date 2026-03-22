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
#include "debug_log.h"

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
    // Start debug log BEFORE anything else
    DebugLogInit();
    DebugLog("[MAIN] Starting Hard Time Vita...");

    DebugLog("[MAIN] Calling InitVita()...");
    InitVita();
    DebugLog("[MAIN] InitVita() done");

    // Mirror original Blitz3D startup sequence
    SeedRnd(MilliSecs());
    DebugLog("[MAIN] SeedRnd done");

    // Initialise game globals
    DebugLog("[MAIN] Calling InitValues()...");
    InitValues();
    DebugLog("[MAIN] InitValues() done");

    // Load user options from ux0:data/HardTime/Options.dat
    DebugLog("[MAIN] Calling LoadOptions()...");
    LoadOptions();
    DebugLog("[MAIN] LoadOptions() done");

    // Intro sequence (logo screen)
    DebugLog("[MAIN] Calling Intro()...");
    Intro();
    DebugLog("[MAIN] Intro() done");

    // Load all shared media
    DebugLog("[MAIN] Calling LoadImages()...");
    LoadImages();
    DebugLog("[MAIN] LoadImages() done");

    DebugLog("[MAIN] Calling LoadTextures()...");
    LoadTextures();
    DebugLog("[MAIN] LoadTextures() done");

    DebugLog("[MAIN] Calling LoadWeaponData()...");
    LoadWeaponData();
    DebugLog("[MAIN] LoadWeaponData() done");

    // --- Main screen loop (mirrors original Repeat/Until loop) ---
    SeedRnd(MilliSecs());
    screen = 1;
    DebugLog("[MAIN] Entering main loop, screen=%d", screen);

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
    DebugLog("[MAIN] Exiting main loop");
    DebugLogClose();
    ShutdownAudio();
    sceKernelExitProcess(0);
    return 0;
}
