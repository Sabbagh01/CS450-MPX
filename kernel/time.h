#ifndef TIME_H
#define TIME_H

extern const struct month_info {
    const char* name;
    int lastday;
} month_info[];

void setTime(int seconds, int minute, int hour);
void getTime();
void setDate(int day, int month, int year);
void getDate();

#endif // TIME_H

