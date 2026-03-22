#pragma once
// =============================================================================
//  values.h  –  Global variables (converted from Values.bb)
//  All Blitz3D Globals / Dims are plain C++ variables / std::arrays
// =============================================================================

#include "blitz_compat.h"
#include "render3d.h"
#include <array>
#include <string>

// ---------------------------------------------------------------------------
//  STRUCTURE / screen state
// ---------------------------------------------------------------------------
extern int version;        // = 4
extern int screen, oldScreen;
extern int screenSource, screenAgenda;
extern int screenCall, callX, callY;
extern int go, gotim;
extern int foc, subfoc;
extern int timer, keytim;
extern int tester;

// ---------------------------------------------------------------------------
//  PROGRESS
// ---------------------------------------------------------------------------
extern int slot;
extern int oldLocation;
extern int gamPoints, gamPointLimit;
extern int gamPause, gamFile, gamDoor;
extern int gamEnded;

static constexpr int MAX_SLOTS = 3;
extern std::array<std::string, MAX_SLOTS+1> gamName;
extern std::array<int, MAX_SLOTS+1> gamPhoto;
extern std::array<int, MAX_SLOTS+1> gamChar;
extern std::array<int, MAX_SLOTS+1> gamPlayer;
extern std::array<int, MAX_SLOTS+1> gamLocation;
extern std::array<int, MAX_SLOTS+1> gamMoney;
extern std::array<int, MAX_SLOTS+1> gamSpeed;
extern std::array<int, MAX_SLOTS+1> gamSecs;
extern std::array<int, MAX_SLOTS+1> gamMins;
extern std::array<int, MAX_SLOTS+1> gamHours;

// handles
extern std::array<int, MAX_SLOTS+1> gamWarrant;
extern std::array<int, MAX_SLOTS+1> gamVictim;
extern std::array<int, MAX_SLOTS+1> gamItem;
extern std::array<int, MAX_SLOTS+1> gamArrival;
extern std::array<int, MAX_SLOTS+1> gamFatality;
extern std::array<int, MAX_SLOTS+1> gamRelease;
extern std::array<int, MAX_SLOTS+1> gamEscape;
extern std::array<int, MAX_SLOTS+1> gamGrowth;
extern std::array<int, MAX_SLOTS+1> gamBlackout;
extern std::array<int, MAX_SLOTS+1> gamBombThreat;

// missions
extern std::array<int, MAX_SLOTS+1> gamMission;
extern std::array<int, MAX_SLOTS+1> gamClient;
extern std::array<int, MAX_SLOTS+1> gamTarget;
extern std::array<int, MAX_SLOTS+1> gamDeadline;
extern std::array<int, MAX_SLOTS+1> gamReward;

// stat highlight timers (1=strength … 7=money)
extern std::array<int, 11> statTim;

// promos
extern int gamPromo;
static constexpr int NO_PROMOS = 300;
extern int promoTim, promoStage;
extern int promoEffect, promoVariable;
extern int promoAccuser, promoVerdict, promoCash;
extern std::string optionA, optionB;
extern std::array<int, 3>  promoActor;
extern std::array<int, 11> promoReact;
extern std::array<int, NO_PROMOS+1> promoUsed;

// phones
extern int phoneRing, phoneTim, phonePromo;
extern std::array<float, 5> phoneX, phoneY, phoneZ;

// outro
extern std::array<int, 11> endChar;
extern std::array<int, 11> endFate;

// ---------------------------------------------------------------------------
//  OPTIONS
// ---------------------------------------------------------------------------
static constexpr int OPT_PLAY_LIM  = 50;
static constexpr int OPT_CHAR_LIM  = 200;
static constexpr int OPT_WEAP_LIM  = 100;

extern int optPopulation;   // default 60
extern int optRes;          // Vita: fixed at 0 (960×544)
extern int optFog;          // 0/1
extern int optFX;           // 0/1
extern int optShadows;      // 0–2
extern int optGore;         // 0=none,1=scars,2=pools,3=limbs

// Keys (Vita: mapped to SceCtrlButtons constants)
extern int keyAttack, keyDefend, keyThrow, keyPickUp;
// Buttons (original gamepad button IDs 1-4 map to Cross/Square/Circle/Triangle)
extern int buttAttack, buttDefend, buttThrow, buttPickUp;

// ---------------------------------------------------------------------------
//  SOUND handles  (BBSound pointers replace Blitz3D integer sound handles)
// ---------------------------------------------------------------------------
#include <memory>

// Music & atmosphere
extern BBSound* sTheme;
extern int chTheme;
extern float musicVol;
extern int chAtmos;
extern BBSound* sAtmos;
extern int chPhone, chAlarm;
extern int chDeath;

// Menu sfx
extern BBSound* sMenuBrowse;
extern BBSound* sMenuSelect;
extern BBSound* sMenuGo;
extern BBSound* sMenuBack;
extern BBSound* sVoid;
extern BBSound* sTrash;
extern BBSound* sCamera;
extern BBSound* sComputer;
extern BBSound* sCash;
extern BBSound* sPaper;
extern BBSound* sMurmur;
extern std::array<BBSound*, 3> sJury;

// World sounds
extern std::array<BBSound*, 4> sDoor;
extern BBSound* sBuzzer;
extern BBSound* sBell;
extern BBSound* sRing;
extern BBSound* sAlarm;
extern BBSound* sTanoy;
extern BBSound* sBasket;

// Movement sounds
extern BBSound* sFall;
extern BBSound* sThud;
extern BBSound* sBreakdown;

// Pain sounds
extern BBSound* sDeath;
extern BBSound* sChoke;
extern BBSound* sSnore;

// Impact sounds
extern BBSound* sBleed;
extern BBSound* sStab;
extern BBSound* sEat;
extern BBSound* sDrink;

// Weapon sounds
extern BBSound* sGeneric;
extern BBSound* sBlade;
extern BBSound* sMetal;
extern BBSound* sWood;
extern BBSound* sCane;
extern BBSound* sString;
extern BBSound* sRock;
extern BBSound* sAxe;
extern BBSound* sBall;
extern BBSound* sPhone;
extern BBSound* sCigar;
extern BBSound* sSyringe;
extern BBSound* sBottle;

// Technology sounds
extern BBSound* sReload;
extern BBSound* sGun;
extern BBSound* sMine;
extern BBSound* sLaser;

// Weapon textures (used in weapon data)
extern GLuint tMachine;
extern GLuint tPistol;

// ---------------------------------------------------------------------------
//  CHARACTERS / COUNTS (actual arrays are in players.h)
// ---------------------------------------------------------------------------
static constexpr int MAX_CHARS  = 201;
static constexpr int MAX_PLAYS  = 51;
static constexpr int MAX_WEAPS  = 101;
static constexpr int MAX_LOCS   = 12;

extern int no_chars;
extern int no_plays;
extern int no_weaps;
extern int no_costumes;  // = 8
extern int no_models;    // = 5
extern int no_hairstyles; // = 31
extern int no_specs;      // = 4

// charStatus is needed for missions (0=alive, 1=incapacitated, 2=dead)
extern std::array<int, MAX_CHARS> charStatus;
// charPromoRef for promo tracking
extern std::array<int, MAX_CHARS> charPromoRef;

// ---------------------------------------------------------------------------
//  RENDER handles  (Blitz3D integer handles → R3D::Handle = int)
// ---------------------------------------------------------------------------
extern int cam;    // camera entity handle
extern int light1; // main directional light

// ---------------------------------------------------------------------------
//  WORLD  (Handle-typed arrays are in players.h to avoid duplication)
// ---------------------------------------------------------------------------
extern int no_chairs, no_beds, no_doors;
extern std::array<std::array<int, 21>, 12> cellLocked;

// Food trays
extern std::array<int, 51> trayState;
extern std::array<int, 51> trayOldState;

// Screen textures
extern std::array<GLuint, 11> tScreen;
extern std::array<GLuint, 11> tTray;
extern std::array<GLuint, 4> tEyes;

// Camera
extern int camType, camTim;
extern int camRectify;
extern float camX, camY, camZ;
extern float camTX, camTY, camTZ;
extern float camPivX, camPivY, camPivZ;
extern float camPivTX, camPivTY, camPivTZ;
extern float speedX, speedY, speedZ;
extern std::array<int, 11> camShortcut;

// Lighting
extern int no_lights;
extern float lightR, lightG, lightB;
extern float lightTR, lightTG, lightTB;
extern float ambR, ambG, ambB;
extern float ambTR, ambTG, ambTB;
extern float atmosR, atmosG, atmosB;
extern float atmosTR, atmosTG, atmosTB;
extern float skyR, skyG, skyB;
extern float skyTR, skyTG, skyTB;

// Fader (screen fade in/out)
extern float  fadeAlpha, fadeTarget;

// ---------------------------------------------------------------------------
//  DOORS (from Values.bb .Doors section)
// ---------------------------------------------------------------------------
extern std::array<std::array<float, 11>, 16> doorA;
extern std::array<std::array<float, 11>, 16> doorX1, doorX2;
extern std::array<std::array<float, 11>, 16> doorY1, doorY2;
extern std::array<std::array<float, 11>, 16> doorZ1, doorZ2;

// ---------------------------------------------------------------------------
//  CELLS (from Values.bb .Cells section)
// ---------------------------------------------------------------------------
extern std::array<float, 21> cellX1, cellX2;
extern std::array<float, 21> cellY1, cellY2;
extern std::array<float, 21> cellZ1, cellZ2;
extern std::array<float, 21> cellDoorX, cellDoorZ;

// BULLETS – declared in players.h (needs NO_BULLETS constant)

// World screen state
extern int wScreen, wOldScreen;

// Limb data
extern std::array<int, 41> limbPrecede, limbSource;

// HUD texture handles
extern GLuint gHealth, gHappiness, gMoney, gPhoto, gMap, gMarker;

// Texture counts (filled by LoadTextures)
extern int no_hairs, no_faces, no_bodies, no_arms, no_legs;

// WEAPON CREATION KITS – declared in world_props.h

// ---------------------------------------------------------------------------
//  Functions
// ---------------------------------------------------------------------------
void InitValues();
void InitDoorData();
void InitCellData();
void InitLimbHierarchy();
