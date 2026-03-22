// =============================================================================
//  texts.cpp  –  All game text strings  (ported from Texts.bb)
// =============================================================================
#include "texts.h"

std::array<BBString, 5>   textWeek     {};
std::array<BBString, 13>  textMonth    {};
std::array<BBString, 5>   textBlock    {};
std::array<BBString, 12>  textLocation {};
std::array<BBString, 21>  textWarrant  {};
std::array<BBString, 21>  textCrime    {};
std::array<BBString, 66>  textFirstName{};
std::array<BBString, 66>  textSurName  {};
std::array<BBString, 101> textNickName {};

void InitTexts() {
    // Weeks
    textWeek[1]="1st week"; textWeek[2]="2nd week";
    textWeek[3]="3rd week"; textWeek[4]="4th week";

    // Months
    textMonth[1]="January";   textMonth[2]="February";  textMonth[3]="March";
    textMonth[4]="April";     textMonth[5]="May";        textMonth[6]="June";
    textMonth[7]="July";      textMonth[8]="August";     textMonth[9]="September";
    textMonth[10]="October";  textMonth[11]="November";  textMonth[12]="December";

    // Blocks
    textBlock[1]="North"; textBlock[2]="East";
    textBlock[3]="South"; textBlock[4]="West";

    // Locations
    textLocation[0]="None";         textLocation[1]="North Block";
    textLocation[2]="Exercise Yard";textLocation[3]="East Block";
    textLocation[4]="Study";        textLocation[5]="South Block";
    textLocation[6]="Hospital";     textLocation[7]="West Block";
    textLocation[8]="Canteen";      textLocation[9]="Main Hall";
    textLocation[10]="Workshop";    textLocation[11]="Bathroom";

    // Warrants
    textWarrant[0]="None";                    textWarrant[1]="Dissent";
    textWarrant[2]="Gang Membership";         textWarrant[3]="Trying To Escape";
    textWarrant[4]="Carrying An Illegal Item";textWarrant[5]="Drug Abuse";
    textWarrant[6]="Dealing";                 textWarrant[7]="Stealing";
    textWarrant[8]="Assault";                 textWarrant[9]="Assaulting A Warden";
    textWarrant[10]="Assault With A Weapon";  textWarrant[11]="Grievous Bodily Harm";
    textWarrant[12]="Attempted Murder";       textWarrant[13]="Murder";
    textWarrant[14]="Serial Murder";

    // Crimes
    textCrime[0]="None";            textCrime[1]="Fraud";
    textCrime[2]="Prostitution";    textCrime[3]="Drug Abuse";
    textCrime[4]="Drug Dealing";    textCrime[5]="Theft";
    textCrime[6]="Armed Robbery";   textCrime[7]="Vandalism";
    textCrime[8]="Assault";         textCrime[9]="Child Abuse";
    textCrime[10]="Rape";           textCrime[11]="Grievous Bodily Harm";
    textCrime[12]="Attempted Murder";textCrime[13]="Manslaughter";
    textCrime[14]="Murder";         textCrime[15]="Terrorism";

    // First names
    textFirstName[1]="James";   textFirstName[2]="John";    textFirstName[3]="Robert";
    textFirstName[4]="Michael"; textFirstName[5]="William"; textFirstName[6]="David";
    textFirstName[7]="Richard"; textFirstName[8]="Joseph";  textFirstName[9]="Thomas";
    textFirstName[10]="Charles";textFirstName[11]="Christopher"; textFirstName[12]="Daniel";
    textFirstName[13]="Matthew";textFirstName[14]="Anthony";textFirstName[15]="Donald";
    textFirstName[16]="Mark";   textFirstName[17]="Paul";   textFirstName[18]="Steven";
    textFirstName[19]="Andrew"; textFirstName[20]="Kenneth";textFirstName[21]="Joshua";
    textFirstName[22]="Kevin";  textFirstName[23]="Brian";  textFirstName[24]="George";
    textFirstName[25]="Timothy";textFirstName[26]="Ronald"; textFirstName[27]="Edward";
    textFirstName[28]="Jason";  textFirstName[29]="Jeffrey";textFirstName[30]="Ryan";
    textFirstName[31]="Jacob";  textFirstName[32]="Gary";   textFirstName[33]="Nicholas";
    textFirstName[34]="Eric";   textFirstName[35]="Jonathan";textFirstName[36]="Stephen";
    textFirstName[37]="Larry";  textFirstName[38]="Justin"; textFirstName[39]="Scott";
    textFirstName[40]="Brandon";textFirstName[41]="Benjamin";textFirstName[42]="Samuel";
    textFirstName[43]="Raymond";textFirstName[44]="Gregory";textFirstName[45]="Frank";
    textFirstName[46]="Alexander";textFirstName[47]="Patrick";textFirstName[48]="Jack";
    textFirstName[49]="Dennis"; textFirstName[50]="Jerry";  textFirstName[51]="Tyler";
    textFirstName[52]="Aaron";  textFirstName[53]="Jose";   textFirstName[54]="Adam";
    textFirstName[55]="Henry";  textFirstName[56]="Nathan"; textFirstName[57]="Douglas";
    textFirstName[58]="Zachary";textFirstName[59]="Peter";  textFirstName[60]="Kyle";
    textFirstName[61]="Walter"; textFirstName[62]="Harold"; textFirstName[63]="Jeremy";
    textFirstName[64]="Ethan";  textFirstName[65]="Carl";

    // Surnames
    textSurName[1]="Smith";   textSurName[2]="Johnson"; textSurName[3]="Williams";
    textSurName[4]="Jones";   textSurName[5]="Brown";   textSurName[6]="Davis";
    textSurName[7]="Miller";  textSurName[8]="Wilson";  textSurName[9]="Moore";
    textSurName[10]="Taylor"; textSurName[11]="Anderson";textSurName[12]="Thomas";
    textSurName[13]="Jackson";textSurName[14]="White";  textSurName[15]="Harris";
    textSurName[16]="Martin"; textSurName[17]="Garcia"; textSurName[18]="Thompson";
    textSurName[19]="Martinez";textSurName[20]="Robinson";textSurName[21]="Clark";
    textSurName[22]="Rodriguez";textSurName[23]="Lewis"; textSurName[24]="Lee";
    textSurName[25]="Walker"; textSurName[26]="Hall";   textSurName[27]="Allen";
    textSurName[28]="Young";  textSurName[29]="Hernandez";textSurName[30]="King";
    textSurName[31]="Wright"; textSurName[32]="Lopez";  textSurName[33]="Hill";
    textSurName[34]="Scott";  textSurName[35]="Green";  textSurName[36]="Adams";
    textSurName[37]="Baker";  textSurName[38]="Gonzalez";textSurName[39]="Nelson";
    textSurName[40]="Carter"; textSurName[41]="Mitchell";textSurName[42]="Perez";
    textSurName[43]="Roberts";textSurName[44]="Turner"; textSurName[45]="Phillips";
    textSurName[46]="Campbell";textSurName[47]="Parker";textSurName[48]="Evans";
    textSurName[49]="Keaton"; textSurName[50]="Steiner";textSurName[51]="Combs";
    textSurName[52]="Carter"; textSurName[53]="Bush";   textSurName[54]="Nixon";
    textSurName[55]="Mathers";textSurName[56]="Schwarz";textSurName[57]="Rajah";
    textSurName[58]="Foster"; textSurName[59]="Robson"; textSurName[60]="Manson";
    textSurName[61]="Pearce"; textSurName[62]="Epton";  textSurName[63]="Dearden";
    textSurName[64]="Mitchell";textSurName[65]="Mendoza";

    // Nicknames
    textNickName[0]="Lemonhead";    textNickName[1]="Sugar Tits";
    textNickName[2]="Hat Trick";    textNickName[3]="Deep Throat";
    textNickName[4]="Big Hit";      textNickName[5]="Super Lucha";
    textNickName[6]="Machoman";     textNickName[7]="Heavyweight";
    textNickName[8]="Thug Angel";   textNickName[9]="God's Son";
    textNickName[10]="Escobar";     textNickName[11]="Young Boy";
    textNickName[12]="Wide Boy";    textNickName[13]="Mr Tickle";
    textNickName[14]="Handyman";    textNickName[15]="Lyracist";
    textNickName[16]="Maitreya";    textNickName[17]="Piston Pecker";
    textNickName[18]="Kampas Krismas"; textNickName[19]="Baby Bull";
    textNickName[20]="Fast Eddie";  textNickName[21]="Slick Rick";
    textNickName[22]="Toadfish";    textNickName[23]="Octogon";
    textNickName[24]="Riverside";   textNickName[25]="Wussy Lee";
    textNickName[26]="Scotbird";    textNickName[27]="Thunder Lips";
    textNickName[28]="Agony Aunt";  textNickName[29]="Downtown";
    textNickName[30]="Boomtown";    textNickName[31]="Voodoo Child";
    textNickName[32]="Little Voice";textNickName[33]="Brother Bear";
    textNickName[34]="Maverick";    textNickName[35]="Sure Shank";
    textNickName[36]="Needles";     textNickName[37]="Iceman";
    textNickName[38]="Crazy Jew";   textNickName[39]="Scally";
    textNickName[40]="Wise Len";    textNickName[41]="Sunshine";
    textNickName[42]="Terminator";  textNickName[43]="Safe Hands";
    textNickName[44]="Fairytale";   textNickName[45]="Original G";
    textNickName[46]="Deep Impact"; textNickName[47]="Road Pig";
    textNickName[48]="X-Factor";    textNickName[49]="Spacker";
    textNickName[50]="Fabulous M";  textNickName[51]="Menace";
    textNickName[52]="Nasty Nas";   textNickName[53]="King Carter";
    textNickName[54]="Sure Shot";   textNickName[55]="Major Merc";
    textNickName[56]="Messiah";     textNickName[57]="King Sin";
    textNickName[58]="Farrenheit";  textNickName[59]="Roughcock";
    textNickName[60]="Syntax Error";textNickName[61]="Muhammad";
    textNickName[62]="Zansibar";    textNickName[63]="Bent Rat";
    textNickName[64]="Kid Gloves";  textNickName[65]="Third Eye";
    textNickName[66]="Tin Ear";     textNickName[67]="Iron Mic";
    textNickName[68]="Ghetto Child";textNickName[69]="Bang Bang";
    textNickName[70]="Apocolypto";  textNickName[71]="Warrior";
    textNickName[72]="Big Pussy";   textNickName[73]="Duke Nukem";
    textNickName[74]="Body Bag";    textNickName[75]="Cum Bucket";
    textNickName[76]="Steroid Roy"; textNickName[77]="Bulletproof";
    textNickName[78]="Stone Malone";textNickName[79]="Assassin";
    textNickName[80]="Nightmare";
}
