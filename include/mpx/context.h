#ifndef MPX_CONTEXT_H
#define MPX_CONTEXT_H

struct context
{
    unsigned short gs,
                   fs,
                   es,
                   ds,
                   ss;
    unsigned int edi,
                 esi,
                 ebp,
                 esp,
                 ebx,
                 edx,
                 ecx,
                 eax;
    unsigned int eip;
    unsigned short cs;
    unsigned int eflags;
};

#endif // MPX_CONTEXT_H
