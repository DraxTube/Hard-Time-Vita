#pragma once
// =============================================================================
//  menus.h  –  Menu screen declarations  (from Menus.bb)
// =============================================================================
#include "blitz_compat.h"

void MainMenu();
void Options();
void RedefineKeys();
void RedefineGamepad();
void SlotSelect();
void Credits();
void Outro();
void EditSelect();

// Weapons data loader (from Weapons.bb)
void LoadWeaponData();
