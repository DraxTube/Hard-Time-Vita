// =============================================================================
//  moves.cpp  –  Movement physics  (ported from Moves.bb)
//  Handles walking, running, jumping, stagger, gravity, door interaction
// =============================================================================
#include "moves.h"
#include "players.h"
#include "anims.h"
#include "values.h"
#include "functions.h"
#include "bullets.h"
#include "render3d.h"
#include "blitz_compat.h"
#include "world_props.h"
#include "particles.h"
#include "crimes.h"
#include <cmath>

// ---------------------------------------------------------------------------
//  Extra movement arrays (all declared here, extern'd via players.h extension)
// ---------------------------------------------------------------------------
std::array<float, P> pOldY       {};
std::array<float, P> pLeaveX     {};
std::array<float, P> pLeaveY     {};
std::array<float, P> pLeaveZ     {};
std::array<float, P> pLeaveA     {};
std::array<float, P> pHurtA      {};
std::array<float, P> pStagger    {};
std::array<float, P> pSpeed      {};
std::array<int,   P> pSting      {};
std::array<int,   P> pOldHealth  {};
std::array<int,   P> pFireTim    {};
std::array<int,   P> pTarget     {};
std::array<int,   P> pExploreRange{};
std::array<int,   P> pOldAnim    {};
std::array<int,   P> pFoodTim    {};
std::array<std::array<int, 11>, P> pDoorFriction {};
std::array<std::array<int, P>,  P> pMultiSting   {};
std::array<int, 3> pHighlight    {};

// Camera
Handle camPivot  = 0;
int    camOldFoc = 0;
float  camX = 0.0f, camY = 75.0f, camZ = 0.0f;

// Weapon extra
std::array<float, WL> weapOldY     {};
std::array<int,   WLS> weapHabitat {};
std::array<int,   WLS> weapCreate  {};

// World textures
Handle tShower = 0, tCrowd = 0;

// Extra sounds
std::array<BBSound*, 7>  sStep     {};
std::array<BBSound*, 6>  sAgony    {};
std::array<BBSound*, 6>  sShot     {};
std::array<BBSound*, 6>  sRicochet {};

// ---------------------------------------------------------------------------
//  LoadMoveSounds  –  call once during world load
// ---------------------------------------------------------------------------
void LoadMoveSounds() {
    const std::string S = "ux0:data/HardTime/Sound/";
    for (int i=1;i<=6;++i) sStep[i]     = Load3DSound(S+"Movement/Step0"+Str(i)+".wav");
    for (int i=1;i<=5;++i) sAgony[i]    = Load3DSound(S+"Movement/Agony0"+Str(i)+".wav");
    for (int i=1;i<=5;++i) sShot[i]     = Load3DSound(S+"Props/Shot0"+Str(i)+".wav");
    for (int i=1;i<=5;++i) sRicochet[i] = Load3DSound(S+"Props/Ricochet0"+Str(i)+".wav");
}

// ---------------------------------------------------------------------------
//  Helper: clamp position to world bounds
// ---------------------------------------------------------------------------
static void ClampToWorld(int cyc) {
    // Simple rectangular prison yard bounds – extend per-location as needed
    static constexpr float BOUND = 340.0f;
    if (pX[cyc] >  BOUND) pX[cyc] =  BOUND;
    if (pX[cyc] < -BOUND) pX[cyc] = -BOUND;
    if (pZ[cyc] >  BOUND) pZ[cyc] =  BOUND;
    if (pZ[cyc] < -BOUND) pZ[cyc] = -BOUND;
}

// ---------------------------------------------------------------------------
//  MovePlayers  –  full movement update each frame
// ---------------------------------------------------------------------------
void MovePlayers() {
    for (int cyc = 1; cyc <= no_plays; ++cyc) {
        if (p[cyc] == 0) continue;

        // --- Gravity ---
        if (pSeat[cyc] == 0 && pBed[cyc] == 0) {
            // Apply gravity if above ground
            if (pY[cyc] > pGround[cyc]) {
                pGravity[cyc] += 0.4f;
                if (pGravity[cyc] > 15.0f) pGravity[cyc] = 15.0f;
                pY[cyc] -= pGravity[cyc];
            } else {
                pY[cyc] = pGround[cyc];
                if (pGravity[cyc] > 6.0f) {
                    // Hard landing
                    pDazed[cyc] = std::max(pDazed[cyc], 30);
                    ProduceSound(p[cyc], sFall, 22050, 0.0f);
                }
                pGravity[cyc] = 1.0f;
            }
        }

        // --- Stagger bleed-off ---
        if (pStagger[cyc] > 0.0f) {
            pStagger[cyc] *= 0.85f;
            if (pStagger[cyc] < 0.2f) pStagger[cyc] = 0.0f;
        }

        // --- Dazed bleed-off ---
        if (pDazed[cyc] > 0) pDazed[cyc]--;

        // Skip movement for seated/bedded characters
        if (pSeat[cyc] > 0 || pBed[cyc] > 0) continue;
        // Skip if grappled by someone else
        if (pGrappler[cyc] > 0) continue;

        // --- Speed determination ---
        float speed = 0.0f;
        bool moving = false;

        if (pAnim[cyc] == 12 || pAnim[cyc] == 16) { speed = 1.8f; moving = true; }  // walk
        if (pAnim[cyc] == 13 || pAnim[cyc] == 18) { speed = 3.5f; moving = true; }  // run
        if (pAnim[cyc] == 14 || pAnim[cyc] == 15) { speed = 1.0f; moving = true; }  // injured/dazed walk
        if (pAnim[cyc] >= 200 && pAnim[cyc] <= 210) moving = false; // grapple moves

        if (moving && speed > 0.0f) {
            // Rotate
            if (cLeft[cyc]  && !cUp[cyc] && !cDown[cyc])
                pA[cyc] = CleanAngle(pA[cyc] + (pDazed[cyc]>0 ? 3.0f : 6.0f));
            if (cRight[cyc] && !cUp[cyc] && !cDown[cyc])
                pA[cyc] = CleanAngle(pA[cyc] - (pDazed[cyc]>0 ? 3.0f : 6.0f));

            // Strafe while walking
            if (cLeft[cyc]  && cUp[cyc])  pA[cyc] = CleanAngle(pA[cyc] + 3.0f);
            if (cRight[cyc] && cUp[cyc])  pA[cyc] = CleanAngle(pA[cyc] - 3.0f);

            // Forward / backward
            float dir = cDown[cyc] ? 1.0f : -1.0f;
            if (!cUp[cyc] && !cDown[cyc]) dir = 0.0f;

            pX[cyc] += Sin(pA[cyc]) * speed * dir;
            pZ[cyc] += Cos(pA[cyc]) * speed * dir;

            // Step sound (every ~20 frames)
            if (pStepTim[cyc] % 20 == 0 && sStep[1])
                ProduceSound(p[cyc], sStep[Rnd(1,6)], 22050, RndF(0.3f,0.7f));
        }

        // --- Apply stagger displacement ---
        if (pStagger[cyc] > 0.0f) {
            pX[cyc] += Sin(pHurtA[cyc]) * pStagger[cyc];
            pZ[cyc] += Cos(pHurtA[cyc]) * pStagger[cyc];
        }

        ClampToWorld(cyc);

        // --- Update 3D pivot position ---
        PositionEntity(pPivot[cyc],     pX[cyc], pY[cyc]+18.0f, pZ[cyc]);
        PositionEntity(pMovePivot[cyc], pX[cyc], pY[cyc]+18.0f, pZ[cyc]);
        RotateEntity(pPivot[cyc], 0.0f, pA[cyc], 0.0f);

        // --- Update mesh position ---
        PositionEntity(p[cyc], pX[cyc], pY[cyc], pZ[cyc]);
        RotateEntity(p[cyc],   0.0f, pA[cyc], 0.0f);

        // --- Update shadows ---
        for (int limb = 1; limb <= 40; ++limb) {
            if (pShadow[cyc][limb] > 0)
                PositionEntity(pShadow[cyc][limb], pX[cyc], pGround[cyc], pZ[cyc]);
        }

        // --- Transition to appropriate movement anim ---
        if (pAnim[cyc] == 0) {
            // Standing: check if player pressed direction
            if (cUp[cyc] || cDown[cyc]) {
                if (pAnim[cyc] == 0) ChangeAnim(cyc, (pRunTim[cyc]>10) ? 13 : 12);
            } else if (cLeft[cyc] || cRight[cyc]) {
                ChangeAnim(cyc, 10);  // turning
            }
        }

        // --- Transition from walking to run when sustained movement ---
        if (pAnim[cyc] == 12 && pAnimTim[cyc] > 15 && cUp[cyc] && !cDown[cyc])
            ChangeAnim(cyc, 13);

        // --- Count run time for acceleration ---
        if (pAnim[cyc] == 13) pRunTim[cyc] = 30;
        else if (pRunTim[cyc] > 0) pRunTim[cyc]--;

        // --- Attack input → trigger attack anim ---
        if (cAttack[cyc] && pGrappling[cyc] == 0 && pAnim[cyc] < 30 && pAnimTim[cyc] > 5) {
            int style = (pWeapon[cyc] > 0) ? weapStyle[weapType[pWeapon[cyc]]] : 0;
            if (style == 0) {
                int atk = Rnd(0,2);
                if (atk==0) ChangeAnim(cyc, 30);
                if (atk==1) ChangeAnim(cyc, 31);
                if (atk==2) ChangeAnim(cyc, 33);
            } else if (style == 1) {
                int atk = Rnd(0,3);
                if (atk==0) ChangeAnim(cyc, 40);
                if (atk==1) ChangeAnim(cyc, 41);
                if (atk==2) ChangeAnim(cyc, 43);
                if (atk==3) ChangeAnim(cyc, 44);
            } else if (style == 7) {
                ChangeAnim(cyc, Rnd(0,1)==0 ? 51 : 53);
            } else if (style == 3 || style == 4) {
                // Gun: play fire anim AND spawn bullet
                ChangeAnim(cyc, style==4 ? 63 : 64);
                if (pWeapon[cyc] > 0 && weapAmmo[pWeapon[cyc]] > 0) {
                    weapAmmo[pWeapon[cyc]]--;
                    FireBullet(cyc,
                        pX[cyc], pY[cyc]+40.0f, pZ[cyc],
                        0.0f, pA[cyc], 0.0f,
                        weapType[pWeapon[cyc]]);
                }
            }
        }

        // --- Defend → grapple lunge ---
        if (cDefend[cyc] && pGrappling[cyc] == 0 && pAnim[cyc] < 30 && pAnimTim[cyc] > 5) {
            if (pFoc[cyc] > 0) ChangeAnim(cyc, 200);  // standing grapple lunge
        }

        // --- Pick up weapon ---
        if (cPickUp[cyc] && pWeapon[cyc] == 0 && pAnimTim[cyc] > 5) {
            // Search for nearby dropped weapon
            for (int w = 1; w <= no_weaps; ++w) {
                if (weapLocation[w] != gamLocation[slot]) continue;
                if (weapState[w] == 0 || weapCarrier[w] != 0) continue;
                float dx = pX[cyc]-weapX[w], dz = pZ[cyc]-weapZ[w];
                if (dx*dx+dz*dz < 30.0f*30.0f) {
                    pWeapon[cyc]  = w;
                    weapCarrier[w]= cyc;
                    weapState[w]  = 1;
                    ChangeAnim(cyc, 20);
                    break;
                }
            }
        }

        // --- Throw weapon ---
        if (cThrow[cyc] && pWeapon[cyc] > 0 && pAnimTim[cyc] > 5) {
            ChangeAnim(cyc, 22);
            int w = pWeapon[cyc];
            weapCarrier[w] = 0;
            pWeapon[cyc]   = 0;
            // Launch weapon forward
            weapFlight[w]  = 5.0f;
            weapFlightA[w] = pA[cyc];
            weapGravity[w] = -1.0f;
            weapState[w]   = 1;
            ShowEntity(weap[w]);
        }
        // --- Door interaction ---
        if (pChar[cyc] == gamChar[slot] && gamDoor == 0 && pAnim[cyc] < 20 && pDazed[cyc] == 0) {
            for (int door = 1; door <= no_doors; ++door) {
                pDoorFriction[cyc][door]--;
                if (pDoorFriction[cyc][door] < 0) pDoorFriction[cyc][door] = 0;
                
                int loc = gamLocation[slot];
                if (loc >= 0 && loc < MAX_LOCS_D && door < MAX_DOORS) {
                    if (pX[cyc] > doorX1[loc][door] && pX[cyc] < doorX2[loc][door] &&
                        pY[cyc] > doorY1[loc][door] && pY[cyc] < doorY2[loc][door] &&
                        pZ[cyc] > doorZ1[loc][door] && pZ[cyc] < doorZ2[loc][door]) {
                        
                        // Check angle towards door
                        if (cUp[cyc]) { // && SatisfiedAngle(pA[cyc], doorA[loc][door], 45.0f))
                            pDoorFriction[cyc][door] += 2;
                        }
                    }
                    if (pDoorFriction[cyc][door] > 10) {
                        ChangeAnim(cyc, 90);
                        gamDoor = door;
                    }
                }
            }
        }

        // --- Chair interaction ---
        if (no_chairs > 0 && pAnim[cyc] < 20 && pDazed[cyc] == 0) {
            for (int chair = 1; chair <= no_chairs; ++chair) {
                pSeatFriction[cyc][chair]--;
                if (pSeatFriction[cyc][chair] < 0) pSeatFriction[cyc][chair] = 0;
                
                if (ChairProximity(cyc, chair) && ChairTaken(chair) == 0) {
                    if (cUp[cyc]) pSeatFriction[cyc][chair] += 2;
                }
                
                if (pSeatFriction[cyc][chair] > 10) {
                    ChangeAnim(cyc, 100);
                    pSeat[cyc] = chair;
                    pLeaveX[cyc] = pX[cyc];
                    pLeaveZ[cyc] = pZ[cyc];
                    pLeaveY[cyc] = pY[cyc] + 5.0f;
                    pLeaveA[cyc] = CleanAngle(pA[cyc] + 180.0f);
                    
                    if (gamLocation[slot] == 11) {
                        float dummyX = pX[cyc] + Sin(pA[cyc]) * -5.0f;
                        float dummyZ = pZ[cyc] + Cos(pA[cyc]) * -5.0f;
                        pLeaveX[cyc] = dummyX;
                        pLeaveZ[cyc] = dummyZ;
                    }
                }
            }
        }
        
        // --- Bed interaction ---
        if (no_beds > 0 && pAnim[cyc] < 20 && pDazed[cyc] == 0) {
            for (int bed = 1; bed <= no_beds; ++bed) {
                pBedFriction[cyc][bed]--;
                if (pBedFriction[cyc][bed] < 0) pBedFriction[cyc][bed] = 0;
                
                if (BedProximity(cyc, bed) && BedTaken(bed) == 0) {
                    if (cUp[cyc]) pBedFriction[cyc][bed] += 2;
                }
                
                if (pBedFriction[cyc][bed] > 10) {
                    ChangeAnim(cyc, 100);
                    pBed[cyc] = bed;
                    float dummyX = pX[cyc] + Sin(pA[cyc]) * -5.0f;
                    float dummyZ = pZ[cyc] + Cos(pA[cyc]) * -5.0f;
                    pLeaveX[cyc] = dummyX;
                    pLeaveZ[cyc] = dummyZ;
                    pLeaveY[cyc] = pY[cyc] + 5.0f;
                    pLeaveA[cyc] = CleanAngle(pA[cyc] + 180.0f);
                }
            }
        }
        
        // --- Leave bed/seat if interrupted ---
        if (pSeat[cyc] > 0 || pBed[cyc] > 0) {
            if (pAnim[cyc] < 100 || pAnim[cyc] > 110) ChangeAnim(cyc, 101);
        }

        // --- Apply attack damage in hit window ---
        if ((pAnim[cyc] >= 30 && pAnim[cyc] <= 53) &&
            (pAnimTim[cyc] >= 8 && pAnimTim[cyc] <= 14)) {
            int wType  = (pWeapon[cyc] > 0) ? weapType[pWeapon[cyc]] : 0;
            int damage = (wType > 0) ? weapDamage[wType] : 10 + charStrength[pChar[cyc]]/10;
            float reach = (wType > 0) ? weapRange[wType] : 18.0f;

            for (int v = 1; v <= no_plays; ++v) {
                if (v == cyc) continue;
                if (pSting[v] > 0) continue;  // already hit this swing
                float dx = pX[v]-pX[cyc], dz = pZ[v]-pZ[cyc];
                if (std::sqrt(dx*dx+dz*dz) > reach) continue;
                // Check facing
                float ang = ATan2(dx, dz);
                if (std::fabs(CleanAngle(ang - pA[cyc])) > 60.0f) continue;

                // Apply hit
                pHealth[v]  -= damage;
                pHP[v]       = 0;
                pSting[v]    = 10;
                pHurtA[v]    = pA[cyc];
                pStagger[v]  = 6.0f;
                charAttacker[pChar[v]] = pChar[cyc];

                ProduceSound(p[v], sImpact[Rnd(1,5)], 22050, 0.0f);
                if (pHealth[v] > 0) ProduceSound(p[v], sPain[Rnd(1,8)], 22050, 0.0f);

                // Blood FX
                if (optGore >= 1)
                    CreateSpurt(pX[v], pY[v]+30.0f, pZ[v], 4, 6, 3);

                // Reaction
                if (AttackViable(v) == 3) {
                    // Already on ground
                    ChangeAnim(v, 72);
                } else {
                    if (pHealth[v] <= 0) ChangeAnim(v, Rnd(0,1)==0 ? 78 : 79);
                    else ChangeAnim(v, pAnim[cyc]<=33 ? 70 : 71);
                }
            }
        }

        // --- Decrement sting immunity ---
        if (pSting[cyc] > 0) pSting[cyc]--;
    }
}
