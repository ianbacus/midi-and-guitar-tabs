#include "tuning.h"

//Recompile the project after updating the tuning of the instrument

#define  SEVENSTRING

#ifdef SEVENSTRING
uint8_t tuning[] = {23, 28, 33, 38, 43, 47,52 };
char ptuning[] = "BEADGBe";
const int tuningMinimum = 23;


#define SIZEOF_THE_TUNING 7


#endif
#ifdef STANDARD
char tuning[] = {28, 33, 38, 43, 47, 52};
char ptuning[] = "eadgbe";
const int tuningMinimum = 28;

#endif
#ifdef DROPD
char tuning[] = {26, 33, 38, 43, 47, 52};
char ptuning[] = "dadgbe";
const int tuningMinimum = 26;

#endif
#ifdef UKE
char tuning[] = {55,48,52,57};
char ptuning[] = "gCEA";
const int tuningMinimum = 48;

#endif
#ifdef DROPD
//BASS
char tuning[] = {16, 21, 26, 31};
char ptuning[] = "eadg";
const int tuningMinimum = 16;

#endif
#ifdef EIGHTSTRING
//8STRING
char tuning[] = {18,23,28, 33, 38, 43, 47, 52};
char ptuning[] = "Fbeadgbe";
const int tuningMinimum = 18;

#endif
#ifdef OPENG
//OPEN G
char tuning[] = {26, 31, 38, 43, 47, 52};
char ptuning[] = "dgdgbe";
const int tuningMinimum = 48;

#endif

const int SIZEOF_TUNING = sizeof(tuning)/sizeof(tuning[0]);