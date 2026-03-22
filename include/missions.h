#pragma once
// =============================================================================
//  missions.h  –  Mission system  (from Missions.bb)
// =============================================================================
#include "blitz_compat.h"

void MissionCycle();                 // called from gameplay loop
void AssignMission(int cyc, int id); // set mission data
int  GetClient(int cyc, int v);      // 0=phone,1=stranger,2=gang,3=warden
int  CheckMission();                 // 1 if current mission complete
void MissionComplete();
void MissionFail();
