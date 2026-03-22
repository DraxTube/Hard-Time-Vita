#pragma once
// =============================================================================
//  particles.h  –  Particle FX, Pools, Explosions  (from Particles.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"
#include <array>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
static constexpr int NO_PARTICLES = 500;
static constexpr int NO_EXPLODES  = 20;
static constexpr int NO_POOLS     = 50;

// ---------------------------------------------------------------------------
//  Particles
// ---------------------------------------------------------------------------
extern std::array<Handle, NO_PARTICLES+1> part;
extern std::array<int,    NO_PARTICLES+1> partType;
extern std::array<float,  NO_PARTICLES+1> partX, partY, partZ, partA;
extern std::array<float,  NO_PARTICLES+1> partGravity, partFlight;
extern std::array<float,  NO_PARTICLES+1> partSize, partAlpha, partFade;
extern std::array<int,    NO_PARTICLES+1> partTim, partState;

// ---------------------------------------------------------------------------
//  Explosions
// ---------------------------------------------------------------------------
extern std::array<int,   NO_EXPLODES+1> exType, exTim, exSource;
extern std::array<float, NO_EXPLODES+1> exX, exY, exZ;
// exHurt[explosion][player]
extern std::array<std::array<int,51>, NO_EXPLODES+1> exHurt;

// ---------------------------------------------------------------------------
//  Pools (blood / water / beer)
// ---------------------------------------------------------------------------
extern std::array<Handle, NO_POOLS+1> pool;
extern std::array<int,    NO_POOLS+1> poolType, poolState;
extern std::array<float,  NO_POOLS+1> poolX, poolY, poolZ, poolA;
extern std::array<float,  NO_POOLS+1> poolSize, poolAlpha;

// ---------------------------------------------------------------------------
//  Sound handles used by FX
// ---------------------------------------------------------------------------
extern BBSound* sExplosion;
extern BBSound* sSplash;
extern BBSound* sBlaze;
extern BBSound* sSwing;
extern std::array<BBSound*, 4>  sShuffle;
extern std::array<BBSound*, 11> sPain;
extern std::array<BBSound*, 7>  sImpact;

// ---------------------------------------------------------------------------
//  Functions
// ---------------------------------------------------------------------------
void LoadParticles();
void LoadPools();
void CreateParticle(float x, float y, float z, int style);
void CreateSpurt(float x, float y, float z, int spread, int density, int style);
void ParticleCycle();
void PoolCycle();
void CreatePool(float x, float y, float z, float size, int layers, int style);
void CreateExplosion(int source, Handle entity, float x, float y, float z, int style);
void ExplosionCycle();
int  BlastProximity(int cyc, float x, float y, float z, float range);
