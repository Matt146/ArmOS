#include "util.h"

char* unsigned_long_to_str(unsigned long zahl) {
    for (size_t i = 0; i < 20; i++) {
        __text_buff_unsigned_long_to_str[i] = '\0';
    }
    char* text =  __text_buff_unsigned_long_to_str;  //Make me static, otherwise it's on the stack and will screw up soon, if it's static, it's allocated always, but is not safe for multi-tasking/threading.
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

void memsetb(uint8_t* src, uint8_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

int strncmp(const char * s1, const char * s2, size_t n ) {
    while ( n && *s1 && ( *s1 == *s2 ) ) {
        ++s1;
        ++s2;
        --n;
    }

    if ( n == 0 ) {
        return 0;
    }
    else {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}