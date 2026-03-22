#pragma once
// =============================================================================
//  anims.h  –  Animation state machine  (from Anims.bb)
// =============================================================================
#include "blitz_compat.h"
#include "render3d.h"

// Extra per-player state needed by Anims.bb
extern std::array<int,   51> pDazed;
extern std::array<int,   51> pFoc;
extern std::array<int,   51> pPromoState;
extern std::array<int,   51> pSpeaking;
extern std::array<int,   51> pStepTim;
extern std::array<float, 51> pOldMoveX, pOldMoveZ;
extern std::array<int,   51> pDT;          // damage timer
extern std::array<Handle, 11> light;       // scene lights

// Load/extract animation sequences for a player
void LoadAnimSequences(int cyc);     // wraps Anims.bb LoadSequences()
void LoadMoveSequences(int cyc);     // wraps Moves.bb LoadMoveSequences()

// Per-frame animation update (call once per player per frame)
void Animations(int cyc);
void MoveAnims(int cyc);

// Animation helpers
void ChangeAnim(int cyc, int anim);
void SharpTransition(int cyc, int anim, float angle);
int  DirPressed(int cyc);
int  HorizontalPressed(int cyc);
int  VerticalPressed(int cyc);
int  ActionPressed(int cyc);
void FaceEntity(int cyc, Handle target, float speed);
void FixGrapple(int cyc, int victim);
void FindMoveCommands(int cyc);
void FacialExpressions(int cyc);

// LoadAnimSeq stub  (Blitz3D returns an int sequence index)
int LoadAnimSeq(Handle mesh, const BBString& path);
int ExtractAnimSeq(Handle mesh, int startFrame, int endFrame, int srcSeq);

// AnimatePlayers - called from gameplay loop
void AnimatePlayers();
