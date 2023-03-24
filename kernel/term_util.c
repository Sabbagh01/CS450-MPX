#include <mpx/term_util.h>

#include <syscall.h>
#include <stddef.h>


const struct serial_text_colors serial_text_colors[] = {
    { STR_BUF(resetColor) },
    { STR_BUF(yellowColor) },
    { STR_BUF(whiteColor) },
    { STR_BUF(redColor) },
    { STR_BUF(purpleColor) },
    { STR_BUF(blueColor) }
};

// simplification for serial terminal color setting.
void setTerminalColor(enum Color color) {
    write(COM1, serial_text_colors[color].colorbytes, serial_text_colors[color].sz);
}

char intParsable(const char* string, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if ( (string[i] > '9') || (string[i] < '0') ) {
            return 0;
        }
    }
    return 1;
}
