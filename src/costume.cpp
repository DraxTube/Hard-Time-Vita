// =============================================================================
//  costume.cpp  –  Character appearance + helper functions  (from Editor.bb)
// =============================================================================
#include "costume.h"
#include "players.h"
#include "values.h"
#include "texts.h"
#include "functions.h"
#include "render3d.h"
#include "blitz_compat.h"
#include <cmath>

// ---------------------------------------------------------------------------
//  Texture handle arrays
// ---------------------------------------------------------------------------
std::array<GLuint, 21>  tHair         {};
std::array<GLuint, 101> tFace         {};
std::array<GLuint, 101> tBody         {};
std::array<GLuint, 101> tArm          {};
std::array<GLuint, 101> tLegs         {};
std::array<GLuint, 11>  tBodyShade    {};
std::array<GLuint, 11>  tArmShade     {};
std::array<GLuint, 4>   tSpecs        {};
std::array<GLuint, 6>   tMouth        {};
std::array<GLuint, 6>   tFaceScar     {};
std::array<GLuint, 6>   tBodyScar     {};
std::array<GLuint, 6>   tArmScar      {};
std::array<GLuint, 7>   tTattooBody   {};
std::array<GLuint, 7>   tTattooVest   {};
std::array<GLuint, 7>   tTattooArm    {};
std::array<GLuint, 7>   tTattooTee    {};
std::array<GLuint, 7>   tTattooSleeve {};
GLuint tShaved = 0, tEars = 0, tSeverEars = 0;

// ---------------------------------------------------------------------------
//  GetRace  –  0=white,1=black,2=asian,3=arab  (based on face texture index)
// ---------------------------------------------------------------------------
int GetRace(int c) {
    int f = charFace[c];
    if (f >= 1  && f <= 20) return 0;  // white
    if (f >= 21 && f <= 40) return 1;  // black
    if (f >= 41 && f <= 60) return 2;  // asian
    if (f >= 61 && f <= 80) return 3;  // arab
    return 0;
}
BBString GetRaceStr(int c) {
    static const char* r[]={"White","Black","Asian","Arab"};
    return r[GetRace(c)];
}

// ---------------------------------------------------------------------------
//  BaggyTop  –  true if costume uses the baggy body mesh
// ---------------------------------------------------------------------------
int BaggyTop(int costume) {
    return (costume==2||costume==4||costume==6||costume==8) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  CellName  –  return display name (nickname or full name)
// ---------------------------------------------------------------------------
BBString CellName(int c) {
    if (c<=0||c>=(int)charName.size()) return "Unknown";
    return charName[c];
}

// (InsideCell and CellVisible defined in world_props.cpp)

// ---------------------------------------------------------------------------
//  ApplyHair  (from Editor.bb ApplyHair())
// ---------------------------------------------------------------------------
void ApplyHair(int cyc) {
    int c = pChar[cyc];
    int hs = charHairStyle[c];

    // Hide all hair meshes first
    static const char* hairParts[] = {
        "Hair_Bald","Hair_Short","Hair_Long","Hair_Raise",
        "Hair_Quiff","Hair_Mop","Hair_Thick","Hair_Thin",
        "Hair_Dreads","Hair_Afro","Hair_Pony","Hair_Cornrow",
        nullptr
    };
    for (int i=0; hairParts[i]; ++i)
        HideEntity(FindChild(p[cyc], hairParts[i]));

    BBString hairerA, hairerB;
    int showA=0, showB=0;

    // Map style index to mesh names (abbreviated – full 31 styles)
    if (hs==0)                   { /* bald */ }
    else if (hs==1)              { hairerA="Hair_Bald"; showA=1; }
    else if (hs>=2  && hs<=5)    { hairerA="Hair_Short"; showA=1; }
    else if (hs>=6  && hs<=9)    { hairerA="Hair_Short"; hairerB="Hair_Raise"; showA=1; showB=1; }
    else if (hs>=10 && hs<=13)   { hairerA="Hair_Quiff"; showA=1; }
    else if (hs>=14 && hs<=17)   { hairerA="Hair_Afro"; showA=1; }
    else if (hs>=18 && hs<=21)   { hairerA="Hair_Dreads"; showA=1; }
    else if (hs>=22 && hs<=25)   { hairerA="Hair_Pony"; showA=1; }
    else if (hs>=26 && hs<=31)   { hairerA="Hair_Long"; showA=1; }

    // Tuck hair under hat/turban
    if (charAccessory[c]==2 || charAccessory[c]==7) {
        if (hs >= 2) hairerA = "Hair_Bald";
    }

    GLuint hairTex = (charHair[c]>0 && charHair[c]<(int)tHair.size())
                     ? tHair[charHair[c]] : 0;

    if (showA) {
        Handle h = FindChild(p[cyc], hairerA);
        if (h) { EntityAlpha(h, 1.0f); if (hairTex) EntityTexture(h, hairTex); ShowEntity(h); }
    }
    if (showB) {
        Handle h = FindChild(p[cyc], hairerB);
        if (h) { EntityAlpha(h, 1.0f); if (hairTex) EntityTexture(h, hairTex); ShowEntity(h); }
    }

    // Shaved layer
    Handle hHead = FindChild(p[cyc], "Head");
    if (hHead) {
        if (hs==1||hs==14||hs==24) EntityTexture(hHead, tShaved, 0, 2);
        else                        EntityTexture(hHead, tMouth[0], 0, 2);
    }
}

// ---------------------------------------------------------------------------
//  ApplyEyewear  (from Editor.bb)
// ---------------------------------------------------------------------------
void ApplyEyewear(int cyc) {
    int c = pChar[cyc];
    HideEntity(FindChild(p[cyc], "Specs"));
    HideEntity(FindChild(p[cyc], "Lens01"));
    HideEntity(FindChild(p[cyc], "Lens02"));

    if (charSpecs[c] <= 0) return;

    Handle hSpecs = FindChild(p[cyc], "Specs");
    if (hSpecs) {
        ShowEntity(hSpecs);
        EntityShininess(hSpecs, 0.5f);
        if (charSpecs[c]>=1 && charSpecs[c]<=3 && tSpecs[charSpecs[c]])
            EntityTexture(hSpecs, tSpecs[charSpecs[c]]);
    }
    for (int i=1;i<=2;++i) {
        Handle h = FindChild(p[cyc], "Lens0" + Str(i));
        if (h) {
            ShowEntity(h);
            EntityShininess(h, 1.0f);
            if (charSpecs[c]==4) { EntityColor(h,0,0,0); EntityAlpha(h,0.75f); }
            else                  { EntityColor(h,255,255,255); EntityAlpha(h,0.35f); }
        }
    }
}

// ---------------------------------------------------------------------------
//  ApplyAccessories  (from Editor.bb)
// ---------------------------------------------------------------------------
void ApplyAccessories(int cyc) {
    int c = pChar[cyc];
    static const char* acc[] = {"Turban","Bling","Tie","BandA","BandB","Armband","Cap",nullptr};
    for (int i=0; acc[i]; ++i) HideEntity(FindChild(p[cyc], acc[i]));

    if (charAccessory[c]==2) ShowEntity(FindChild(p[cyc],"Turban"));
    if (charAccessory[c]==3) { ShowEntity(FindChild(p[cyc],"Bling")); EntityShininess(FindChild(p[cyc],"Bling"),1.0f); }
    if (charAccessory[c]==4) ShowEntity(FindChild(p[cyc],"Tie"));
    if (charAccessory[c]==5) {
        ShowEntity(FindChild(p[cyc],"BandA"));
        int hs=charHairStyle[c];
        if (hs<=1||hs==13||hs==14||hs==23||hs==24) ShowEntity(FindChild(p[cyc],"BandB"));
    }
    if (charAccessory[c]==6) ShowEntity(FindChild(p[cyc],"Armband"));
    if (charAccessory[c]==7) ShowEntity(FindChild(p[cyc],"Cap"));
}

// ---------------------------------------------------------------------------
//  ApplyClothing  (from Editor.bb)
// ---------------------------------------------------------------------------
void ApplyClothing(int cyc) {
    int c = pChar[cyc];
    int race = GetRace(c);

    // Face
    GLuint faceTex = (charFace[c]>0 && charFace[c]<(int)tFace.size()) ? tFace[charFace[c]] : 0;
    for (int limb=1;limb<=2;++limb) {
        Handle h=pLimb[cyc][limb];
        if (h && faceTex) EntityTexture(h, faceTex, 0, 1);
    }
    // Ears
    for (int limb=37;limb<=38;++limb) {
        Handle h=pLimb[cyc][limb];
        if (h) {
            if (faceTex) EntityTexture(h, faceTex, 0, 1);
            if (tEars)   EntityTexture(h, tEars, 0, 3);
        }
    }

    // Body
    Handle hBody = pLimb[cyc][3];
    if (hBody) {
        GLuint bodyTex = 0;
        int role = charRole[c];
        if (role==0) {
            int cos = charCostume[c];
            if (cos==0)        bodyTex = (charStrength[c]<70) ? tBody[10] : tBody[1];
            else if (cos<=2)   bodyTex = tBody[2];
            else if (cos<=4)   bodyTex = tBody[3];
            else               bodyTex = tBody[4+charBlock[c]];
        } else if (role==1) bodyTex = tBody[4];
        else                bodyTex = tBody[9];
        if (bodyTex) EntityTexture(hBody, bodyTex, 0, 1);
        if (race>0 && tBodyShade[race]) EntityTexture(hBody, tBodyShade[race], 0, 2);
        if (charGang[c]>0) {
            int cos=charCostume[c];
            GLuint tattTex=0;
            if (cos<=0) tattTex=tTattooBody[charGang[c]];
            else if(cos<=2) tattTex=tTattooVest[charGang[c]];
            if (tattTex) EntityTexture(hBody, tattTex, 0, 3);
        }
        // Toggle baggy body mesh
        Handle hBaggy = FindChild(p[cyc],"Body_Baggy");
        Handle hFit   = FindChild(p[cyc],"Body");
        if (BaggyTop(charCostume[c])) { if(hBaggy)ShowEntity(hBaggy); if(hFit)HideEntity(hFit); }
        else                           { if(hFit)ShowEntity(hFit);     if(hBaggy)HideEntity(hBaggy); }
    }

    // Arms (limbs 4-29)
    for (int limb=4; limb<=29; ++limb) {
        Handle h=pLimb[cyc][limb];
        if (!h) continue;
        int cos=charCostume[c];
        GLuint armTex=0;
        if (charRole[c]==0) {
            if (cos<=2)       armTex=tArm[1];
            else if (cos<=4)  armTex=tArm[2];
            else if (cos<=6)  armTex=tArm[3+charBlock[c]];
            else if (cos<=8)  armTex=tArm[7+charBlock[c]];
        } else if (charRole[c]==1) armTex=tArm[3];
        else                       armTex=tArm[12];
        if (armTex) EntityTexture(h, armTex, 0, 1);
        if (race>0 && tArmShade[race]) EntityTexture(h, tArmShade[race], 0, 2);
        if (charGang[c]>0 && charCostume[c]<=2 && tTattooArm[charGang[c]])
            EntityTexture(h, tTattooArm[charGang[c]], 0, 3);
    }

    // Legs (limbs 30-36)
    for (int limb=30; limb<=36; ++limb) {
        Handle h=pLimb[cyc][limb];
        if (!h) continue;
        GLuint legTex=0;
        if (charRole[c]==0)      legTex=tLegs[1+charBlock[c]];
        else if (charRole[c]==1) legTex=tLegs[1];
        else                     legTex=tLegs[6];
        if (legTex) EntityTexture(h, legTex, 0, 1);
    }
}

// ---------------------------------------------------------------------------
//  GangAdjust  (from Editor.bb)
// ---------------------------------------------------------------------------
void GangAdjust(int c) {
    if (charRole[c]==0) charAccessory[c]=charGang[c];
    if (charGang[c]==1 && charHairStyle[c]>1) { charHairStyle[c]=Rnd(0,1); charSpecs[c]=4; }
    if (charGang[c]==5 && charCostume[c]>2)   charCostume[c]=Rnd(0,2);
}

// ---------------------------------------------------------------------------
//  ApplyCostume  –  apply all appearance elements at once
// ---------------------------------------------------------------------------
void ApplyCostume(int cyc) {
    if (p[cyc] == 0) return;
    ApplyClothing(cyc);
    ApplyHair(cyc);
    ApplyEyewear(cyc);
    ApplyAccessories(cyc);
}

// ---------------------------------------------------------------------------
//  ReloadModel  –  free and recreate the 3D mesh for a player in the editor
// ---------------------------------------------------------------------------
void ReloadModel(int cyc) {
    if (p[cyc]) { FreeEntity(p[cyc]); p[cyc]=0; }

    const std::string BASE = "ux0:data/HardTime/Characters/Models/";
    p[cyc] = LoadAnimMesh(BASE+"Model"+Dig(charModel[pChar[cyc]],10)+".3ds", 0);

    float sc = charHeight[pChar[cyc]] * 0.0025f;
    ScaleEntity(p[cyc], 0.34f+sc, 0.34f+sc, 0.34f+sc);
    PositionEntity(p[cyc], pX[cyc], pY[cyc], pZ[cyc]);
    RotateEntity(p[cyc], 0.0f, pA[cyc], 0.0f);
    ApplyCostume(cyc);
}

// (GenerateCharacter defined in data.cpp)
