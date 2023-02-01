#include <mpx/io.h>
#include <mpx/serial.h>
#include <sys_req.h>
#include <string.h>
#include <stdlib.h>

#include "time.h"
#include "comhand.h"


void setTimeCommand() {
    size_t input_len;
    char input_buffer[30] = { 0 };
    int hour, minute, second;

    const char error_msg[] = "\r\nCould not parse, please re-enter time:";
    
    while(1) {

        //yellow color
        const char yellowColor[] = "\033[0;33m";
        sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

        const char hour_msg[] = "\r\nEnter the hour (0-24):\r\n";
        sys_req(WRITE, COM1, hour_msg, sizeof(hour_msg));

        //white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

        input_len = sys_req(READ, COM1, input_buffer, sizeof(input_buffer));
        hour = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
        
        if ( (hour < 24) && (hour >= 0) ) {
            break;
        }
        //red color
        const char redColor[] = "\033[0;31m";
        sys_req(WRITE, COM1, redColor, sizeof(redColor));

        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));

        //white color
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

    }
    while(1) {

        //yellow color
        const char yellowColor[] = "\033[0;33m";
        sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

        const char minute_msg[] = "\r\nEnter the minute (0-60):\r\n";
        sys_req(WRITE, COM1, minute_msg, sizeof(minute_msg));

        //white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

        input_len = sys_req(READ, COM1, input_buffer, sizeof(input_buffer));
        minute = atoi(input_buffer);
        memset(input_buffer, 0, input_len);
        
        if ( (minute < 60) && (minute >= 0) ) {
            break;
        }
        //red color
        const char redColor[] = "\033[0;31m";
        sys_req(WRITE, COM1, redColor, sizeof(redColor));

        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));

        //white color
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    }
    while (1) {
         //yellow color
        const char yellowColor[] = "\033[0;33m";
        sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

        const char second_msg[] = "\r\nEnter the second (0-60):\r\n";
        sys_req(WRITE, COM1, second_msg, sizeof(second_msg));

         //white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

        input_len = sys_req(READ, COM1, input_buffer, sizeof(input_buffer));
        second = atoi(input_buffer);
        
        if ( (second < 60) && (second >= 0) ) {
            break;
        }
        //red color
        const char redColor[] = "\033[0;31m";
        sys_req(WRITE, COM1, redColor, sizeof(redColor));

        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));

        //white color
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    }
    sys_req(WRITE, COM1, "\r\n", 2);

    setTime (hour, minute, second);
}

void getTimeCommand() {

    //yellow color
    const char yellowColor[] = "\033[0;33m";
    sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

    const char day_msg[] = "\r\nThe time is:\r\n";
    sys_req(WRITE, COM1, day_msg, sizeof (day_msg));

    //white color
    const char whiteColor[] = "\033[0;37m";
    sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    
    getTime();
}

void setDateCommand() {
    size_t input_len;
    char input_buffer[30] = { 0 };
    int day, month, year;

    //red color
    const char yellowColor[] = "\033[0;31m";
    sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));
    
    const char error_msg[] = "\r\nCould not parse, please re-enter time:";

    // white color
    const char whiteColor[] = "\033[0;37m";
    sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    
    while(1) {

        //yellow color
        const char yellowColor[] = "\033[0;33m";
        sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

        const char month_msg[] = "\r\nEnter the month (1-12):\r\n";
        sys_req(WRITE, COM1, month_msg, sizeof (month_msg));

        // white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

        input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
        month = atoi (input_buffer);
        memset (input_buffer, 0, input_len);

        if ( (month <= 12) && (month >= 1) ) {
            break;
        }
        //red color
        const char redColor[] = "\033[0;31m";
        sys_req(WRITE, COM1, redColor, sizeof(redColor));

        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));

        //white color
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));       
    }
    while(1) {
        //yellow color
        const char yellowColor[] = "\033[0;33m";
        sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

        const char day_msg[] = "\r\nEnter the day (1-31):\r\n";
        sys_req(WRITE, COM1, day_msg, sizeof (day_msg));

        // white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

        input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
        day = atoi (input_buffer);
        memset (input_buffer, 0, input_len);
        
        if ( (day <= month_info[month - 1].lastday) && (day >= 1) ) {
            break;
        }

        //red color
        const char redColor[] = "\033[0;31m";
        sys_req(WRITE, COM1, redColor, sizeof(redColor));

        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));

        //white color
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    }
    while(1) {

        //yellow color
        const char yellowColor[] = "\033[0;33m";
        sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

        const char year_msg[] = "\r\nEnter the last two digits of the year (1-99):\r\n";
        sys_req(WRITE, COM1, year_msg, sizeof (year_msg));

        // white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

        input_len = sys_req(READ, COM1, input_buffer, sizeof (input_buffer));
        year = atoi (input_buffer);
        sys_req (WRITE, COM1, "\r\n", 2);

        if ( (year < 2100) && (year >= 0) ) {
            break;
        }
        //red color
        const char redColor[] = "\033[0;31m";
        sys_req(WRITE, COM1, redColor, sizeof(redColor));

        sys_req(WRITE, COM1, error_msg, sizeof(error_msg));

        //white color
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    }

    setDate (day, month, year);
}

void getDateCommand() {

    //yellow color
    const char yellowColor[] = "\033[0;33m";
    sys_req(WRITE, COM1, yellowColor, sizeof(yellowColor));

    const char day_msg[] = "\r\nThe date is:\r\n";
    sys_req(WRITE, COM1, day_msg, sizeof (day_msg));

    // white color
    const char whiteColor[] = "\033[0;37m";
    sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
    
    getDate();
}

void versionCommand() {
    const char ver_msg[] = "\r\nMPX vR1.\r\nDate:\r\n";
    sys_req(WRITE, COM1, ver_msg, sizeof (ver_msg));
    // print compilation date, will probably require some scripting and
    // the passing of a preprocessor define to the compiler in the makefile
}

void helpCommand() {
    const char help_msg[] = "\r\n"
                            "1) Help - Provides usage instructions for all commands\r\n"
                            "2) Set Time - Sets the time\r\n"
                            "3) Get Time - Returns the current time\r\n";
    sys_req(WRITE, COM1, help_msg, sizeof (help_msg));
}

void comhand(){
    //blue color
    const char blueColor[] = "\033[0;34m";
    sys_req(WRITE, COM1, blueColor, sizeof(blueColor));

    const char menu_welcome_msg[] = "Welcome to 5x5 MPX.\r\n";
    const char menu_options[] = "Please select an option by choosing a number.\r\n"
                                "1) Help.        2) Set Time.    3) Get Time.    4) Set Date.\r\n"
                                "5) Get Date.    6) Version.     7) Shut Down.\r\n"
                                "Enter number of choice:\r\n";
    
    sys_req(WRITE, COM1, menu_welcome_msg, sizeof (menu_welcome_msg));

    //white color
    const char whiteColor[] = "\033[0;37m";
    sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));

    while (1) {

        //purple color
        const char purpleColor[] = "\033[0;36m";
        sys_req(WRITE, COM1, purpleColor, sizeof(purpleColor));

        //display the menu
        sys_req(WRITE, COM1, menu_options, sizeof (menu_options));

        //white color
        const char whiteColor[] = "\033[0;37m";
        sys_req(WRITE, COM1, whiteColor, sizeof(whiteColor));
        
        // Get the input and call corresponding function
        char user_input_buffer[100];
        sys_req(READ, COM1, user_input_buffer, sizeof (user_input_buffer));

        if (user_input_buffer[0] == '1') {
            helpCommand();
        } else if (user_input_buffer[0] == '4') {
            setDateCommand();
        } else if (user_input_buffer[0] == '5') {
            getDateCommand();
        } else if (user_input_buffer[0] == '6') {
            versionCommand();
        } else if (user_input_buffer[0]  == '3') {
            getTimeCommand();
        } else if (user_input_buffer[0] == '2') {
            setTimeCommand();
        } else if (user_input_buffer[0] == '4') {
            setDateCommand();
        }
    }
}

