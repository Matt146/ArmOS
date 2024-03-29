#include "util.h"

static uint8_t util_ulong_to_str_mux;
char* unsigned_long_to_str(unsigned long zahl) {
    mutex_lock(&util_ulong_to_str_mux);
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
   mutex_unlock(&util_ulong_to_str_mux);
   return &text[loc];  //Start from where loc left off
}

char *int_base_to_string(size_t number, size_t base) {
    static char lock = 0;
    mutex_lock(&lock);

    static char digits[] = "0123456789ABCDEF";
    static char buffer[50];
    char *str;

    str = &buffer[49];
    *str = '\0';

    do {
        *--str = digits[number % base];
        number /= base;
    } while(number != 0);

    mutex_unlock(&lock);
    return str;
}

void memsetb(uint8_t* src, uint8_t data, size_t count) {
    for(size_t i = 0; i < count; i++)
        *src++ = data;
}

// From: https://stackoverflow.com/questions/32560167/strncmp-implementation
int strncmp( const char * s1, const char * s2, size_t n ) {
    while ( n && *s1 && ( *s1 == *s2 ) ) {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 ) {
        return 0;
    } else {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

void* memcpy (void* dest, const void* src, size_t len) {
  char *d = dest;
  const char *s = src;
  while (len--)
    *d++ = *s++;
  return dest;
}


static int next;
int rand( void ) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}
 
void srand( unsigned int seed )
{
    next = seed;
}

uint64_t rand64(void) {
  uint64_t r = 0;
  for (int i = 0; i < 64; i += RAND_MAX_WIDTH) {
    r <<= RAND_MAX_WIDTH;
    r ^= (unsigned) rand();
  }
  return r;
}

uint64_t pow(uint64_t x, uint64_t n)
{
    uint64_t i; /* Variable used in loop counter */
    uint64_t number = 1;

    for (i = 0; i < n; ++i)
        number *= x;

    return(number);
}

size_t strlen(const char *str) {
    size_t len = 0;
    while(str[len])
        len++;
    return len;
}
