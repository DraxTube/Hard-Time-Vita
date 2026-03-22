#pragma once
// =============================================================================
//  data.h  –  Save / Load / Generate function declarations  (from Data.bb)
// =============================================================================
#include "blitz_compat.h"

// ---------------------------------------------------------------------------
//  Save / Load
// ---------------------------------------------------------------------------
void SaveOptions();
void LoadOptions();
void SaveProgress();
void LoadProgress();
void SaveChars();
void LoadChars();
void SaveItems();
void LoadItems();
void SavePhotos();
void LoadPhotos();

// ---------------------------------------------------------------------------
//  Generation  (new-game creation)
// ---------------------------------------------------------------------------
void GenerateGame();
void AssignCell(int charId);
void FindCellMates();

// ---------------------------------------------------------------------------
//  Population helpers  (used by generation & AI)
// ---------------------------------------------------------------------------
int  CellPopulation(int block, int cell);
int  AreaPopulation(int area, int role);
int  TranslateBlock(int block);
int  FindCarrier(int cyc);
