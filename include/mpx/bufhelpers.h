#ifndef BUFHELPERS_H
#define BUFHELPERS_H
#include <string.h>

// macro that gives the determination of the size for constant or literal string arrays
// use only with strings backed by arrays or in conjunction with literals
// good for sys_req to passes the span of a string w/o the automatic null terminator
#define STR_A_SZ(str) (sizeof (str) - 1)
// substitute for the last two paramters to sys_req or similar for the WRITE or READ operations
// * for writing generic information/strings with lengths known at compile time without accounting for null terminators
#define BUF(buf) buf, (sizeof (buf))
// * for printing strings with lengths that are known at compile time, i.e literals or const char* arrays
#define STR_BUF(str) str, STR_A_SZ(str)
// * for strings which may have lengths which are not known at compile time
#define DSTR_BUF(str) str, strlen(str)

#endif // BUFHELPERS_H
