#include <mpx/time.h>

#include <mpx/io.h>
#include <mpx/serial.h>
#include <mpx/sys_req.h>
#include <mpx/interrupts.h>
#include <string.h>


const struct month_info month_info[] = {
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

unsigned char decimalToBCD(int integer) {
    return ((integer / 10) << 4) | ((integer) % 10);
}

int BCDtoDecimal(unsigned char bcd) {
    return (int)(bcd & 0x0F) + (((int)(bcd >> 4)) * 10);
}

void setTime(int hours, int minute, int seconds) {
    cli ();
    
    outb(0x70, 0x00);
    outb(0x71, decimalToBCD(seconds));
    outb(0x70, 0x02);
    outb(0x71, decimalToBCD(minute));
    outb(0x70, 0x04);
    outb(0x71, decimalToBCD(hours));
    
    sti ();

    return;
} 

void getTime() {
    cli();
    
    outb(0x70, 0x00); // access seconds
    int seconds = BCDtoDecimal(inb(0x71));
    outb(0x70, 0x02); // access minutes
    int minutes = BCDtoDecimal(inb(0x71));
    outb(0x70, 0x04); // access hours
    int hours = BCDtoDecimal(inb(0x71));
    
    sti();

    // print time
    char timebuffer[100];
    char fmt[] = "\0:\0:\0"; // format string template for interim formatting
    int fmtp = 0; // format index
    int bufsz = 0; // current buffer progression/size
    // convert seconds, minutes, hours to strings
    char seconds_str[3];
    itoa (seconds_str, seconds);
    if (seconds < 10) {
        seconds_str[1] = seconds_str[0];
        seconds_str[0] = '0';
        seconds_str[2] = '\0';
    }
    char minutes_str[3];
    itoa (minutes_str, minutes);
    if (minutes < 10) {
        minutes_str[1] = minutes_str[0];
        minutes_str[0] = '0';
        minutes_str[2] = '\0';
    }
    char hours_str[3];
    itoa (hours_str, hours);
    if (hours < 10) {
        hours_str[1] = hours_str[0];
        hours_str[0] = '0';
        hours_str[2] = '\0';
    }
    // going to [unsafely] assume we will not overrun datebuffer
    // print hours
    for (int i = 0; hours_str[i] != '\0'; ++i, ++bufsz) {
        timebuffer[bufsz] = hours_str[i];
    }
    ++fmtp;
    // format
    for (; fmt[fmtp] != '\0'; ++fmtp, ++bufsz) {
        timebuffer[bufsz] = fmt[fmtp];
    }
    // print minutes
    for (int i = 0; minutes_str[i] != '\0'; ++i, ++bufsz) {
        timebuffer[bufsz] = minutes_str[i];
    }
    ++fmtp;
    // format
    for (; fmt[fmtp] != '\0'; ++fmtp, ++bufsz) {
        timebuffer[bufsz] = fmt[fmtp];
    }
    // print seconds
    for (int i = 0; seconds_str[i] != '\0'; ++i, ++bufsz) {
        timebuffer[bufsz] = seconds_str[i];
    }
    timebuffer[bufsz++] = '\r';
    timebuffer[bufsz++] = '\n';
    sys_req(WRITE, COM1, timebuffer, bufsz);

    return;
}

void setDate(int day, int month, int year) {
    cli ();

    outb (0x70, (0x09 & ~0x80) | 0x80); // year
    outb (0x71, decimalToBCD (year));
    outb (0x70, (0x08 & ~0x80) | 0x80); // month
    outb (0x71, decimalToBCD (month));
    outb (0x70, (0x07 & ~0x80) | 0x80); // access day
    outb (0x71, decimalToBCD (day)); 
    
    sti ();
    
    return;
}

void getDate() {
    cli ();

    outb (0x70, (0x09 & ~0x80) | 0x80); // year
    int year = BCDtoDecimal (inb (0x71));
    outb (0x70, (0x08 & ~0x80) | 0x80); // month
    int month = BCDtoDecimal (inb (0x71));
    outb (0x70, (0x07 & ~0x80) | 0x80); // access day
    int day = BCDtoDecimal (inb (0x71));
    
    sti ();
    
    // print date
    char datebuffer[100];
    char fmt[] = "\0 \0, 20\0"; // format string template for interim formatting
    int fmtp = 0; // format index
    int bufsz = 0; // current buffer progression/size
    // convert day, year to strings
    char daystr[3];
    itoa (daystr, day);
    char yearstr[3];
    itoa (yearstr, year);
    if (year < 10) {
        yearstr[1] = yearstr[0];
        yearstr[0] = '0';
        yearstr[2] = '\0';
    }
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
    datebuffer[bufsz++] = '\r';
    datebuffer[bufsz++] = '\n';
    sys_req(WRITE, COM1, datebuffer, bufsz);

    return;
}

