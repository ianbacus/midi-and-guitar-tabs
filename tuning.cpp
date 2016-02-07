#include "tuning.h"
//Recompile the project after updating the tuning of the instrument




/*
std::vector<int> tuning = {28, 33, 38, 43, 47, 52};
std::vector<char> ptuning = {'e','a','d','g','b','e'};
int SIZEOF_TUNING = 6;
*/

//STANDARD GUITAR

char tuning[] = {28, 33, 38, 43, 47, 52};
char ptuning[] = "eadgbe";


//CODY BASS TAB
//char tuning[] = {16, 21, 26, 31};
//char ptuning[] = "eadg";

//8STRING TAB
//char tuning[] = {76, 81, 86, 91, 95, 100};
//char ptuning[] = "Fbeadfgbe";


//goldberg variations tuning
//char tuning[] = {26, 31, 38, 43, 47, 52};
//char ptuning[] = "dgdgbe";


const int SIZEOF_TUNING = sizeof(tuning)/sizeof(tuning[0]);








/*
__________________________________________________________________________________________
*/
//char tuning[6];
/*
tuning[0] = 28;  //E:28, D:26
tuning[1] = 33; //A:33
tuning[2] = 38; //D:38   
tuning[3] = 43; //G:43
tuning[4] = 47; //B:47
tuning[5] = 52; //E:52

*/

//char tuning[] = {16, 21, 26, 31, 35, 40};



//extern char tuning[8] = {18, 23, 28, 33, 38, 43, 47, 52};
/*
tuning[0] = 18;//F#:18 E:16
tuning[1] = 23; //B
tuning[2] = 28;  //E:28, D:26
tuning[3] = 33; //A:33
tuning[4] = 38; //D:38   
tuning[5] = 43; //G:43
tuning[6] = 47; //B:47
tuning[7] = 52; //E:52
*/