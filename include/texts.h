#pragma once
// =============================================================================
//  texts.h  –  All game text strings  (from Texts.bb)
// =============================================================================
#include "blitz_compat.h"
#include <array>

extern std::array<BBString, 5>   textWeek;
extern std::array<BBString, 13>  textMonth;
extern std::array<BBString, 5>   textBlock;
extern std::array<BBString, 12>  textLocation;
extern std::array<BBString, 21>  textWarrant;
extern std::array<BBString, 21>  textCrime;
extern std::array<BBString, 66>  textFirstName;
extern std::array<BBString, 66>  textSurName;
extern std::array<BBString, 101> textNickName;

void InitTexts();   // populates all arrays
