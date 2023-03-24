#ifndef MPX_TERM_UTIL_H
#define MPX_TERM_UTIL_H

#include <stddef.h>
#include <mpx/syscalls.h>


// colors, note that how a terminal represents colors will not be constant
// i.e, terminals may be configured with different color palettes
// color strings below thus represent colors on most terminals using their default palette
#define resetColor  "\033[0m"
#define yellowColor "\033[0;33m"
#define whiteColor  "\033[0;37m"
#define redColor    "\033[0;31m"
#define purpleColor "\033[0;36m"
#define blueColor   "\033[0;34m"

struct serial_text_colors { 
    char* colorbytes; 
    size_t sz;
};

extern const struct serial_text_colors serial_text_colors[];

enum Color {
    Reset = 0x00,
    Yellow,
    White,
    Red,
    Purple,
    Blue
};

void setTerminalColor(enum Color color);

char intParsable(const char* string, size_t size);

#endif // MPX_TERM_UTIL_H
