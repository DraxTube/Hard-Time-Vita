// =============================================================================
//  particles.cpp  –  Particle FX, Pools, Explosions  (ported from Particles.bb)
// =============================================================================
#include "particles.h"
#include "players.h"
#include "anims.h"
#include "values.h"
#include "functions.h"
#include "render3d.h"
#include "blitz_compat.h"

// ---------------------------------------------------------------------------
//  Array definitions
// ---------------------------------------------------------------------------
std::array<Handle, NO_PARTICLES+1> part        {};
std::array<int,    NO_PARTICLES+1> partType    {};
std::array<float,  NO_PARTICLES+1> partX       {};
std::array<float,  NO_PARTICLES+1> partY       {};
std::array<float,  NO_PARTICLES+1> partZ       {};
std::array<float,  NO_PARTICLES+1> partA       {};
std::array<float,  NO_PARTICLES+1> partGravity {};
std::array<float,  NO_PARTICLES+1> partFlight  {};
std::array<float,  NO_PARTICLES+1> partSize    {};
std::array<float,  NO_PARTICLES+1> partAlpha   {};
std::array<float,  NO_PARTICLES+1> partFade    {};
std::array<int,    NO_PARTICLES+1> partTim     {};
std::array<int,    NO_PARTICLES+1> partState   {};

std::array<int,   NO_EXPLODES+1> exType   {};
std::array<int,   NO_EXPLODES+1> exTim    {};
std::array<int,   NO_EXPLODES+1> exSource {};
std::array<float, NO_EXPLODES+1> exX      {};
std::array<float, NO_EXPLODES+1> exY      {};
std::array<float, NO_EXPLODES+1> exZ      {};
std::array<std::array<int,51>, NO_EXPLODES+1> exHurt {};

std::array<Handle, NO_POOLS+1> pool      {};
std::array<int,    NO_POOLS+1> poolType  {};
std::array<int,    NO_POOLS+1> poolState {};
std::array<float,  NO_POOLS+1> poolX     {};
std::array<float,  NO_POOLS+1> poolY     {};
std::array<float,  NO_POOLS+1> poolZ     {};
std::array<float,  NO_POOLS+1> poolA     {};
std::array<float,  NO_POOLS+1> poolSize  {};
std::array<float,  NO_POOLS+1> poolAlpha {};

BBSound* sExplosion = nullptr;
BBSound* sSplash    = nullptr;
BBSound* sBlaze     = nullptr;
BBSound* sSwing     = nullptr;
std::array<BBSound*, 4>  sShuffle {};
std::array<BBSound*, 11> sPain    {};
std::array<BBSound*, 7>  sImpact  {};

// ---------------------------------------------------------------------------
//  LoadParticles  (from Particles.bb)
// ---------------------------------------------------------------------------
void LoadParticles() {
    const std::string BASE = "ux0:data/HardTime/";
    for (int cyc = 1; cyc <= NO_PARTICLES; ++cyc) {
        part[cyc] = CreateSprite();
        EntityFX(part[cyc], 9);
        partState[cyc] = 0;
        HideEntity(part[cyc]);
    }

    // Load combat sounds
    sExplosion = Load3DSound(BASE + "Sound/Props/Explosion.wav");
    sSplash    = Load3DSound(BASE + "Sound/Props/Splash.wav");
    sBlaze     = Load3DSound(BASE + "Sound/Props/Blaze.wav");
    sSwing     = Load3DSound(BASE + "Sound/Movement/Swing.wav");
    for (int i = 1; i <= 3; ++i)
        sShuffle[i] = Load3DSound(BASE + "Sound/Movement/Shuffle0" + Str(i) + ".wav");
    for (int i = 1; i <= 8; ++i)
        sPain[i] = Load3DSound(BASE + "Sound/Movement/Pain0" + Dig(i,10) + ".wav");
    for (int i = 1; i <= 6; ++i)
        sImpact[i] = Load3DSound(BASE + "Sound/Movement/Impact0" + Str(i) + ".wav");
}

// ---------------------------------------------------------------------------
//  LoadPools  (from Particles.bb)
// ---------------------------------------------------------------------------
void LoadPools() {
    const std::string BASE = "ux0:data/HardTime/";
    for (int cyc = 1; cyc <= NO_POOLS; ++cyc) {
        pool[cyc] = CreateSprite();
        SpriteViewMode(pool[cyc], 2);
        HideEntity(pool[cyc]);
        poolState[cyc] = 0;
    }
}

// ---------------------------------------------------------------------------
//  CreateParticle  (from Particles.bb)
// ---------------------------------------------------------------------------
void CreateParticle(float x, float y, float z, int style) {
    if (optFX <= 0) return;

    // Find empty slot
    int cyc = 0;
    for (int c = 1; c <= NO_PARTICLES; ++c)
        if (partState[c] == 0) { cyc = c; break; }
    if (cyc == 0) cyc = Rnd(1, NO_PARTICLES);
    if (cyc == 0) return;

    partX[cyc] = x; partY[cyc] = y; partZ[cyc] = z;
    partA[cyc]       = static_cast<float>(Rnd(0, 360));
    partGravity[cyc] = RndF(1.0f, 2.0f);
    partFlight[cyc]  = 0.3f;
    partSize[cyc]    = RndF(1.0f, 5.0f);
    partAlpha[cyc]   = RndF(0.5f, 0.9f);
    partFade[cyc]    = 0.02f;
    partType[cyc]    = style;

    // Per-type overrides
    int r = 255, g = 255, b = 255;
    switch (style) {
        case 1:  // fire
            r=220; g=Rnd(0,100); b=0; break;
        case 2:  // smoke
            r=g=b=Rnd(0,100);
            partSize[cyc]=RndF(1.0f,3.0f); partFlight[cyc]=0.1f;
            partGravity[cyc]=0.1f; partAlpha[cyc]=RndF(0.4f,0.8f); partFade[cyc]=0.01f; break;
        case 3:  // blood
            r=Rnd(50,200); g=0; b=0;
            partFlight[cyc]=0.2f; partGravity[cyc]=RndF(0.5f,1.0f);
            partAlpha[cyc]=RndF(0.7f,0.9f); partFade[cyc]=0.035f; break;
        case 4:  // impact
            r=Rnd(90,110); g=Rnd(70,90); b=Rnd(40,60);
            partFlight[cyc]=0.15f; partGravity[cyc]=RndF(0.5f,1.0f);
            partAlpha[cyc]=RndF(0.6f,0.8f); partFade[cyc]=0.035f; break;
        case 5:  // dust
            r=100; g=80; b=50;
            partAlpha[cyc]=RndF(0.2f,0.5f); partSize[cyc]=RndF(1.0f,3.0f);
            partGravity[cyc]=0.5f; break;
        case 6:  // water
            r=40; g=60; b=80;
            partFlight[cyc]=0.3f; partSize[cyc]=RndF(2.0f,6.0f);
            partGravity[cyc]=RndF(0.0f,1.0f);
            partAlpha[cyc]=RndF(0.3f,0.7f); partFade[cyc]=0.02f; break;
        case 7:  // small fire
            r=220; g=Rnd(0,100); b=0;
            partSize[cyc]=RndF(0.1f,1.0f); partGravity[cyc]=0.0f; partFade[cyc]=0.1f; break;
        case 8:  // multicolour
            r=Rnd(100,250); g=Rnd(100,250); b=Rnd(100,250); break;
        case 9:  // green mist
            r=0; g=Rnd(100,180); b=0;
            partGravity[cyc]=RndF(0.75f,1.25f); partFade[cyc]=0.03f; break;
        case 10: // explosion fire
            r=220; g=Rnd(0,100); b=0;
            partSize[cyc]=RndF(5.0f,10.0f); partAlpha[cyc]=RndF(0.8f,1.0f); break;
        case 11: // explosion foam
            r=g=b=Rnd(100,200);
            partSize[cyc]=RndF(5.0f,10.0f); partAlpha[cyc]=RndF(0.6f,0.8f); break;
        case 12: // explosion water
            r=40; g=80; b=120;
            partSize[cyc]=RndF(5.0f,10.0f); partAlpha[cyc]=RndF(0.7f,0.9f); break;
        case 13: // explosion beer
            r=Rnd(50,150); g=50; b=0;
            partSize[cyc]=RndF(5.0f,10.0f); partAlpha[cyc]=RndF(0.7f,0.9f); break;
        case 14: // beer small
            r=Rnd(50,150); g=50; b=0;
            partSize[cyc]=RndF(0.5f,2.0f); partFlight[cyc]=0.0f; partGravity[cyc]=0.0f; break;
    }
    EntityColor(part[cyc], r, g, b);

    partTim[cyc] = 0;
    partState[cyc] = 1;
    ShowEntity(part[cyc]);
    PositionEntity(part[cyc], partX[cyc], partY[cyc], partZ[cyc]);
    RotateEntity(part[cyc], 0.0f, partA[cyc], 0.0f);
    ScaleSprite(part[cyc], partSize[cyc], partSize[cyc]);
    EntityAlpha(part[cyc], partAlpha[cyc]);
}

// ---------------------------------------------------------------------------
//  CreateSpurt  (from Particles.bb)
// ---------------------------------------------------------------------------
void CreateSpurt(float x, float y, float z, int spread, int density, int style) {
    if (optFX <= 0) return;
    if (optFX <= 1) density /= 2;
    for (int c = 0; c < density; ++c) {
        float px = x + Rnd(-spread, spread);
        float py = y + Rnd(-spread, spread);
        float pz = z + Rnd(-spread, spread);
        if (style < 99) {
            CreateParticle(px, py, pz, style);
        } else {
            CreateParticle(px, py, pz, 4);
            CreateParticle(px, py, pz, 3);
        }
    }
}

// ---------------------------------------------------------------------------
//  ParticleCycle  (from Particles.bb)
// ---------------------------------------------------------------------------
void ParticleCycle() {
    for (int cyc = 1; cyc <= NO_PARTICLES; ++cyc) {
        if (partState[cyc] == 0) continue;

        if (partType[cyc] != 7) {
            // Gravity
            partGravity[cyc] -= 0.05f;
            if (partType[cyc] == 2 && partGravity[cyc] < 0.1f)  partGravity[cyc] = 0.1f;
            if (partType[cyc] == 14 && partGravity[cyc] < -0.1f) partGravity[cyc] = -0.1f;
            partY[cyc] += partGravity[cyc];
            // Forward flight
            MoveEntity(part[cyc], 0.0f, 0.0f, partFlight[cyc]);
            partX[cyc] = EntityX(part[cyc]);
            partZ[cyc] = EntityZ(part[cyc]);
        }

        PositionEntity(part[cyc], partX[cyc], partY[cyc], partZ[cyc]);
        RotateEntity(part[cyc], 0.0f, partA[cyc], 0.0f);
        ScaleSprite(part[cyc], partSize[cyc], partSize[cyc]);

        partAlpha[cyc] -= partFade[cyc];
        EntityAlpha(part[cyc], partAlpha[cyc]);
        partTim[cyc]++;

        if (partAlpha[cyc] <= 0.0f || partTim[cyc] > 1000)
            partState[cyc] = 0;

        if (partState[cyc] == 0) HideEntity(part[cyc]);
    }
}

// ---------------------------------------------------------------------------
//  CreatePool  (from Particles.bb)
// ---------------------------------------------------------------------------
void CreatePool(float x, float y, float z, float size, int layers, int style) {
    if (optGore < 2) return;
    for (int count = 0; count < layers; ++count) {
        int cyc = 0;
        for (int c = 1; c <= NO_POOLS; ++c)
            if (poolState[c] == 0) { cyc = c; break; }
        if (cyc == 0) cyc = Rnd(1, NO_POOLS);

        poolX[cyc] = (count > 0) ? x + Rnd(-5,5) : x;
        poolZ[cyc] = (count > 0) ? z + Rnd(-5,5) : z;
        poolA[cyc] = static_cast<float>(Rnd(0,360));
        poolY[cyc] = y;
        poolSize[cyc]  = size;
        poolAlpha[cyc] = 0.7f;
        poolState[cyc] = 1;
        poolType[cyc]  = style;
        ShowEntity(pool[cyc]);

        // Colour by type: 1=blood,2=foam,3=water,4=beer
        if (style==1) EntityColor(pool[cyc], Rnd(150,220), 0, 0);
        if (style==2) EntityColor(pool[cyc], 255, 255, 255);
        if (style==3) EntityColor(pool[cyc], 100, 200, 255);
        if (style==4) EntityColor(pool[cyc], 150, 50, 0);
    }
}

// ---------------------------------------------------------------------------
//  PoolCycle  (from Particles.bb)
// ---------------------------------------------------------------------------
void PoolCycle() {
    for (int cyc = 1; cyc <= NO_POOLS; ++cyc) {
        if (poolState[cyc] != 1) continue;

        PositionEntity(pool[cyc], poolX[cyc], poolY[cyc], poolZ[cyc]);
        RotateEntity(pool[cyc], 90.0f, poolA[cyc], 0.0f);

        poolAlpha[cyc] -= 0.0005f;
        if (poolY[cyc] < 0) poolAlpha[cyc] -= 0.001f;
        EntityAlpha(pool[cyc], poolAlpha[cyc]);

        poolSize[cyc] -= 0.01f;
        if (poolY[cyc] < 0) poolSize[cyc] -= 0.01f;
        ScaleSprite(pool[cyc], poolSize[cyc], poolSize[cyc]);

        if (poolSize[cyc] < 0.5f || poolAlpha[cyc] < 0.01f) {
            poolState[cyc] = 0;
            HideEntity(pool[cyc]);
        }
    }
}

// ---------------------------------------------------------------------------
//  BlastProximity  (from Particles.bb)
// ---------------------------------------------------------------------------
int BlastProximity(int cyc, float x, float y, float z, float range) {
    if (x > exX[cyc]-range && x < exX[cyc]+range &&
        z > exZ[cyc]-range && z < exZ[cyc]+range &&
        y > exY[cyc]-50.0f && y < exY[cyc]+50.0f)
        return 1;
    return 0;
}

// ---------------------------------------------------------------------------
//  CreateExplosion  (from Particles.bb)
// ---------------------------------------------------------------------------
void CreateExplosion(int source, Handle entity, float x, float y, float z, int style) {
    if (optFX <= 0) return;

    int cyc = 0;
    for (int c = 1; c <= NO_EXPLODES; ++c)
        if (exTim[c] == 0) { cyc = c; break; }
    if (cyc == 0) cyc = Rnd(1, NO_EXPLODES);

    if (style >= 11) {
        ProduceSound(entity, sExplosion, 0, 0.5f);
        ProduceSound(entity, sSplash, 22050, 0.0f);
    } else {
        ProduceSound(entity, sExplosion, 0, 1.0f);
    }

    exSource[cyc] = source;
    exType[cyc]   = style;
    exTim[cyc]    = 20;
    exX[cyc] = x; exY[cyc] = y; exZ[cyc] = z;
    for (int v = 1; v <= no_plays; ++v) exHurt[cyc][v] = 0;

    // Alert guards
    for (int c = 1; c <= no_plays; ++c) {
        if (charRole[pChar[c]] == 1) {
            pAgenda[c] = 1;
            pExploreX[c] = x; pExploreY[c] = pY[c]; pExploreZ[c] = z;
            pSubX[c] = 9999.0f; pSubZ[c] = 9999.0f;
        }
    }
}

// ---------------------------------------------------------------------------
//  ExplosionCycle  (from Particles.bb)
// ---------------------------------------------------------------------------
void ExplosionCycle() {
    for (int cyc = 1; cyc <= NO_EXPLODES; ++cyc) {
        if (exTim[cyc] <= 0) continue;

        // Blaze at key frames
        if (exTim[cyc]==20 || exTim[cyc]==15 || exTim[cyc]==10 || exTim[cyc]==5) {
            int density = (optFX <= 1) ? 12 : 25;
            for (int c = 0; c < density; ++c)
                CreateParticle(exX[cyc]+Rnd(-15,15),
                                Rnd((int)exY[cyc]-5,(int)exY[cyc]+10),
                                exZ[cyc]+Rnd(-15,15), exType[cyc]);
            density = (optFX <= 1) ? 7 : 15;
            for (int c = 0; c < density; ++c)
                CreateParticle(exX[cyc]+Rnd(-10,10), exY[cyc]+Rnd(-5,5),
                                exZ[cyc]+Rnd(-10,10), exType[cyc]);
            density = (optFX <= 1) ? 2 : 5;
            for (int c = 0; c < density; ++c)
                CreateParticle(exX[cyc]+Rnd(-5,5), exY[cyc], exZ[cyc]+Rnd(-5,5), exType[cyc]);
            density = (optFX <= 1) ? 5 : 10;
            for (int c = 0; c < density; ++c)
                CreateParticle(exX[cyc]+Rnd(-10,10), Rnd((int)exY[cyc],(int)exY[cyc]+5),
                                exZ[cyc]+Rnd(-10,10), 2);
        }

        // Pool mess at halfway
        if (exTim[cyc] == 10 && exType[cyc] >= 11)
            CreatePool(exX[cyc], 12.0f, exZ[cyc], RndF(10.0f,15.0f), 1, exType[cyc]-9);

        // Human damage
        if (exTim[cyc] >= 5 && exTim[cyc] <= 18) {
            for (int v = 1; v <= no_plays; ++v) {
                if (BlastProximity(cyc, pX[v], pY[v], pZ[v], 40))
                    pDazed[v] = Rnd(100, 300);

                if (exHurt[cyc][v] == 0 &&
                    BlastProximity(cyc, pX[v], pY[v], pZ[v], 30)) {
                    charAttacker[pChar[v]] = pChar[exSource[cyc]];

                    if (exType[cyc] == 10) {
                        ProduceSound(p[v], sBlaze, 22050, 0.5f);
                        if (pHealth[v] > 0) ProduceSound(p[v], sPain[Rnd(1,8)], 22050, 0.0f);
                        CreateSpurt(pX[v], pY[v]+30.0f, pZ[v], 5, 10, 2);
                        CreatePool(pX[v], pGround[v], pZ[v], RndF(5.0f,10.0f), 3, 1);
                        pHealth[v] -= 10;
                    }
                    pHealth[v] -= 10;
                    pHP[v] = 0;

                    if (BlastProximity(cyc, pX[v], pY[v], pZ[v], 15)) {
                        if (exType[cyc] == 10)
                            pHealth[v] -= 10;
                        // knockback
                        pY[v] += 10.0f;
                    }

                    exHurt[cyc][v] = 1;
                }
            }
        }

        exTim[cyc]--;
    }
}
