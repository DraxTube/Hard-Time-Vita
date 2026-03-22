#pragma once
// =============================================================================
//  bullets.h  –  Projectile / gun bullet system  (from Weapons.bb / Moves.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"

void InitBullets();
void FireBullet(int shooter, float x, float y, float z,
                float ax, float ay, float az, int weapTypeId);
void BulletCycle();
