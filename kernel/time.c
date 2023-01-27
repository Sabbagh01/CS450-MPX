#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <comhand.h>
#include <mpx/interrupts.h>
#include <string.h>
#include <time.h>


unsigned char decimalToBCD(int integer) {
    return ((integer / 10) << 4) | ((integer) % 10);
}

int BCDtoDecimal(unsigned char bcd) {
    return (int)(bcd & 0x0F) + (((int)(bcd >> 4)) * 10);
}

void setTime(int hours, int minute) {
    hours += minute;
    minute += hours;
    return;
//     //();
//     // convert to UTC
//     hours = hours + 4;

//     // if hours > 24
//     if(hours >= 24){
//         hours = hours - 24;
        
//         // increment date by 1
//         // int day = BintoDec(outb(0x70, 0x70));
//         // int month = BintoDec(outb(0x70,0x08));
//         // int year = BintoDec(outb(0x70,0x09));
        
//         //setDate(day, month, year);
//     }

//     // convert to binary
//     outb(0x70, 0x00);
//     //outb(0x71,dectoBin(seconds));
    
//     outb(0x70, 0x02);
//     //outb(0x71, dectoBin(minute));
    
//     outb(0x70, 0x04);
//    // outb(0x71, dectoBin(hours));
     
//     //sti();

}

void getTime() {
}

const struct month_info {
    char* name;
    int lastday;
} month_info[] = {
    {"January", 31},
    {"February", 28},
    {"March", 31},
    {"April", 30},
    {"May", 31},
    {"June", 30},
    {"July", 31},
    {"August", 31},
    {"September", 30},
    {"October", 31},
    {"November", 30},
    {"December", 31}
};

void setDate(int day, int month, int year) {
    // preconditions
    if ((month > 12) || (month < 0)) {
        return;
    }
    if ((day > month_info[month - 1].lastday) || (day < 0)) {
        if (!(month == 2 && day == 29)) {
            return;
        }
    }
    if (year < 0) {
        return;
    }
    
    unsigned char bcdday = decimalToBCD (day);
    unsigned char bcdmonth = decimalToBCD (month);
    unsigned char bcdyear = decimalToBCD (year);
    
    cli ();
    
    outb (0x70, (0x07 & ~0x80) | 0x80); // access day
    outb (0x71, bcdday);
    outb (0x70, (0x08 & ~0x80) | 0x80); // month
    outb (0x71, bcdmonth);
    outb (0x70, (0x09 & ~0x80) | 0x80); // year
    outb (0x71, bcdyear);
    
    sti ();
    
    return;
}

void getDate() {
    cli ();

    outb (0x70, (0x07 & ~0x80) | 0x80); // access day
    int day = BCDtoDecimal (inb (0x71));
    outb (0x70, (0x08 & ~0x80) | 0x80); // month
    int month = BCDtoDecimal (inb (0x71));
    outb (0x70, (0x09 & ~0x80) | 0x80); // year
    int year = BCDtoDecimal (inb (0x71));
    
    sti ();
    
    // print date
    char datebuffer[100];
    char fmt[] = "\0 \0, 20\0"; // format string template for interim formatting
    int fmtp = 0; // format index
    int bufsz = 0; // current buffer progression/size
    // convert day, year to string
    char daystr[3];
    itoa (daystr, day);
    char yearstr[3];
    itoa (yearstr, year);
    // going to [unsafely] assume we will not overrun datebuffer
    // print month
    for (int i = 0; month_info[month - 1].name[i] != '\0'; ++i, ++bufsz) {
        datebuffer[bufsz] = month_info[month - 1].name[i];
    }
    ++fmtp;
    // format
    for (; fmt[fmtp] != '\0'; ++fmtp, ++bufsz) {
        datebuffer[bufsz] = fmt[fmtp];
    }
    // print day
    for (int i = 0; daystr[i] != '\0'; ++i, ++bufsz) {
        datebuffer[bufsz] = daystr[i];
    }
    ++fmtp;
    // format
    for (; fmt[fmtp] != '\0'; ++fmtp, ++bufsz) {
        datebuffer[bufsz] = fmt[fmtp];
    }
    // print year
    for (int i = 0; yearstr[i] != '\0'; ++i, ++bufsz) {
        datebuffer[bufsz] = yearstr[i];
    }
    datebuffer[bufsz++] = '\n';
    sys_req(WRITE, COM1, datebuffer, bufsz);
}

