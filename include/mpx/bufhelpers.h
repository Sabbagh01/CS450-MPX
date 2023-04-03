#ifndef BUFHELPERS_H
#define BUFHELPERS_H

// macro that gives the determination of the size for constant or literal string arrays
// use only with strings backed by arrays or in conjunction with literals
// good for sys_req to passes the span of a string w/o the automatic null terminator
#define STR_A_SZ(str) (sizeof (str) - 1)
// substitute for the last two paramters to sys_req or similar for the WRITE or READ operations
#define BUF(buf) buf, (sizeof (buf))
#define STR_BUF(str) str, STR_A_SZ(str)

#endif // BUFHELPERS_H
