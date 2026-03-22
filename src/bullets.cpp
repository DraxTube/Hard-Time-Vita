// =============================================================================
//  bullets.cpp  –  Gun bullet system  (ported from Weapons.bb / Moves.bb)
// =============================================================================
#include "bullets.h"
#include "players.h"
#include "values.h"
#include "crimes.h"
#include "anims.h"
#include "moves.h"
#include "particles.h"
#include "functions.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <cmath>

void InitBullets() {
    // Bullets are invisible pivots that fly through the scene
    for (int i = 1; i <= NO_BULLETS; ++i) {
        bullet[i]      = CreatePivot();
        bulletState[i] = 0;
        HideEntity(bullet[i]);
    }
}

void FireBullet(int shooter, float x, float y, float z,
                float ax, float ay, float az, int weapTypeId) {
    // Find free slot
    int slot_ = 0;
    for (int i = 1; i <= NO_BULLETS; ++i)
        if (bulletState[i] == 0) { slot_ = i; break; }
    if (slot_ == 0) slot_ = Rnd(1, NO_BULLETS);

    bulletX[slot_] = x; bulletY[slot_] = y; bulletZ[slot_] = z;
    // Direction from yaw angle (ax = pitch unused for simplicity)
    bulletXA[slot_] =  Sin(ay) * 12.0f;
    bulletYA[slot_] = -ay * 0.05f;   // slight drop
    bulletZA[slot_] =  Cos(ay) * 12.0f;
    bulletState[slot_]   = 1;
    bulletTim[slot_]     = 0;
    bulletShooter[slot_] = shooter;

    PositionEntity(bullet[slot_], x, y, z);
    ShowEntity(bullet[slot_]);

    // Muzzle flash FX
    if (optFX > 0)
        CreateSpurt(x, y, z, 3, 4, 1);  // fire particles

    // Gun sound
    int wt = weapTypeId;
    if (wt > 0 && weapStyle[wt] >= 3 && sShot[1])
        ProduceSound(0, sShot[Rnd(1,5)], 22050, RndF(0.5f,1.0f));

    (void)ax; (void)az;
}

void BulletCycle() {
    for (int i = 1; i <= NO_BULLETS; ++i) {
        if (bulletState[i] == 0) continue;

        // Move bullet
        bulletX[i] += bulletXA[i];
        bulletY[i] += bulletYA[i];
        bulletZ[i] += bulletZA[i];
        PositionEntity(bullet[i], bulletX[i], bulletY[i], bulletZ[i]);
        bulletTim[i]++;

        // Lifetime: 60 frames (~2 seconds at 30fps)
        if (bulletTim[i] > 60) {
            bulletState[i] = 0;
            HideEntity(bullet[i]);
            continue;
        }

        // Hit detection against all players
        bool hit = false;
        for (int v = 1; v <= no_plays; ++v) {
            if (v == bulletShooter[i]) continue;
            if (pHealth[v] <= 0) continue;

            float dx = bulletX[i]-pX[v];
            float dy = bulletY[i]-(pY[v]+30.0f);
            float dz = bulletZ[i]-pZ[v];
            float dist = std::sqrt(dx*dx+dy*dy+dz*dz);

            if (dist < 12.0f) {
                // Determine damage from weapon type
                int shooter = bulletShooter[i];
                int wIdx    = (shooter>=1&&shooter<=no_plays) ? pWeapon[shooter] : 0;
                int dmg     = (wIdx>0) ? weapDamage[weapType[wIdx]] : 30;

                pHealth[v] -= dmg;
                pHP[v]      = 0;
                charAttacker[pChar[v]] = (shooter>=1&&shooter<=no_plays) ? pChar[shooter] : 0;

                // Sound + FX
                if (pHealth[v]>0) ProduceSound(p[v], sPain[Rnd(1,8)], 22050, 0.0f);
                ProduceSound(p[v], sImpact[Rnd(1,5)], 22050, 0.0f);
                if (optGore>=1) CreateSpurt(bulletX[i],bulletY[i],bulletZ[i],4,6,3);

                // Reaction animation
                if (pHealth[v] <= 0)
                    ChangeAnim(v, Rnd(0,1)==0 ? 78 : 79);
                else {
                    pDazed[v] = Rnd(20,60);
                    ChangeAnim(v, 70);
                }

                // Ricochet sound on wall behind
                if (sRicochet[1]) ProduceSound(0, sRicochet[Rnd(1,5)], 22050, 0.5f);

                hit = true;
                break;
            }
        }

        if (hit) {
            bulletState[i] = 0;
            HideEntity(bullet[i]);
        }
    }
}
