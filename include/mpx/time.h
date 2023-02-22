#ifndef MPX_TIME_H
#define MPX_TIME_H

/**
@file mpx/time.h
@brief Functions to get and set time and date
*/

extern const struct {
    /** @brief The name of a month. */
    const char* name;
    /** @brief The last day of a month. */
    int lastday;
}
/**
@brief Holds month specific information such as end days and strings.
*/
month_info[];

/**
@brief
    Sets the set (24 hour) time of day.
@param seconds
    Time in seconds, from 0-59.
@param minute
    Time in minutes, from 0-59.
@param hour
    Time in hours, from 0-23.
*/
void setTime(int seconds, int minute, int hour);

/**
@brief
    Prints the (24 hour) time of day.
*/
void getTime();

/**
@brief
    Sets the date.
@param day
    Day of the month, must be greater than 0 and less than `month_info[month - 1].last_day`.
@param month
    Month of the year, must be from 1-12.
@param year
    Current year of the twenty-first century. Must be from 0-99.
*/
void setDate(int day, int month, int year);

/**
@brief
    Prints the set date.
*/
void getDate();

#endif // MPX_TIME_H

