#include "bits.h"

// create_bitmask - Creates a bitmask to extract bits a...b
// from an integer. All you need to do to use it is get the value
// and AND it with the other number.
// IE:
// uint64_t mask = create_bitmask(12, 16);
// uint64_t result = mask & NUMBER_YOU_WANT_TO_EXTRACT_BITS_FROM;
uint64_t create_bitmask(uint64_t a, uint64_t b) {
    uint64_t r = 0;
    for (uint64_t i = a; i <= b; i++) {
        r |= (1 << i);
    }

    return r;
}

/****************************************
 * TUTORIAL CODE HERE
 ***************************************/
char* long_to_str(long zahl) {
   static char text[20];   //Make me static, otherwise it's on the stack and will screw up soon, if it's static, it's allocated always, but is not safe for multi-tasking/threading.
   int loc=19;
   text[19] = 0; //NULL terminate the string
   do  //While we have something left, lets add a character to the string
   {
       --loc;
       text[loc] = (zahl%10)+'0';
       zahl/=10;
   } while (zahl);
   return &text[loc];  //Start from where loc left off
}