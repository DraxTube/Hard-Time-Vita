#pragma once
// =============================================================================
//  players.h  –  Player & character data arrays  (from Values.bb + Players.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"
#include <array>
#include <string>

static constexpr int P   = 51;   // OPT_PLAY_LIM + 1
static constexpr int C   = 201;  // OPT_CHAR_LIM + 1
static constexpr int WL  = 101;  // OPT_WEAP_LIM + 1
static constexpr int WLS = 26;   // weapList + 1
static constexpr int LIMBS = 41;
static constexpr int SEQ_MAX = 621;

// ---------------------------------------------------------------------------
//  Player runtime arrays  (index 1..no_plays)
// ---------------------------------------------------------------------------
extern std::array<Handle, P> p;            // 3D mesh entity handle
extern std::array<Handle, P> pPivot;       // collision pivot
extern std::array<Handle, P> pMovePivot;

extern std::array<float, P> pX, pY, pZ;   // world position
extern std::array<float, P> pA;            // angle (yaw)
extern std::array<float, P> pTX, pTY, pTZ, pTA; // target position/angle
extern std::array<float, P> pOldX, pOldZ;
extern std::array<float, P> pGravity;
extern std::array<float, P> pGround;
extern std::array<float, P> pSubX, pSubZ;   // sub-target (avoidance)
extern std::array<float, P> pExploreX, pExploreY, pExploreZ;
extern std::array<float, P> pCharge;
extern std::array<float, P> pSeatX, pSeatY, pSeatZ, pSeatA;
extern std::array<float, P> pAnimSpeed;

extern std::array<int, P> pChar;          // character index
extern std::array<int, P> pControl;       // 0=CPU,1=KB,2=pad,3=both
extern std::array<int, P> pAgenda;        // 0=contemplate,1=explore,2=follow,3=lockdown,4=weapon
extern std::array<int, P> pOldAgenda;
extern std::array<int, P> pState;
extern std::array<int, P> pAnim;
extern std::array<int, P> pAnimTim;
extern std::array<int, P> pHealth;
extern std::array<int, P> pHealthLimit;
extern std::array<int, P> pHP;
extern std::array<int, P> pInjured;
extern std::array<int, P> pWeapon;
extern std::array<int, P> pPhone;
extern std::array<int, P> pSeat;
extern std::array<int, P> pBed;
extern std::array<int, P> pGrappling;
extern std::array<int, P> pGrappler;
extern std::array<int, P> pNowhere;
extern std::array<int, P> pSatisfied;
extern std::array<int, P> pRunTim;
extern std::array<int, P> pControlTim;
extern std::array<int, P> pEyes;
extern std::array<int, P> pOldEyes;
extern std::array<int, P> pCollisions;
extern std::array<int, P> pFollowFoc;
extern std::array<int, P> pWeapFoc;

// 2D player arrays
extern std::array<std::array<Handle, LIMBS>, P> pLimb;
extern std::array<std::array<int,    LIMBS>, P> pScar;
extern std::array<std::array<int,    LIMBS>, P> pOldScar;
extern std::array<std::array<Handle, LIMBS>, P> pShadow;
extern std::array<std::array<int,    SEQ_MAX>, P> pSeq;
extern std::array<std::array<int,    P>,      P> pInteract;
extern std::array<std::array<int,    51>,     P> pSeatFriction;
extern std::array<std::array<int,    21>,     P> pBedFriction;
extern std::array<std::array<int,    WL>,     P> pWeaponTim;

// Input commands (set by GetInput each frame)
extern std::array<int, P> cUp, cDown, cLeft, cRight;
extern std::array<int, P> cAttack, cDefend, cThrow, cPickUp;

// Camera focus
extern int camFoc;   // which player index the camera follows

// ---------------------------------------------------------------------------
//  Character data arrays  (index 1..no_chars, persistent)
// ---------------------------------------------------------------------------
extern std::array<std::string, C> charName;
extern std::array<int, C> charModel;
extern std::array<int, C> charHeight;
extern std::array<int, C> charSpecs;
extern std::array<int, C> charAccessory;
extern std::array<int, C> charHairStyle;
extern std::array<int, C> charHair;
extern std::array<int, C> charFace;
extern std::array<int, C> charCostume;
extern std::array<int, C> charHealth;
extern std::array<int, C> charHP;
extern std::array<int, C> charStrength;
extern std::array<int, C> charAgility;
extern std::array<int, C> charHappiness;
extern std::array<int, C> charBreakdown;
extern std::array<int, C> charIntelligence;
extern std::array<int, C> charReputation;
extern std::array<int, C> charOldStrength;
extern std::array<int, C> charOldAgility;
extern std::array<int, C> charOldIntelligence;
extern std::array<int, C> charOldReputation;
extern std::array<int, C> charExperience;
extern std::array<int, C> charPlayer;
extern std::array<int, C> charPhoto;
extern std::array<int, C> charSnapped;
extern std::array<int, C> charRole;       // 0=prisoner,1=warden
extern std::array<int, C> charSentence;
extern std::array<int, C> charCrime;
extern std::array<int, C> charLocation;
extern std::array<int, C> charBlock;
extern std::array<int, C> charCell;
extern std::array<int, C> charGang;
extern std::array<int, C> charAttacker;
extern std::array<int, C> charWitness;
extern std::array<int, C> charFollowTim;
extern std::array<int, C> charBribeTim;
extern std::array<int, C> charWeapon;
extern std::array<int, C> charInjured;

extern std::array<float, C> charX, charY, charZ, charA;

// 2D character arrays
extern std::array<std::array<int, LIMBS>, C> charScar;
extern std::array<std::array<int, 7>,     C> charGangHistory;
extern std::array<std::array<int, C>,     C> charRelation;
extern std::array<std::array<int, C>,     C> charAngerTim;
extern std::array<std::array<int, 31>,    C> charWeapHistory;
extern std::array<std::array<int, C>,     C> charPromo;

// ---------------------------------------------------------------------------
//  Weapon runtime arrays  (index 1..no_weaps)
// ---------------------------------------------------------------------------
extern std::array<Handle, WL> weap;
extern std::array<Handle, WL> weapGround;
extern std::array<Handle, WL> weapWall;
extern std::array<int, WL>    weapType;
extern std::array<int, WL>    weapCarrier;
extern std::array<int, WL>    weapThrower;
extern std::array<int, WL>    weapClip;
extern std::array<int, WL>    weapAmmo;
extern std::array<int, WL>    weapScar;
extern std::array<int, WL>    weapOldScar;
extern std::array<int, WL>    weapState;
extern std::array<int, WL>    weapLocation;
extern std::array<float, WL>  weapX, weapY, weapZ;
extern std::array<float, WL>  weapOldX, weapOldY, weapOldZ;
extern std::array<float, WL>  weapA;
extern std::array<float, WL>  weapFlight;
extern std::array<float, WL>  weapFlightA;
extern std::array<float, WL>  weapGravity;
extern std::array<float, WL>  weapBounce;
extern std::array<std::array<int, P>, WL> weapSting;

// Weapon type data  (index 1..weapList)
extern int weapList;   // = 25
extern std::array<std::string, WLS> weapName;
extern std::array<std::string, WLS> weapFile;
extern std::array<BBSound*, WLS>    weapSound;
extern std::array<GLuint,   WLS>    weapTex;
extern std::array<float, WLS>       weapSize;
extern std::array<float, WLS>       weapWeight;
extern std::array<float, WLS>       weapRange;
extern std::array<float, WLS>       weapShiny;
extern std::array<int, WLS>         weapValue;
extern std::array<int, WLS>         weapDamage;
extern std::array<int, WLS>         weapStyle;  // 0=hand,1=sword,2=shield,3=pistol...

// ---------------------------------------------------------------------------
//  World
// ---------------------------------------------------------------------------
extern Handle world;
extern int no_chairs, no_beds, no_doors;
extern std::array<Handle, 21> tSign;
extern std::array<Handle, 5>  tBlock;
extern std::array<Handle, 21> tCell;
extern Handle tFence, tNet;
extern std::array<std::array<int, 21>, 12> cellLocked;

// ---------------------------------------------------------------------------
//  Functions
// ---------------------------------------------------------------------------
void LoadPlayers();
void PlayerCycle();
void ApplyCostume(int cyc);
void SeverLimbs(int cyc);
void LoadSequences(int cyc);

void LoadWeapons();
void WeaponCycle();
void LoadWeaponData();

void LoadWorld();
void SetCollisions();

// ---------------------------------------------------------------------------
//  Extra globals from Values.bb not yet in a dedicated header
// ---------------------------------------------------------------------------
// Lighting
extern float lightR, lightG, lightB;
extern float ambR, ambG, ambB;
extern float skyR, skyG, skyB;
// Fader (screen fade in/out)
extern Handle fader;
extern float  fadeAlpha, fadeTarget;
// Bullets
static constexpr int NO_BULLETS = 40;
extern std::array<Handle, NO_BULLETS+1> bullet;
extern std::array<float,  NO_BULLETS+1> bulletX, bulletY, bulletZ;
extern std::array<float,  NO_BULLETS+1> bulletXA, bulletYA, bulletZA;
extern std::array<int,    NO_BULLETS+1> bulletState, bulletTim, bulletShooter;
// World screen state
extern int wScreen, wOldScreen;
// Limb data
extern std::array<int, 41> limbPrecede, limbSource;
// HUD texture handles
extern GLuint gHealth, gHappiness, gMoney, gPhoto, gMap, gMarker;
// Texture counts (filled by LoadTextures)
extern int no_hairs, no_faces, no_bodies, no_arms, no_legs;
